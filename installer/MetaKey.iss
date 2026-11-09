; Standalone Windows installer for the MetaKey VST3 (the key-detection companion to MetaTune).
; Build:  "%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe" /DAppVersion=X.Y.Z installer\MetaKey.iss
;         (after scripts\build-windows.cmd Release) -> C:\Users\TylerPC\Downloads\Updated\MetaKey-Setup.exe
; MetaTune bundles this and launches it from its Auto Key panel when MetaKey isn't installed.

#define AppName "MetaKey"
#ifndef AppVersion
  #define AppVersion "0.1.0"
#endif
#define Bundle "MetaKey.vst3"
#define BuiltBundle "..\build\msvc-Release\MetaKey_artefacts\Release\VST3\" + Bundle

[Setup]
AppId={{9C4E7F2B-1A6D-4E8C-B3F5-2D70A9C1E6B4}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher=MetaKey personal build
DefaultDirName={autopf}\{#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=C:\Users\TylerPC\Downloads\Updated
OutputBaseFilename=MetaKey-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName}

[InstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#Bundle}"

[Files]
Source: "{#BuiltBundle}\*"; DestDir: "{commoncf64}\VST3\{#Bundle}"; Flags: recursesubdirs createallsubdirs ignoreversion

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#Bundle}"

[Messages]
FinishedLabel=MetaKey is installed.%n%nThe plugin (VST3) is in: {commoncf64}\VST3\MetaKey.vst3%n%nIn FL Studio, run Options > Manage plugins > Find more plugins, then add MetaKey (an effect) to your beat's mixer track.

[Code]
function IsRunning (const ExeName: String): Boolean;
var
  Wmi, Svc, Found: Variant;
begin
  Result := False;
  try
    Wmi := CreateOleObject ('WbemScripting.SWbemLocator');
    Svc := Wmi.ConnectServer ('.', 'root\CIMV2');
    Found := Svc.ExecQuery ('SELECT Name FROM Win32_Process WHERE Name = "' + ExeName + '"');
    Result := Found.Count > 0;
  except
    Result := False;
  end;
end;

function FLStudioRunning: Boolean;
begin
  Result := IsRunning ('FL64.exe') or IsRunning ('FL.exe');
end;

function InitializeSetup: Boolean;
begin
  Result := True;
  while FLStudioRunning do
  begin
    if MsgBox ('FL Studio is open. Close it, then click OK to continue.' + #13#10 + #13#10 +
               'Click Cancel to stop the installation.', mbInformation, MB_OKCANCEL) = IDCANCEL then
    begin
      Result := False;
      Exit;
    end;
  end;
end;
