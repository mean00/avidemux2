##########################
# Included files
##########################
!include Sections.nsh
!include MUI.nsh
!include WinMessages.nsh
!include revision.nsh

Name "Avidemux 2.4.2 r${REVISION}"

SetCompressor /SOLID lzma
SetCompressorDictSize 96

##########################
# Defines
##########################
!define INTERNALNAME "Avidemux 2.4"
!define REGKEY "SOFTWARE\${INTERNALNAME}"
!define UNINST_REGKEY "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${INTERNALNAME}"
!define VERSION 2.4.2.${REVISION}
!define COMPANY "Free Software Foundation"
!define URL "http://www.avidemux.org"

!define INSTALL_OPTS_INI "InstallOptions.ini"

##########################
# MUI defines
##########################
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install-blue-full.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "PageHeader.bmp"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${REGKEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME StartMenuGroup
!define MUI_STARTMENUPAGE_DEFAULTFOLDER Avidemux
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_WELCOMEFINISHPAGE_BITMAP "WelcomeFinishStrip.bmp"
!define MUI_UNICON "..\xpm\adm.ico"
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!define MUI_COMPONENTSPAGE_NODESC

##########################
# Variables
##########################
Var CreateDesktopIcon
Var CreateStartMenuGroup
Var CreateQuickLaunchIcon
Var StartMenuGroup

##########################
# Installer pages
##########################
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE License.rtf
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE CheckSelectedUIs
!insertmacro MUI_PAGE_COMPONENTS
Page custom InstallOptionsPage
!define MUI_PAGE_CUSTOMFUNCTION_PRE IsStartMenuRequired
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuGroup
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_PAGE_CUSTOMFUNCTION_PRE ActivateInternalSections
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION RunAvidemux
!define MUI_FINISHPAGE_RUN_TEXT "Run ${INTERNALNAME} now"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\Change Log.html"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "View Change Log now"
!define MUI_FINISHPAGE_LINK "Visit the Avidemux Win32 Builds website"
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.razorbyte.com.au/avidemux/"
!define MUI_PAGE_CUSTOMFUNCTION_PRE ConfigureFinishPage
!define MUI_PAGE_CUSTOMFUNCTION_SHOW OnShowFinishPage
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

##########################
# Installer languages
##########################
!insertmacro MUI_LANGUAGE English

##########################
#Reserve files
##########################
ReserveFile "${INSTALL_OPTS_INI}"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

##########################
# Installer attributes
##########################
OutFile avidemux_2.4_r${REVISION}_win32.exe
InstallDir "$PROGRAMFILES\Avidemux 2.4"
CRCCheck on
XPStyle on
ShowInstDetails nevershow
VIProductVersion 2.4.2.${REVISION}
VIAddVersionKey ProductName Avidemux
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey FileVersion ""
VIAddVersionKey FileDescription ""
VIAddVersionKey LegalCopyright ""
InstallDirRegKey HKLM "${REGKEY}" Path
ShowUninstDetails nevershow
BrandingText "Packaged by Gruntster"
InstType Standard
InstType Full

##########################
# Macros
##########################
!macro InstallGtkLanguage LANG_NAME LANG_CODE
	SetOverwrite on

	!insertmacro SectionFlagIsSet ${SecUiGtk} ${SF_SELECTED} installGtk${LANG_CODE} endGtk${LANG_CODE}

installGtk${LANG_CODE}:
    SetOutPath $INSTDIR\share\locale\${LANG_CODE}\LC_MESSAGES
    File ..\..\..\avidemux_2.4_build\share\locale\${LANG_CODE}\LC_MESSAGES\avidemux.mo
    File ..\..\..\avidemux_2.4_build\share\locale\${LANG_CODE}\LC_MESSAGES\gtk20.mo

    WriteRegStr HKLM "${REGKEY}\Components" ${LANG_NAME} 1
endGtk${LANG_CODE}:
!macroend

!macro InstallQt4Language LANG_NAME LANG_CODE
	SetOverwrite on

	!insertmacro SectionFlagIsSet ${SecUiQt4} ${SF_SELECTED} installQt4${LANG_CODE} endQt4${LANG_CODE}

installQt4${LANG_CODE}:	
	SetOutPath $INSTDIR\i18n
    File ..\..\..\avidemux_2.4_build\i18n\avidemux_${LANG_CODE}.qm
    File ..\..\..\avidemux_2.4_build\i18n\qt_${LANG_CODE}.qm
    
    WriteRegStr HKLM "${REGKEY}\Components" ${LANG_NAME} 1
endQt4${LANG_CODE}:    
!macroend

!macro UninstallLanguage LANG_NAME LANG_CODE
    RmDir /r /REBOOTOK $INSTDIR\share\locale\${LANG_CODE}
    Delete /REBOOTOK $INSTDIR\i18n\avidemux_${LANG_CODE}.qm
    Delete /REBOOTOK $INSTDIR\i18n\qt_${LANG_CODE}.qm
    
    DeleteRegValue HKLM "${REGKEY}\Components" ${LANG_NAME}
!macroend

# Macro for selecting sections based on registry setting
!macro SELECT_SECTION SECTION_NAME SECTION_ID
    Push $R0
    ReadRegStr $R0 HKLM "${REGKEY}\Components" "${SECTION_NAME}"
    StrCmp $R0 1 0 next${SECTION_ID}
    !insertmacro SelectSection "${SECTION_ID}"
    GoTo done${SECTION_ID}
next${SECTION_ID}:
    !insertmacro UnselectSection "${SECTION_ID}"
done${SECTION_ID}:
    Pop $R0
!macroend

##########################
# Installer sections
##########################
Section "Core files (required)" SecCore
    SectionIn 1 2 RO
    SetOutPath $INSTDIR
    SetOverwrite on
    File "..\..\..\avidemux_2.4_build\Build Info.txt"
    File "..\..\..\avidemux_2.4_build\Change Log.html"
    File ..\..\..\avidemux_2.4_build\zlib1.dll
    File ..\..\..\avidemux_2.4_build\freetype6.dll
    File ..\..\..\avidemux_2.4_build\iconv.dll
    File ..\..\..\avidemux_2.4_build\intl.dll
    File ..\..\..\avidemux_2.4_build\libaften.dll
    File ..\..\..\avidemux_2.4_build\libexpat.dll
    File ..\..\..\avidemux_2.4_build\libfaac.dll
    File ..\..\..\avidemux_2.4_build\libfaad2.dll
    File ..\..\..\avidemux_2.4_build\libfontconfig-1.dll
    File ..\..\..\avidemux_2.4_build\libglib-2.0-0.dll
    File ..\..\..\avidemux_2.4_build\libmp3lame-0.dll
    File ..\..\..\avidemux_2.4_build\libpng12-0.dll
    File ..\..\..\avidemux_2.4_build\libx264.dll
    File ..\..\..\avidemux_2.4_build\libxml2.dll
    File ..\..\..\avidemux_2.4_build\ogg.dll
    File ..\..\..\avidemux_2.4_build\pthreadGC2.dll
    File ..\..\..\avidemux_2.4_build\SDL.dll
    File ..\..\..\avidemux_2.4_build\vorbis.dll
    File ..\..\..\avidemux_2.4_build\vorbisenc.dll
    File ..\..\..\avidemux_2.4_build\xmltok.dll
    File ..\..\..\avidemux_2.4_build\xvidcore.dll
    SetOutPath $INSTDIR\etc\fonts
    File /r ..\..\..\avidemux_2.4_build\etc\fonts\*

    # if $PROFILE\avidemux exists and $APPDATA\avidemux doesn't, then move it
    IfFileExists "$PROFILE\avidemux" 0 end
    IfFileExists "$APPDATA\avidemux" end

    Rename "$PROFILE\avidemux" "$APPDATA\avidemux"

end:
    WriteRegStr HKLM "${REGKEY}\Components" "Core files (required)" 1
SectionEnd

SectionGroup /e "User interfaces" SecGrpUI
    Section "Command line" SecUiCli
        SectionIn 1 2
        SetOutPath $INSTDIR
        SetOverwrite on
        File ..\..\..\avidemux_2.4_build\avidemux2_cli.exe
        WriteRegStr HKLM "${REGKEY}\Components" "Command line" 1
    SectionEnd

    Section GTK+ SecUiGtk
        SectionIn 1 2
        SetOverwrite on
        SetOutPath $INSTDIR\etc\gtk-2.0
        File /r ..\..\..\avidemux_2.4_build\etc\gtk-2.0\*
        SetOutPath $INSTDIR\etc\pango
        File /r ..\..\..\avidemux_2.4_build\etc\pango\*
        SetOutPath $INSTDIR\lib\gtk-2.0
        File /r ..\..\..\avidemux_2.4_build\lib\gtk-2.0\*
        SetOutPath $INSTDIR\share\themes
        File /r ..\..\..\avidemux_2.4_build\share\themes\*
        SetOutPath $INSTDIR
        File ..\..\..\avidemux_2.4_build\avidemux2_gtk.exe
        File ..\..\..\avidemux_2.4_build\gtk2_prefs.exe
        File ..\..\..\avidemux_2.4_build\libatk-1.0-0.dll
        File ..\..\..\avidemux_2.4_build\libcairo-2.dll
        File ..\..\..\avidemux_2.4_build\libgdk_pixbuf-2.0-0.dll
        File ..\..\..\avidemux_2.4_build\libgdk-win32-2.0-0.dll
        File ..\..\..\avidemux_2.4_build\libgmodule-2.0-0.dll
        File ..\..\..\avidemux_2.4_build\libgobject-2.0-0.dll
        File ..\..\..\avidemux_2.4_build\libgthread-2.0-0.dll
        File ..\..\..\avidemux_2.4_build\libgtk-win32-2.0-0.dll
        File ..\..\..\avidemux_2.4_build\libpango-1.0-0.dll
        File ..\..\..\avidemux_2.4_build\libpangocairo-1.0-0.dll
        File ..\..\..\avidemux_2.4_build\libpangowin32-1.0-0.dll
        WriteRegStr HKLM "${REGKEY}\Components" GTK+ 1
    SectionEnd
    
    Section Qt4 SecUiQt4
        SectionIn 2
        SetOutPath $INSTDIR
        SetOverwrite on
        File ..\..\..\avidemux_2.4_build\QtGui4.dll
        File ..\..\..\avidemux_2.4_build\avidemux2_qt4.exe
        File ..\..\..\avidemux_2.4_build\mingwm10.dll
        File ..\..\..\avidemux_2.4_build\QtCore4.dll
        WriteRegStr HKLM "${REGKEY}\Components" Qt4 1
    SectionEnd    
SectionGroupEnd

SectionGroup "Additional languages" SecGrpLang
    Section "Catalan (GTK+ only)" SecLangCatalan
        SectionIn 2
        !insertmacro InstallGtkLanguage Catalan ca
    SectionEnd

    Section "Czech (GTK+ only)" SecLangCzech
        SectionIn 2
        !insertmacro InstallGtkLanguage Czech cs
    SectionEnd

    Section "French" SecLangFrench
        SectionIn 2
        !insertmacro InstallGtkLanguage French fr
        !insertmacro InstallQt4Language French fr
    SectionEnd

    Section "German (GTK+ only)" SecLangGerman
        SectionIn 2
        !insertmacro InstallGtkLanguage German de
    SectionEnd

    Section "Greek (GTK+ only)" SecLangGreek
        SectionIn 2
        !insertmacro InstallGtkLanguage Greek el
    SectionEnd

    Section "Italian" SecLangItalian
        SectionIn 2
        !insertmacro InstallGtkLanguage Italian it
        !insertmacro InstallQt4Language Italian it
    SectionEnd

    Section "Japanese (GTK+ only)" SecLangJapanese
        SectionIn 2
        !insertmacro InstallGtkLanguage Japanese ja
    SectionEnd

    Section "Russian (GTK+ only)" SecLangRussian
        SectionIn 2
        !insertmacro InstallGtkLanguage Russian ru
    SectionEnd

    Section "Serbian Cyrillic (GTK+ only)" SecLangSerbianCyrillic
        SectionIn 2
        !insertmacro InstallGtkLanguage SerbianCyrillic sr
    SectionEnd

    Section "Serbian Latin (GTK+ only)" SecLangSerbianLatin
        SectionIn 2
        !insertmacro InstallGtkLanguage SerbianLatin sr@latin
    SectionEnd

    Section "Spanish (GTK+ only)" SecLangSpanish
        SectionIn 2
        !insertmacro InstallGtkLanguage Spanish es
    SectionEnd

    Section "Turkish (GTK+ only)" SecLangTurkish
        SectionIn 2
        !insertmacro InstallGtkLanguage Turkish tr
    SectionEnd
SectionGroupEnd

Section AvsProxy SecAvsProxy
    SectionIn 2
    SetOutPath $INSTDIR
    SetOverwrite on
    File /r ..\..\..\avidemux_2.4_build\avsproxy.exe
    File /r ..\..\..\avidemux_2.4_build\avsproxy_gui.exe
    WriteRegStr HKLM "${REGKEY}\Components" "AvsProxy" 1
SectionEnd

Section "Sample external filter" SecFilter
    SectionIn 2
    SetOutPath $INSTDIR\plugin
    SetOverwrite on
    File /r ..\..\..\avidemux_2.4_build\plugin\*
    WriteRegStr HKLM "${REGKEY}\Components" "Sample external filter" 1
SectionEnd

Section "-Start menu Change Log" SecStartMenuChangeLog
    CreateDirectory $SMPROGRAMS\$StartMenuGroup
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetOutPath $INSTDIR
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Change Log.lnk" "$INSTDIR\Change Log.html"
    !insertmacro MUI_STARTMENU_WRITE_END
    WriteRegStr HKLM "${REGKEY}\Components" "Start menu" 1
SectionEnd

Section "-Start menu GTK+" SecStartMenuGtk
    CreateDirectory $SMPROGRAMS\$StartMenuGroup
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetOutPath $INSTDIR
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\${INTERNALNAME} GTK+.lnk" $INSTDIR\avidemux2_gtk.exe
    !insertmacro MUI_STARTMENU_WRITE_END
    WriteRegStr HKLM "${REGKEY}\Components" "Start menu" 1
SectionEnd

Section "-Start menu Qt4" SecStartMenuQt4
    CreateDirectory $SMPROGRAMS\$StartMenuGroup
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetOutPath $INSTDIR
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\${INTERNALNAME} Qt4.lnk" $INSTDIR\avidemux2_qt4.exe
    !insertmacro MUI_STARTMENU_WRITE_END
    WriteRegStr HKLM "${REGKEY}\Components" "Start menu" 1
SectionEnd

Section "-Start menu AVS Proxy GUI" SecStartMenuAvsProxyGui
    CreateDirectory $SMPROGRAMS\$StartMenuGroup
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetOutPath $INSTDIR
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\AVS Proxy GUI.lnk" "$INSTDIR\avsproxy_gui.exe"
    !insertmacro MUI_STARTMENU_WRITE_END
    WriteRegStr HKLM "${REGKEY}\Components" "Start menu" 1
SectionEnd

Section "-Quick Launch GTK+" SecQuickLaunchGtk
    SetOutPath $INSTDIR
    CreateShortcut "$QUICKLAUNCH\${INTERNALNAME} GTK+.lnk" $INSTDIR\avidemux2_gtk.exe
    WriteRegStr HKLM "${REGKEY}\Components" "Quick Launch" 1
SectionEnd

Section "-Quick Launch Qt4" SecQuickLaunchQt4
    SetOutPath $INSTDIR
    CreateShortcut "$QUICKLAUNCH\${INTERNALNAME} Qt4.lnk" $INSTDIR\avidemux2_qt4.exe
    WriteRegStr HKLM "${REGKEY}\Components" "Quick Launch" 1
SectionEnd

Section "-Desktop GTK+" SecDesktopGtk
    SetOutPath $INSTDIR
    CreateShortcut "$DESKTOP\${INTERNALNAME} GTK+.lnk" $INSTDIR\avidemux2_gtk.exe
    WriteRegStr HKLM "${REGKEY}\Components" "Desktop" 1
SectionEnd

Section "-Desktop Qt4" SecDesktopQt4
    SetOutPath $INSTDIR
    CreateShortcut "$DESKTOP\${INTERNALNAME} Qt4.lnk" $INSTDIR\avidemux2_qt4.exe
    WriteRegStr HKLM "${REGKEY}\Components" "Desktop" 1
SectionEnd

Section -post SecUninstaller
    SectionIn 1 2
    WriteRegStr HKLM "${REGKEY}" Path $INSTDIR
    SetOutPath $INSTDIR
    WriteUninstaller $INSTDIR\uninstall.exe
    WriteRegStr HKLM "${UNINST_REGKEY}" DisplayName "${INTERNALNAME}"
    WriteRegStr HKLM "${UNINST_REGKEY}" DisplayVersion "${VERSION}"
    WriteRegStr HKLM "${UNINST_REGKEY}" DisplayIcon $INSTDIR\uninstall.exe
    WriteRegStr HKLM "${UNINST_REGKEY}" UninstallString $INSTDIR\uninstall.exe
    WriteRegDWORD HKLM "${UNINST_REGKEY}" NoModify 1
    WriteRegDWORD HKLM "${UNINST_REGKEY}" NoRepair 1
SectionEnd

##########################
# Uninstaller sections
##########################
Section /o "un.Sample external filter" UnSecFilter
    RmDir /r /REBOOTOK $INSTDIR\plugin
    DeleteRegValue HKLM "${REGKEY}\Components" "Sample external filter"
SectionEnd

Section /o "un.AvsProxy" UnSecAvsProxy
    Delete /REBOOTOK $INSTDIR\avsproxy.exe
    Delete /REBOOTOK $INSTDIR\avsproxy_gui.exe
    DeleteRegValue HKLM "${REGKEY}\Components" "AvsProxy"
SectionEnd

Section /o un.Catalan UnSecLangCatalan
	!insertmacro UninstallLanguage Catalan ca
SectionEnd

Section /o un.Czech UnSecLangCzech
	!insertmacro UninstallLanguage Czech cs
SectionEnd

Section /o un.French UnSecLangFrench
	!insertmacro UninstallLanguage French fr
SectionEnd

Section /o un.German UnSecLangGerman
	!insertmacro UninstallLanguage German de
SectionEnd

Section /o un.Greek UnSecLangGreek
	!insertmacro UninstallLanguage Greek el
SectionEnd

Section /o un.Italian UnSecLangItalian
	!insertmacro UninstallLanguage Italian it
SectionEnd

Section /o un.Japanese UnSecLangJapanese
	!insertmacro UninstallLanguage Japanese ja
SectionEnd

Section /o un.Russian UnSecLangRussian
	!insertmacro UninstallLanguage Russian ru
SectionEnd

Section /o un.Serbian UnSecLangSerbianCyrillic
	!insertmacro UninstallLanguage SerbianCyrillic sr
SectionEnd

Section /o un.SerbianLatin UnSecLangSerbianLatin
	!insertmacro UninstallLanguage SerbianLatin sr@latin
SectionEnd

Section /o un.Spanish UnSecLangSpanish
	!insertmacro UninstallLanguage Spanish es
SectionEnd

Section /o un.Turkish UnSecLangTurkish
	!insertmacro UninstallLanguage Turkish tr
SectionEnd

Section /o un.GTK+ UnSecUiGtk
    Delete /REBOOTOK $INSTDIR\libpangowin32-1.0-0.dll
    Delete /REBOOTOK $INSTDIR\libpangocairo-1.0-0.dll
    Delete /REBOOTOK $INSTDIR\libpango-1.0-0.dll
    Delete /REBOOTOK $INSTDIR\libgtk-win32-2.0-0.dll
    Delete /REBOOTOK $INSTDIR\libgthread-2.0-0.dll
    Delete /REBOOTOK $INSTDIR\libgobject-2.0-0.dll
    Delete /REBOOTOK $INSTDIR\libgmodule-2.0-0.dll
    Delete /REBOOTOK $INSTDIR\libgdk-win32-2.0-0.dll
    Delete /REBOOTOK $INSTDIR\libgdk_pixbuf-2.0-0.dll
    Delete /REBOOTOK $INSTDIR\libcairo-2.dll
    Delete /REBOOTOK $INSTDIR\libatk-1.0-0.dll
    Delete /REBOOTOK $INSTDIR\gtk2_prefs.exe
    Delete /REBOOTOK $INSTDIR\avidemux2_gtk.exe
    RmDir /r /REBOOTOK $INSTDIR\share\themes
    RmDir /r /REBOOTOK $INSTDIR\lib\gtk-2.0
    RmDir /r /REBOOTOK $INSTDIR\etc\gtk-2.0
    RmDir /r /REBOOTOK $INSTDIR\etc\pango
    DeleteRegValue HKLM "${REGKEY}\Components" GTK+
SectionEnd

Section /o un.Qt4 UnSecUiQt4
    Delete /REBOOTOK $INSTDIR\QtCore4.dll
    Delete /REBOOTOK $INSTDIR\mingwm10.dll
    Delete /REBOOTOK $INSTDIR\avidemux2_qt4.exe
    Delete /REBOOTOK $INSTDIR\QtGui4.dll
    DeleteRegValue HKLM "${REGKEY}\Components" Qt4
SectionEnd

Section /o "un.Command line" UnSecUiCli
    Delete /REBOOTOK $INSTDIR\avidemux2_cli.exe
    DeleteRegValue HKLM "${REGKEY}\Components" "Command line"
SectionEnd

Section /o "un.Core files (required)" UnSecCore
    Delete /REBOOTOK $INSTDIR\xvidcore.dll
    Delete /REBOOTOK $INSTDIR\xmltok.dll
    Delete /REBOOTOK $INSTDIR\vorbisenc.dll
    Delete /REBOOTOK $INSTDIR\vorbis.dll
    Delete /REBOOTOK $INSTDIR\SDL.dll
    Delete /REBOOTOK $INSTDIR\pthreadGC2.dll
    Delete /REBOOTOK $INSTDIR\ogg.dll
    Delete /REBOOTOK $INSTDIR\libxml2.dll
    Delete /REBOOTOK $INSTDIR\libx264.dll
    Delete /REBOOTOK $INSTDIR\libpng12-0.dll
    Delete /REBOOTOK $INSTDIR\libmp3lame-0.dll
    Delete /REBOOTOK $INSTDIR\libglib-2.0-0.dll
    Delete /REBOOTOK $INSTDIR\libfontconfig-1.dll
    Delete /REBOOTOK $INSTDIR\libfaad2.dll
    Delete /REBOOTOK $INSTDIR\libfaac.dll
    Delete /REBOOTOK $INSTDIR\libexpat.dll
    Delete /REBOOTOK $INSTDIR\libaften.dll
    Delete /REBOOTOK $INSTDIR\intl.dll
    Delete /REBOOTOK $INSTDIR\iconv.dll
    Delete /REBOOTOK $INSTDIR\freetype6.dll
    Delete /REBOOTOK $INSTDIR\zlib1.dll
    Delete /REBOOTOK "$INSTDIR\Build Info.txt"
    Delete /REBOOTOK "$INSTDIR\Change Log.html"
    Delete /REBOOTOK "$INSTDIR\stdout.txt"
    Delete /REBOOTOK "$INSTDIR\stderr.txt"
    RmDir /r /REBOOTOK $INSTDIR\etc\fonts
    DeleteRegValue HKLM "${REGKEY}\Components" Avidemux
    
    RmDir /REBOOTOK $INSTDIR\share\gettext
    RmDir /REBOOTOK $INSTDIR\share
SectionEnd

Section /o "un.Start menu" UnSecStartMenu
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Change Log.lnk"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\${INTERNALNAME} GTK+.lnk"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\${INTERNALNAME} Qt4.lnk"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\AVS Proxy GUI.lnk"
SectionEnd

Section /o "un.Quick Launch" UnSecQuickLaunch
    Delete /REBOOTOK "$QUICKLAUNCH\${INTERNALNAME} GTK+.lnk"
    Delete /REBOOTOK "$QUICKLAUNCH\${INTERNALNAME} Qt4.lnk"
SectionEnd

Section /o "un.Desktop" UnSecDesktop
    Delete /REBOOTOK "$DESKTOP\${INTERNALNAME} GTK+.lnk"
    Delete /REBOOTOK "$DESKTOP\${INTERNALNAME} Qt4.lnk"
SectionEnd

Section un.post UnSecUninstaller
    RmDir /REBOOTOK $INSTDIR\etc
    RmDir /REBOOTOK $INSTDIR\i18n
    RmDir /REBOOTOK $INSTDIR\lib
    RmDir /REBOOTOK $INSTDIR\share\locale
    RmDir /REBOOTOK $INSTDIR\share
    
    DeleteRegKey HKLM "${UNINST_REGKEY}"
    Delete /REBOOTOK $INSTDIR\uninstall.exe
    DeleteRegValue HKLM "${REGKEY}" StartMenuGroup
    DeleteRegValue HKLM "${REGKEY}" Path
    DeleteRegKey /IfEmpty HKLM "${REGKEY}\Components"
    DeleteRegKey /IfEmpty HKLM "${REGKEY}"
    RmDir /REBOOTOK $SMPROGRAMS\$StartMenuGroup
    RmDir /REBOOTOK $INSTDIR
    Push $R0
    StrCpy $R0 $StartMenuGroup 1
    StrCmp $R0 ">" no_smgroup
no_smgroup:
    Pop $R0
SectionEnd

##########################
# Installer functions
##########################
Function .onInit
    ReadRegStr $R0  HKLM "${UNINST_REGKEY}" "UninstallString"
    StrCmp $R0 "" startInstall
 
    MessageBox MB_YESNO|MB_ICONEXCLAMATION "${INTERNALNAME} has already been installed. $\n$\nDo you want to remove \
      the previous version before installing $(^Name)?" IDNO startInstall
  
    # Run the uninstaller
    ClearErrors
    ExecWait '$R0 _?=$INSTDIR' ; Do not copy the uninstaller to a temp file

startInstall:
    InitPluginsDir
    
    !insertmacro MUI_INSTALLOPTIONS_EXTRACT "${INSTALL_OPTS_INI}"

    # Make sure a User Interface is selected in previous install preferences
    ReadRegStr $0 HKLM "${REGKEY}\Components" "Command line"
    StrCmp $0 1 populate
    ReadRegStr $0 HKLM "${REGKEY}\Components" "GTK+"
    StrCmp $0 1 populate
    ReadRegStr $0 HKLM "${REGKEY}\Components" "Qt4"
    StrCmp $0 1 populate

    # No UI exists, so go with defaults
    Goto end

    #Select sections based on last install
populate:
    !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuGroup
    !insertmacro SELECT_SECTION "Command line" ${SecUiCli}
    !insertmacro SELECT_SECTION Qt4 ${SecUiQt4}
    !insertmacro SELECT_SECTION GTK+ ${SecUiGtk}
    !insertmacro SELECT_SECTION Catalan ${SecLangCatalan}
    !insertmacro SELECT_SECTION Czech ${SecLangCzech}
    !insertmacro SELECT_SECTION French ${SecLangFrench}
    !insertmacro SELECT_SECTION German ${SecLangGerman}
    !insertmacro SELECT_SECTION Greek ${SecLangGreek}
    !insertmacro SELECT_SECTION Italian ${SecLangItalian}
    !insertmacro SELECT_SECTION Japanese ${SecLangJapanese}
    !insertmacro SELECT_SECTION Russian ${SecLangRussian}
    !insertmacro SELECT_SECTION SerbianCyrillic ${SecLangSerbianCyrillic}
    !insertmacro SELECT_SECTION SerbianLatin ${SecLangSerbianLatin}
    !insertmacro SELECT_SECTION Spanish ${SecLangSpanish}
    !insertmacro SELECT_SECTION Turkish ${SecLangTurkish}
    !insertmacro SELECT_SECTION "Sample external filter" ${SecFilter}
    !insertmacro SELECT_SECTION AvsProxy ${SecAvsProxy}

    #startMenu:
    ReadRegStr $0 HKLM "${REGKEY}\Components" "Start menu"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "${INSTALL_OPTS_INI}" "Field 3" "State" $0
    
    #desktop:
    ReadRegStr $0 HKLM "${REGKEY}\Components" "Desktop"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "${INSTALL_OPTS_INI}" "Field 2" "State" $0
    
    #quickLaunch:
    ReadRegStr $0 HKLM "${REGKEY}\Components" "Quick Launch"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "${INSTALL_OPTS_INI}" "Field 4" "State" $0
    
end:
FunctionEnd

Function .onSelChange
	!insertmacro SectionFlagIsSet ${SecUiGtk} ${SF_SELECTED} end checkCLI
checkCLI:
	!insertmacro SectionFlagIsSet ${SecUiCli} ${SF_SELECTED} end disable
	
disable:  # GTK langs only
	SectionSetFlags ${SecLangCatalan} SF_RO
	SectionSetFlags ${SecLangCzech} SF_RO
	SectionSetFlags ${SecLangFrench} SF_RO
	SectionSetFlags ${SecLangGerman} SF_RO
	SectionSetFlags ${SecLangGreek} SF_RO	
	SectionSetFlags ${SecLangJapanese} SF_RO
	SectionSetFlags ${SecLangRussian} SF_RO
	SectionSetFlags ${SecLangSerbianCyrillic} SF_RO
	SectionSetFlags ${SecLangSerbianLatin} SF_RO
	SectionSetFlags ${SecLangSpanish} SF_RO
	SectionSetFlags ${SecLangTurkish} SF_RO
end:
FunctionEnd

Function CheckSelectedUIs
	!insertmacro SectionFlagIsSet ${SecGrpUI} ${SF_SELECTED} end checkPartial
checkPartial:
	!insertmacro SectionFlagIsSet ${SecGrpUI} ${SF_PSELECTED} end displayError
displayError:
    MessageBox MB_OK|MB_ICONSTOP "At least one User Interface must be selected."
    Abort
end:
FunctionEnd

LangString INSTALL_OPTS_PAGE_TITLE ${LANG_ENGLISH} "Choose Install Options"
LangString INSTALL_OPTS_PAGE_SUBTITLE ${LANG_ENGLISH} "Choose where to install Avidemux icons."

Function InstallOptionsPage
    Call IsInstallOptionsRequired

    !insertmacro MUI_HEADER_TEXT "$(INSTALL_OPTS_PAGE_TITLE)" "$(INSTALL_OPTS_PAGE_SUBTITLE)"
    !insertmacro MUI_INSTALLOPTIONS_DISPLAY "${INSTALL_OPTS_INI}"
    
    Call ReadInstallOptions
FunctionEnd

Function IsInstallOptionsRequired
	!insertmacro SectionFlagIsSet ${SecUiGtk} ${SF_SELECTED} end resetOptions
	!insertmacro SectionFlagIsSet ${SecUiQt4} ${SF_SELECTED} end resetOptions

resetOptions:
    StrCpy $CreateDesktopIcon 0
    StrCpy $CreateStartMenuGroup 0
    StrCpy $CreateQuickLaunchIcon 0
    Abort

end:
FunctionEnd

Function ReadInstallOptions
    !insertmacro MUI_INSTALLOPTIONS_READ $CreateDesktopIcon "${INSTALL_OPTS_INI}" "Field 2" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $CreateStartMenuGroup "${INSTALL_OPTS_INI}" "Field 3" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $CreateQuickLaunchIcon "${INSTALL_OPTS_INI}" "Field 4" "State"
FunctionEnd

Function IsStartMenuRequired
    StrCmp $CreateStartMenuGroup 1 +2
        Abort    
FunctionEnd

Function ActivateInternalSections
    #AVS Proxy GUI shortcut:
    SectionGetFlags ${SecAvsProxy} $0
    IntOp $0 $0 & ${SF_SELECTED}
    IntOp $0 $0 & $CreateStartMenuGroup
    SectionSetFlags ${SecStartMenuAvsProxyGui} $0
    
    #Change Log shortcut:
    SectionSetFlags ${SecStartMenuChangeLog} $CreateStartMenuGroup

    #GTK shortcuts:
    SectionGetFlags ${SecUiGtk} $0
    IntOp $0 $0 & ${SF_SELECTED}

    IntOp $1 $0 & $CreateDesktopIcon
    SectionSetFlags ${SecDesktopGtk} $1
    
    IntOp $1 $0 & $CreateQuickLaunchIcon
    SectionSetFlags ${SecQuickLaunchGtk} $1
    
    IntOp $1 $0 & $CreateStartMenuGroup
    SectionSetFlags ${SecStartMenuGtk} $1
    
    #Qt4 shortcuts:
    SectionGetFlags ${SecUiQt4} $0
    IntOp $0 $0 & ${SF_SELECTED}
    
    IntOp $1 $0 & $CreateDesktopIcon
    SectionSetFlags ${SecDesktopQt4} $1
    
    IntOp $1 $0 & $CreateQuickLaunchIcon
    SectionSetFlags ${SecQuickLaunchQt4} $1
    
    IntOp $1 $0 & $CreateStartMenuGroup
    SectionSetFlags ${SecStartMenuQt4} $1
FunctionEnd

Function ConfigureFinishPage
    SectionGetFlags ${SecUiGtk} $0
    IntOp $0 $0 & ${SF_SELECTED}
    StrCmp $0 ${SF_SELECTED} end
    
    SectionGetFlags ${SecUiQt4} $0
    IntOp $0 $0 & ${SF_SELECTED}
    StrCmp $0 ${SF_SELECTED} end

    DeleteINISec "$PLUGINSDIR\ioSpecial.ini" "Field 4"
        
end:
FunctionEnd

Function OnShowFinishPage
    # Make hyperlink underlined to keep it consistent with web browsers
    GetDlgItem $0 $MUI_HWND 1205
    CreateFont $1 "$(^Font)" 9 500 /UNDERLINE
    SendMessage $0 ${WM_SETFONT} $1 0    
FunctionEnd

Function RunAvidemux
    SetOutPath $INSTDIR

#GTK:
    SectionGetFlags ${SecUiGtk} $0
    IntOp $0 $0 & ${SF_SELECTED}
    
    StrCmp $0 ${SF_SELECTED} 0 Qt4
        Exec "$INSTDIR\avidemux2_gtk.exe"

    Goto end
    
Qt4:
    SectionGetFlags ${SecUiQt4} $0
    IntOp $0 $0 & ${SF_SELECTED}
    
    StrCmp $0 ${SF_SELECTED} 0 End
        Exec "$INSTDIR\avidemux2_qt4.exe"    
    
end:
FunctionEnd

##########################
# Uninstaller functions
##########################
Function un.onInit
    ReadRegStr $INSTDIR HKLM "${REGKEY}" Path
    !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuGroup
    !insertmacro SELECT_SECTION "Core files (required)" ${UnSecCore}
    !insertmacro SELECT_SECTION "Command line" ${UnSecUiCli}
    !insertmacro SELECT_SECTION Qt4 ${UnSecUiQt4}
    !insertmacro SELECT_SECTION GTK+ ${UnSecUiGtk}
    !insertmacro SELECT_SECTION Catalan ${UnSecLangCatalan}
    !insertmacro SELECT_SECTION Czech ${UnSecLangCzech}
    !insertmacro SELECT_SECTION French ${UnSecLangFrench}
    !insertmacro SELECT_SECTION German ${UnSecLangGerman}
    !insertmacro SELECT_SECTION Greek ${UnSecLangGreek}
    !insertmacro SELECT_SECTION Italian ${UnSecLangItalian}
    !insertmacro SELECT_SECTION Japanese ${UnSecLangJapanese}
    !insertmacro SELECT_SECTION Russian ${UnSecLangRussian}
    !insertmacro SELECT_SECTION SerbianCyrillic ${UnSecLangSerbianCyrillic}
    !insertmacro SELECT_SECTION SerbianLatin ${UnSecLangSerbianLatin}
    !insertmacro SELECT_SECTION Spanish ${UnSecLangSpanish}
    !insertmacro SELECT_SECTION Turkish ${UnSecLangTurkish}
    !insertmacro SELECT_SECTION AvsProxy ${UnSecAvsProxy}
    !insertmacro SELECT_SECTION "Sample external filter" ${UnSecFilter}
    !insertmacro SELECT_SECTION "Start menu" ${UnSecStartMenu}
    !insertmacro SELECT_SECTION "Quick Launch" ${UnSecQuickLaunch}
    !insertmacro SELECT_SECTION "Desktop" ${UnSecDesktop}
FunctionEnd
