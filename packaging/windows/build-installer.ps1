[CmdletBinding()]
param(
    [string]$Version = "0.4.1-beta.1",
    [string]$NumericVersion = "0.4.1.10"
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$distDir = Join-Path $repoRoot "dist"
$packageDir = Join-Path $distDir "TinyBloom-Desktop-v$Version-windows-x64"
$installerScript = Join-Path $PSScriptRoot "TinyBloom.iss"
$outputName = "TinyBloom-Desktop-v$Version-Windows-x64-Setup.exe"
$outputPath = Join-Path $distDir $outputName
$checksumPath = "$outputPath.sha256.txt"

if (-not (Test-Path -LiteralPath (Join-Path $packageDir "TinyBloom.exe"))) {
    throw "Packaged application not found: $packageDir"
}

$isccCandidates = @(
    (Join-Path $env:LOCALAPPDATA "Programs\Inno Setup 7\ISCC.exe"),
    (Join-Path $env:ProgramFiles "Inno Setup 7\ISCC.exe"),
    (Join-Path ${env:ProgramFiles(x86)} "Inno Setup 7\ISCC.exe"),
    (Join-Path $env:LOCALAPPDATA "Programs\Inno Setup 6\ISCC.exe"),
    (Join-Path ${env:ProgramFiles(x86)} "Inno Setup 6\ISCC.exe")
)

$iscc = $isccCandidates | Where-Object { $_ -and (Test-Path -LiteralPath $_) } | Select-Object -First 1
if (-not $iscc) {
    throw "Inno Setup 6 or 7 was not found. Install it from https://jrsoftware.org/isdl.php"
}

$compilerArguments = @("/DAppVersion=$Version", "/DAppNumericVersion=$NumericVersion", "/DAppSourceDir=$packageDir", "/DOutputDir=$distDir")
$runtimeFiles = @{
    GccRuntimeHash = 'libgcc_s_seh-1.dll'
    CppRuntimeHash = 'libstdc++-6.dll'
    PthreadRuntimeHash = 'libwinpthread-1.dll'
}
foreach ($runtime in $runtimeFiles.GetEnumerator()) {
    $path = Join-Path $packageDir $runtime.Value
    if (Test-Path -LiteralPath $path -PathType Leaf) {
        $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
        $compilerArguments += "/D$($runtime.Key)=$hash"
    }
}
& $iscc @compilerArguments $installerScript
if ($LASTEXITCODE -ne 0) {
    throw "Inno Setup failed with exit code $LASTEXITCODE"
}

if (-not (Test-Path -LiteralPath $outputPath)) {
    throw "Installer was not created: $outputPath"
}

$hash = (Get-FileHash -LiteralPath $outputPath -Algorithm SHA256).Hash
$checksumLine = "$hash  $outputName`r`n"
[System.IO.File]::WriteAllText($checksumPath, $checksumLine, [System.Text.UTF8Encoding]::new($false))

Write-Output "Installer: $outputPath"
Write-Output "SHA256:   $hash"
Write-Output "Checksum: $checksumPath"
