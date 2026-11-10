#define MyAppName "VoidClip"
#define MyAppVersion "1.0.0"
#ifndef SourceDir
  #define SourceDir "..\dist\VoidClip.vst3"
#endif

[Setup]
AppId={{B6E8A1C2-5D3F-4B7A-9C1E-7A4F2D9E0B31}
AppName={#MyAppName} VST3
AppVersion={#MyAppVersion}
AppPublisher=VoidAudio
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
CreateAppDir=yes
OutputDir=..\dist
OutputBaseFilename=VoidClip-Setup-{#MyAppVersion}
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
UninstallDisplayName={#MyAppName} VST3

[Files]
Source: "{#SourceDir}\*"; DestDir: "{commoncf64}\VST3\VoidClip.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Messages]
FinishedLabel=VoidClip wurde in C:\Program Files\Common Files\VST3 installiert.%nStarte deine DAW und scanne nach neuen Plugins.
