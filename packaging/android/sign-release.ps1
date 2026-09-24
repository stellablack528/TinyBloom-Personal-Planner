param(
    [string]$InputApk = "build-android-arm64-release/android-build/build/outputs/apk/release/android-build-release-unsigned.apk",
    [string]$OutputApk = "dist/TinyBloom-Mobile-v0.3.1-beta.1-Android-arm64.apk",
    [string]$SigningProperties = ".signing/keystore.properties",
    [string]$BuildToolsDirectory = "E:/Android/Sdk/build-tools/36.0.0"
)

$ErrorActionPreference = "Stop"

function Read-Properties([string]$Path) {
    $result = @{}
    foreach ($line in Get-Content -LiteralPath $Path) {
        if ($line -match '^([^#=]+)=(.*)$') {
            $result[$matches[1].Trim()] = $matches[2].Trim()
        }
    }
    return $result
}

$inputPath = (Resolve-Path -LiteralPath $InputApk).Path
$propertiesPath = (Resolve-Path -LiteralPath $SigningProperties).Path
$properties = Read-Properties $propertiesPath
foreach ($requiredKey in @("storeFile", "storePassword", "keyAlias", "keyPassword")) {
    if (-not $properties.ContainsKey($requiredKey) -or [string]::IsNullOrWhiteSpace($properties[$requiredKey])) {
        throw "Missing '$requiredKey' in $propertiesPath"
    }
}

$zipalign = Join-Path $BuildToolsDirectory "zipalign.exe"
$apksigner = Join-Path $BuildToolsDirectory "apksigner.bat"
if (-not (Test-Path -LiteralPath $zipalign) -or -not (Test-Path -LiteralPath $apksigner)) {
    throw "Android build tools were not found in $BuildToolsDirectory"
}

$outputFullPath = [IO.Path]::GetFullPath((Join-Path (Get-Location) $OutputApk))
$outputDirectory = Split-Path -Parent $outputFullPath
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
$alignedApk = Join-Path ([IO.Path]::GetTempPath()) ("tinybloom-aligned-" + [Guid]::NewGuid() + ".apk")

try {
    & $zipalign -P 16 -f 4 $inputPath $alignedApk
    if ($LASTEXITCODE -ne 0) { throw "zipalign failed" }

    & $apksigner sign `
        --ks $properties.storeFile `
        --ks-key-alias $properties.keyAlias `
        --ks-pass "pass:$($properties.storePassword)" `
        --key-pass "pass:$($properties.keyPassword)" `
        --min-sdk-version 21 `
        --max-sdk-version 36 `
        --v1-signing-enabled true `
        --v2-signing-enabled true `
        --v3-signing-enabled true `
        --v4-signing-enabled false `
        --out $outputFullPath `
        $alignedApk
    if ($LASTEXITCODE -ne 0) { throw "apksigner failed" }

    & $apksigner verify --verbose --min-sdk-version 21 --max-sdk-version 36 $outputFullPath
    if ($LASTEXITCODE -ne 0) { throw "APK signature verification failed" }

    & $zipalign -c -P 16 -v 4 $outputFullPath | Select-Object -Last 1
    if ($LASTEXITCODE -ne 0) { throw "APK alignment verification failed" }

    $hash = (Get-FileHash -LiteralPath $outputFullPath -Algorithm SHA256).Hash.ToLowerInvariant()
    Write-Output "APK: $outputFullPath"
    Write-Output "SHA-256: $hash"
}
finally {
    if (Test-Path -LiteralPath $alignedApk) {
        Remove-Item -LiteralPath $alignedApk -Force
    }
}
