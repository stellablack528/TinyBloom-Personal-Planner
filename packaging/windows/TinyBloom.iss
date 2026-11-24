#define AppName "TinyBloom Desktop"
#define AppExeName "TinyBloom.exe"

#ifndef AppVersion
  #define AppVersion "0.4.0-beta.1"
#endif

#ifndef AppNumericVersion
  #define AppNumericVersion "0.4.0.1"
#endif

#ifndef AppSourceDir
  #define AppSourceDir "..\..\dist\TinyBloom-Desktop-v" + AppVersion + "-windows-x64"
#endif

#ifndef OutputDir
  #define OutputDir "..\..\dist"
#endif

#ifndef GccRuntimeHash
  #define GccRuntimeHash ""
#endif
#ifndef CppRuntimeHash
  #define CppRuntimeHash ""
#endif
#ifndef PthreadRuntimeHash
  #define PthreadRuntimeHash ""
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
; Keep installation responsive on everyday laptops. ultra64 saves only a few
; megabytes but needs a much larger dictionary and causes visible CPU/disk spikes.
Compression=lzma2/normal
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
VersionInfoVersion={#AppNumericVersion}
VersionInfoCompany=TinyBloom Contributors
VersionInfoDescription=TinyBloom Desktop Windows Installer
VersionInfoProductName=TinyBloom Desktop
VersionInfoProductVersion={#AppNumericVersion}
VersionInfoCopyright=Copyright (C) TinyBloom Contributors

[Languages]
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[CustomMessages]
chinesesimplified.AppBusy=旧版 TinyBloom 尚未退出。请在时钟旁的小叶子上点右键，选择“退出 TinyBloom”，然后重试安装。
english.AppBusy=TinyBloom is still running. Right-click its leaf icon near the clock, select Exit TinyBloom, and try the installation again.
chinesesimplified.FolderUnavailable=安装目录中的文件暂时无法替换。请关闭正在使用此目录的程序，或选择你有写入权限的目录，再重新安装。受影响的文件：%1
english.FolderUnavailable=Setup could not update a file in this folder. Close programs using the folder, or choose a writable installation folder and try again. Affected file: %1

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkedonce

[Files]
Source: "..\..\resources\sounds\CREDITS.txt"; DestDir: "{app}"; DestName: "Audio-Credits.txt"; Flags: ignoreversion
Source: "{#AppSourceDir}\*"; DestDir: "{app}"; Excludes: "\libgcc_s_seh-1.dll,\libstdc++-6.dll,\libwinpthread-1.dll,\opengl32sw.dll,\translations\*,\qmltooling\*,*.qmltypes"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppSourceDir}\libgcc_s_seh-1.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist solidbreak; Check: RuntimeNeedsUpdate('libgcc_s_seh-1.dll', '{#GccRuntimeHash}')
Source: "{#AppSourceDir}\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist; Check: RuntimeNeedsUpdate('libstdc++-6.dll', '{#CppRuntimeHash}')
Source: "{#AppSourceDir}\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist; Check: RuntimeNeedsUpdate('libwinpthread-1.dll', '{#PthreadRuntimeHash}')
Source: "{#AppSourceDir}\translations\*_zh_CN.qm"; DestDir: "{app}\translations"; Flags: ignoreversion skipifsourcedoesntexist solidbreak

[Icons]
Name: "{autoprograms}\TinyBloom Desktop"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"
Name: "{autodesktop}\TinyBloom Desktop"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,TinyBloom Desktop}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent

[Code]
const
  ProcessQueryLimitedInformation = $1000;
  WmClose = $0010;
  DeleteAccess = $00010000;
  ShareReadWriteDelete = 7;
  OpenExisting = 3;

function NextTopLevelWindow(Parent, After: HWND; ClassName, WindowName: NativeUInt): HWND;
  external 'FindWindowExW@user32.dll stdcall';
function WindowTitleLength(Window: HWND): Integer;
  external 'GetWindowTextLengthW@user32.dll stdcall';
function WindowProcessId(Window: HWND; var ProcessId: Cardinal): Cardinal;
  external 'GetWindowThreadProcessId@user32.dll stdcall';
function OpenProcessHandle(Access: Cardinal; Inherit: Boolean; ProcessId: Cardinal): THandle;
  external 'OpenProcess@kernel32.dll stdcall';
function QueryProcessPath(Process: THandle; Flags: Cardinal; Buffer: String; var Length: Cardinal): Boolean;
  external 'QueryFullProcessImageNameW@kernel32.dll stdcall';
