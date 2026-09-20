#define AppName "TinyBloom Desktop"
#define AppExeName "TinyBloom.exe"

#ifndef AppVersion
  #define AppVersion "0.2.0"
#endif

#ifndef AppSourceDir
  #define AppSourceDir "..\..\dist\TinyBloom-Desktop-v" + AppVersion + "-windows-x64"
#endif

#ifndef OutputDir
  #define OutputDir "..\..\dist"
#endif

[Setup]
AppId={{31C68E64-F2CD-4EE6-AC9D-5B1BCA01F1F0}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher=TinyBloom Contributors
AppPublisherURL=https://github.com/stellablack528/TinyBloom-Personal-Planner
AppSupportURL=https://github.com/stellablack528/TinyBloom-Personal-Planner/issues
AppUpdatesURL=https://github.com/stellablack528/TinyBloom-Personal-Planner/releases
DefaultDirName={localappdata}\Programs\TinyBloom Desktop
DefaultGroupName=TinyBloom Desktop
DisableProgramGroupPage=yes
LicenseFile=..\..\LICENSE
OutputDir={#OutputDir}
OutputBaseFilename=TinyBloom-Desktop-v{#AppVersion}-Windows-x64-Setup
SetupIconFile=..\..\resources\icons\app-icon.ico
UninstallDisplayIcon={app}\{#AppExeName}
UninstallDisplayName={#AppName}
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
VersionInfoVersion={#AppVersion}.0
VersionInfoCompany=TinyBloom Contributors
VersionInfoDescription=TinyBloom Desktop Windows Installer
VersionInfoProductName=TinyBloom Desktop
VersionInfoProductVersion={#AppVersion}
VersionInfoCopyright=Copyright (C) TinyBloom Contributors

[Languages]
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkedonce

[Files]
Source: "{#AppSourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\TinyBloom Desktop"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"
Name: "{autodesktop}\TinyBloom Desktop"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,TinyBloom Desktop}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent
