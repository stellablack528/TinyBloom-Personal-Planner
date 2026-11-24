#pragma once

#include <QByteArray>
#include <QString>

// Read the first few seconds and prepare a quiet PCM WAV for the focus reminder.
QByteArray prepareFocusClip(const QString &filePath);
