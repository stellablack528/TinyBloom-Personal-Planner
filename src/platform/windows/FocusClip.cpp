#include "FocusClip.h"

#include <QDataStream>
#include <QDir>
#include <QIODevice>
#include <QScopeGuard>
#include <QtEndian>
#include <QtCore/qt_windows.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>

#include <cmath>

namespace {
using Microsoft::WRL::ComPtr;
constexpr DWORD AudioStream = MF_SOURCE_READER_FIRST_AUDIO_STREAM;
constexpr int MaximumSeconds = 5;
constexpr double PeakLimit = 0.14;

QByteArray writeQuietWave(const QByteArray &pcm, UINT32 rate, UINT32 channels, UINT32 bits)
{
    const qsizetype sampleBytes = bits / 8;
    const qsizetype frames = pcm.size() / (sampleBytes * channels);
    if (!frames) return {};

    QByteArray wave;
    QDataStream out(&wave, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    const quint32 size = frames * channels * 2;
    out.writeRawData("RIFF", 4);
    out << quint32(36 + size);
    out.writeRawData("WAVEfmt ", 8);
    out << quint32(16) << quint16(1) << quint16(channels) << quint32(rate)
        << quint32(rate * channels * 2) << quint16(channels * 2) << quint16(16);
    out.writeRawData("data", 4);
    out << size;
    wave.resize(44 + size);

    const auto sampleAt = [bits, sampleBytes, &pcm](qsizetype index) {
        const auto *p = reinterpret_cast<const uchar *>(pcm.constData() + index * sampleBytes);
        switch (bits) {
        case 8: return (int(p[0]) - 128) / 128.0;
        case 16: return qFromLittleEndian<qint16>(p) / 32768.0;
        case 24: {
            const qint32 value = int(p[0]) | (int(p[1]) << 8) | (int(p[2]) << 16);
            return (value - (value >= 0x800000 ? 0x1000000 : 0)) / 8388608.0;
        }
        default: return qFromLittleEndian<qint32>(p) / 2147483648.0;
        }
    };
    double peak = 0;
    for (qsizetype i = 0; i < frames * channels; ++i)
        peak = qMax(peak, std::abs(sampleAt(i)));
    if (peak == 0) return {};
    const double gain = qMin(1.0, PeakLimit / peak);
    const double fadeInFrames = rate * 0.08;
    const double fadeOutFrames = rate * 0.22;
    auto *output = reinterpret_cast<uchar *>(wave.data() + 44);
    for (qsizetype frame = 0; frame < frames; ++frame) {
        const double envelope = qMin(1.0, qMin(frame / fadeInFrames, (frames - frame - 1) / fadeOutFrames));
        for (UINT32 channel = 0; channel < channels; ++channel) {
            const qsizetype i = frame * channels + channel;
            qToLittleEndian<qint16>(qRound(32767 * gain * envelope * sampleAt(i)), output + i * 2);
        }
    }
    return wave;
}
}

QByteArray prepareFocusClip(const QString &filePath)
{
    const HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(com)) return {};
    const auto stopCom = qScopeGuard([] { CoUninitialize(); });

    // Load on demand so Windows editions without Media Foundation can still run TinyBloom.
    const auto platform = LoadLibraryExW(L"mfplat.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    const auto readerLibrary = LoadLibraryExW(L"mfreadwrite.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    const auto closeLibraries = qScopeGuard([=] {
        if (readerLibrary) FreeLibrary(readerLibrary);
        if (platform) FreeLibrary(platform);
    });
    if (!platform || !readerLibrary) return {};
    const auto start = reinterpret_cast<decltype(&MFStartup)>(GetProcAddress(platform, "MFStartup"));
    const auto stop = reinterpret_cast<decltype(&MFShutdown)>(GetProcAddress(platform, "MFShutdown"));
    const auto createType = reinterpret_cast<decltype(&MFCreateMediaType)>(GetProcAddress(platform, "MFCreateMediaType"));
    const auto createReader = reinterpret_cast<decltype(&MFCreateSourceReaderFromURL)>(
        GetProcAddress(readerLibrary, "MFCreateSourceReaderFromURL"));
    if (!start || !stop || !createType || !createReader || FAILED(start(MF_VERSION, MFSTARTUP_LITE))) return {};
    const auto stopMf = qScopeGuard([=] { stop(); });

    ComPtr<IMFSourceReader> reader;
    ComPtr<IMFMediaType> format;
    const auto nativePath = QDir::toNativeSeparators(filePath).toStdWString();
    if (FAILED(createReader(nativePath.c_str(), nullptr, reader.GetAddressOf()))
        || FAILED(reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE))
        || FAILED(reader->SetStreamSelection(AudioStream, TRUE))
        || FAILED(createType(format.GetAddressOf()))
        || FAILED(format->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio))
        || FAILED(format->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM))
        || FAILED(reader->SetCurrentMediaType(AudioStream, nullptr, format.Get()))
        || FAILED(reader->GetCurrentMediaType(AudioStream, format.ReleaseAndGetAddressOf())))
        return {};

    UINT32 rate = 0, channels = 0, bits = 0;
    if (FAILED(format->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &rate))
        || FAILED(format->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels))
        || FAILED(format->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bits))
        || rate < 8000 || rate > 192000 || channels < 1 || channels > 2
        || (bits != 8 && bits != 16 && bits != 24 && bits != 32))
        return {};

    const qsizetype limit = rate * channels * (bits / 8) * MaximumSeconds;
    QByteArray pcm;
    pcm.reserve(limit);
    for (int reads = 0; reads < 2000 && pcm.size() < limit; ++reads) {
        DWORD flags = 0;
        ComPtr<IMFSample> sample;
        if (FAILED(reader->ReadSample(AudioStream, 0, nullptr, &flags, nullptr, sample.GetAddressOf()))
            || (flags & (MF_SOURCE_READERF_ERROR | MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)))
            return {};
        if (sample) {
            ComPtr<IMFMediaBuffer> buffer;
            BYTE *bytes = nullptr;
            DWORD length = 0;
            if (FAILED(sample->ConvertToContiguousBuffer(buffer.GetAddressOf()))
                || FAILED(buffer->Lock(&bytes, nullptr, &length)))
                return {};
            pcm.append(reinterpret_cast<const char *>(bytes), qMin<qsizetype>(limit - pcm.size(), length));
            buffer->Unlock();
        }
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
    }
    return writeQuietWave(pcm, rate, channels, bits);
}
