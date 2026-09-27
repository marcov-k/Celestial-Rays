#define AppName "Celestial-Rays-Demo"
#define Publisher "Marco Vasko"
#define ExecutableName "Celestial-Rays-Demo.exe"

[Setup]
AppId={{6e4e392f1716421d9441ecea29692505}}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#Publisher}

DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}

OutputDir=output
OutputBaseFilename=Celestial-Rays-Demo

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

PrivilegesRequired=admin

Compression=lzma
SolidCompression=yes

WizardStyle=modern

UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\{#ExecutableName}

[Files]
Source: "{#BuildDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#ExecutableName}"; WorkingDir: "{app}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#ExecutableName}"; WorkingDir: "{app}"

[Run]
Filename: "{app}\{#ExecutableName}"; WorkingDir: "{app}"; Description: "Launch {#AppName}"; Flags: nowait postinstall skipifsilent
