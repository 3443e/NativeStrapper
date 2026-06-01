!include "MUI2.nsh"
!include "Sections.nsh"

Name "NativeStrapper"
OutFile "NativeStrapper-Setup.exe"
InstallDir "$LOCALAPPDATA\NativeStrapper"
RequestExecutionLevel user

!define MUI_ICON "assets\NativeStrapper.ico"
!define MUI_UNICON "assets\NativeStrapper.ico"

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section "NativeStrapper" SecMain
    SectionIn RO ; required, can't uncheck
    SetOutPath "$INSTDIR"
    File /r "release\*.*"
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\NativeStrapper" "DisplayName" "NativeStrapper"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\NativeStrapper" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\NativeStrapper" "InstallLocation" "$INSTDIR"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\NativeStrapper" "DisplayIcon" "$INSTDIR\nativestrapper.exe"
SectionEnd

Section "Desktop Shortcut" SecDesktop
    CreateShortcut "$DESKTOP\NativeStrapper.lnk" "$INSTDIR\nativestrapper.exe" "" "$INSTDIR\nativestrapper.exe"
SectionEnd

Section "Start Menu Shortcut" SecStartMenu
    CreateDirectory "$SMPROGRAMS\NativeStrapper"
    CreateShortcut "$SMPROGRAMS\NativeStrapper\NativeStrapper.lnk" "$INSTDIR\nativestrapper.exe" "" "$INSTDIR\nativestrapper.exe"
    CreateShortcut "$SMPROGRAMS\NativeStrapper\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

LangString DESC_SecMain ${LANG_ENGLISH} "NativeStrapper core files."
LangString DESC_SecDesktop ${LANG_ENGLISH} "Create a shortcut on the desktop."
LangString DESC_SecStartMenu ${LANG_ENGLISH} "Create a shortcut in the Start Menu."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
    Delete "$INSTDIR\Uninstall.exe"
    Delete "$DESKTOP\NativeStrapper.lnk"
    Delete "$SMPROGRAMS\NativeStrapper\NativeStrapper.lnk"
    Delete "$SMPROGRAMS\NativeStrapper\Uninstall.lnk"
    RMDir "$SMPROGRAMS\NativeStrapper"
    RMDir /r "$INSTDIR"
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\NativeStrapper"
SectionEnd