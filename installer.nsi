!include "MUI2.nsh"

Name "NativeStrapper"
OutFile "NativeStrapper-Setup.exe"
InstallDir "$LOCALAPPDATA\NativeStrapper"
RequestExecutionLevel user

!define MUI_ICON "assets\NativeStrapper.ico"
!define MUI_UNICON "assets\NativeStrapper.ico"

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section "Install"
    SetOutPath "$INSTDIR"
    File /r "release\*.*"
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\NativeStrapper" "DisplayName" "NativeStrapper"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\NativeStrapper" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\NativeStrapper" "InstallLocation" "$INSTDIR"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\NativeStrapper" "DisplayIcon" "$INSTDIR\nativestrapper.exe"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir /r "$INSTDIR"
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\NativeStrapper"
SectionEnd