function CloseWinHandle(Handle: THandle): Boolean;
  external 'CloseHandle@kernel32.dll stdcall';
function PostWindowMessage(Window: HWND; Msg: Cardinal; WParam, LParam: NativeUInt): Boolean;
  external 'PostMessageW@user32.dll stdcall';
function OpenExistingFile(FileName: String; Access, ShareMode: Cardinal; Security: NativeUInt;
  Creation, Attributes: Cardinal; Template: THandle): THandle;
  external 'CreateFileW@kernel32.dll stdcall';

function VisitInstalledWindows(const ExePath: String; RequestClose: Boolean): Boolean;
var
  Window: HWND;
  Pid, PathLength: Cardinal;
  Process: THandle;
  ImagePath: String;
begin
  Result := False;
  Window := NextTopLevelWindow(0, 0, 0, 0);
  while Window <> 0 do begin
    if WindowTitleLength(Window) > 0 then begin
      Pid := 0;
      WindowProcessId(Window, Pid);
      if Pid <> 0 then begin
        Process := OpenProcessHandle(ProcessQueryLimitedInformation, False, Pid);
        if Process <> 0 then begin
          PathLength := 32768;
          ImagePath := StringOfChar(#0, PathLength);
          if QueryProcessPath(Process, 0, ImagePath, PathLength) then begin
            SetLength(ImagePath, PathLength);
            if CompareText(ImagePath, ExePath) = 0 then begin
              Result := True;
              if RequestClose then PostWindowMessage(Window, WmClose, 0, 0);
            end;
          end;
          CloseWinHandle(Process);
        end;
      end;
    end;
    Window := NextTopLevelWindow(0, Window, 0, 0);
  end;
end;

function RuntimeNeedsUpdate(const FileName, ExpectedHash: String): Boolean;
var
  Target: String;
begin
  Result := True;
  Target := ExpandConstant('{app}\') + FileName;
  if (ExpectedHash <> '') and FileExists(Target) then begin
    try
      Result := CompareText(GetSHA256OfFile(Target), ExpectedHash) <> 0;
    except
      Log('Could not compare existing runtime: ' + FileName);
    end;
  end;
  if not Result then Log('Keeping identical runtime: ' + FileName);
end;

function CanReplace(const Target: String): Boolean;
var
  Handle: THandle;
begin
  Result := True;
  if not FileExists(Target) then Exit;
  Handle := OpenExistingFile(Target, DeleteAccess, ShareReadWriteDelete, 0, OpenExisting, 0, 0);
  Result := Handle <> THandle(-1);
  if Result then CloseWinHandle(Handle);
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  AppDir, ExePath, BlockedFile: String;
  Attempt: Integer;
begin
  Result := '';
  AppDir := ExpandConstant('{app}\');
  ExePath := AppDir + '{#AppExeName}';
  if VisitInstalledWindows(ExePath, True) then begin
    Log('Waiting for this installation of TinyBloom to close normally.');
    for Attempt := 1 to 40 do begin
      Sleep(150);
      if not VisitInstalledWindows(ExePath, False) and CanReplace(ExePath) then Break;
    end;
    if VisitInstalledWindows(ExePath, False) then begin
      Result := CustomMessage('AppBusy');
      Exit;
    end;
  end;

  BlockedFile := '';
  if not CanReplace(ExePath) then BlockedFile := '{#AppExeName}'
  else if not CanReplace(AppDir + 'Qt6Core.dll') then BlockedFile := 'Qt6Core.dll'
  else if not CanReplace(AppDir + 'Qt6Gui.dll') then BlockedFile := 'Qt6Gui.dll'
  else if RuntimeNeedsUpdate('libgcc_s_seh-1.dll', '{#GccRuntimeHash}') and
      not CanReplace(AppDir + 'libgcc_s_seh-1.dll') then BlockedFile := 'libgcc_s_seh-1.dll'
  else if RuntimeNeedsUpdate('libstdc++-6.dll', '{#CppRuntimeHash}') and
      not CanReplace(AppDir + 'libstdc++-6.dll') then BlockedFile := 'libstdc++-6.dll'
  else if RuntimeNeedsUpdate('libwinpthread-1.dll', '{#PthreadRuntimeHash}') and
      not CanReplace(AppDir + 'libwinpthread-1.dll') then BlockedFile := 'libwinpthread-1.dll';
  if BlockedFile <> '' then Result := FmtMessage(CustomMessage('FolderUnavailable'), [BlockedFile]);
end;
