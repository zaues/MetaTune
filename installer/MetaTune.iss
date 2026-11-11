; Windows installer for the MetaTune VST3 (personal build).
; Build:  "%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe" installer\MetaTune.iss
;         (after scripts\build-windows.cmd Release) -> C:\Users\TylerPC\Downloads\Updated\MetaTune-Setup.exe
; Installs the plugin into the standard VST3 folder, where FL Studio and other hosts find it,
; and keeps an uninstaller plus the license files in Program Files\MetaTune.

#define AppName "MetaTune"
#ifndef AppVersion
  #define AppVersion "0.6.1"
#endif
#define Bundle "MetaTune.vst3"
#define BuiltBundle "..\build\msvc-Release\MetaTune_artefacts\Release\VST3\" + Bundle
#define BuiltUpdater "..\build\msvc-Release\MetaTuneUpdater_artefacts\Release\MetaTune Updater.exe"
#define KeyBundle "MetaKey.vst3"
#define BuiltKey "..\build\msvc-Release\MetaKey_artefacts\Release\VST3\" + KeyBundle
#define BuiltKeySetup "C:\Users\TylerPC\Downloads\Updated\MetaKey-Setup.exe"

[Setup]
AppId={{5B7C2E1A-8F3D-4C6B-9A2E-7D41F0C3B8E5}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher=MetaTune personal build
DefaultDirName={autopf}\{#AppName}
DisableDirPage=no
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=C:\Users\TylerPC\Downloads\Updated
OutputBaseFilename=MetaTune-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName}
LicenseFile=..\LICENSE

[Types]
Name: "full"; Description: "MetaTune + MetaKey (recommended)"
Name: "custom"; Description: "Custom"; Flags: iscustom

[Components]
Name: "metatune"; Description: "MetaTune - pitch correction"; Types: full custom; Flags: fixed
Name: "metakey"; Description: "MetaKey - key detector (companion plugin)"; Types: full custom

[InstallDelete]
; The old MetaTune build is the same plugin underneath: hosts would list it twice.
Type: filesandordirs; Name: "{commoncf64}\VST3\MetaTune.vst3"
; Remove the previous bundle first so the folder takes this build's exact capitalisation.
Type: filesandordirs; Name: "{commoncf64}\VST3\{#Bundle}"
Type: filesandordirs; Name: "{commoncf64}\VST3\{#KeyBundle}"

[Files]
Source: "{#BuiltBundle}\*"; DestDir: "{commoncf64}\VST3\{#Bundle}"; Flags: recursesubdirs createallsubdirs ignoreversion
; MetaKey: the separate key-detection plugin (optional component, on by default).
Source: "{#BuiltKey}\*"; DestDir: "{commoncf64}\VST3\{#KeyBundle}"; Components: metakey; Flags: recursesubdirs createallsubdirs ignoreversion
; Always bundle the standalone MetaKey installer so MetaTune's Auto Key can install it on demand.
Source: "{#BuiltKeySetup}"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
; The plugin's Update button runs a temp copy of this; the copy lets this file be replaced during an update.
Source: "{#BuiltUpdater}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\THIRD_PARTY_NOTICES.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\licenses\*"; DestDir: "{app}\licenses"; Flags: ignoreversion

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#Bundle}"
Type: filesandordirs; Name: "{commoncf64}\VST3\{#KeyBundle}"

[Messages]
FinishedLabel=MetaTune and MetaKey are installed.%n%nThe plugins (VST3) are in the standard VST3 folder: {commoncf64}\VST3%nThe updater and licenses are in: {app}%n%nOpen FL Studio. If they aren't in the plugin list yet, go to Options > Manage plugins > Find more plugins.

[Code]
// FL Studio keeps loaded plugins locked: replacing the bundle while it runs fails or leaves the old one.
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
    Result := False; // if the check itself fails, don't block the install
  end;
end;

function FLStudioRunning: Boolean;
begin
  Result := IsRunning ('FL64.exe') or IsRunning ('FL.exe');
end;

function InitializeSetup: Boolean;
var
  Waited: Integer;
begin
  Result := True;
  // Silent install from MetaTune Updater: it already closed FL; give it a moment, never show a dialog.
  if WizardSilent then
  begin
    Waited := 0;
    while FLStudioRunning and (Waited < 30) do
    begin
      Sleep (1000);
      Waited := Waited + 1;
    end;
    Result := not FLStudioRunning;
    Exit;
  end;
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
