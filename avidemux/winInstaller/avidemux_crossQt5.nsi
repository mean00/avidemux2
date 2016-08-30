##########################
# Included files
##########################
!addincludedir ${NSIDIR}
!addplugindir ${NSIDIR}/plugin

!include Sections.nsh
!include MUI2.nsh
!include nsDialogs.nsh
!include Memento.nsh
!include FileFunc.nsh
!include UAC.nsh
!include WinVer.nsh
!include WordFunc.nsh
!include AvidemuxVersion.nsh
SetCompressor /SOLID lzma
SetCompressorDictSize 96
RequestExecutionLevel user

##########################
# Defines
##########################
!define REVISION ${SVN_VERSION}
!define EXEDIR "${NSIDIR}/install"

!define PRODUCT_VERSION "${CORE_VERSION}.${POINT_RELEASE}.${SVN_VERSION}"
!define PRODUCT_NAME "Avidemux ${CORE_VERSION} - ${BUILD_BITS} bits"
!define PRODUCT_FULLNAME "Avidemux ${PRODUCT_VERSION} (${BUILD_BITS}-bit Release)"

!if ${BUILD_BITS} == 64
    !define SHORTCUT_NAME "${PRODUCT_NAME}"
    !define REG_GROUPNAME "${PRODUCT_NAME} (${BUILD_BITS}-bit)"
!else
    !define SHORTCUT_NAME "${PRODUCT_NAME} (${BUILD_BITS}-bit)"
    !define REG_GROUPNAME "${PRODUCT_NAME}"
!endif

!define REGKEY "SOFTWARE\${REG_GROUPNAME}"
!define UNINST_REGKEY "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${REG_GROUPNAME}"
!define COMPANY "Free Software Foundation"
!define URL "http://www.avidemux.org"

OutFile "${EXEDIR}/avidemux_${CORE_VERSION}.${POINT_RELEASE}_r${REVISION}_win${BUILD_BITS}.exe"
Name "${PRODUCT_FULLNAME}"

##########################
# Memento defines
##########################
!define MEMENTO_REGISTRY_ROOT HKLM
!define MEMENTO_REGISTRY_KEY "${REGKEY}"

##########################
# MUI defines
##########################
!define MUI_ICON "${NSIDIR}/../common/xpm/adm.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "${NSIDIR}/PageHeader.bmp"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${REGKEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME StartMenuGroup
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Avidemux (${BUILD_BITS} bits)"
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSIDIR}/WelcomeFinishStrip.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSIDIR}/WelcomeFinishStrip.bmp"
!define MUI_UNICON "${NSIDIR}/../common/xpm/adm.ico"
!define MUI_COMPONENTSPAGE_NODESC

##########################
# Variables
##########################
Var CreateDesktopIcon
Var CreateStartMenuGroup
Var CreateQuickLaunchIcon
Var StartMenuGroup
Var PreviousVersion
Var PreviousVersionState
Var ReinstallUninstall

##########################
# Installer pages
##########################
!define MUI_WELCOMEPAGE_TITLE "${PRODUCT_FULLNAME} Setup Wizard"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${NSIDIR}/License.rtf"
 Page custom ReinstallPage ReinstallPageLeave
!insertmacro MUI_PAGE_COMPONENTS
Page custom InstallOptionsPage
!define MUI_PAGE_CUSTOMFUNCTION_PRE IsStartMenuRequired
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuGroup
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_PAGE_CUSTOMFUNCTION_PRE ActivateInternalSections
!define MUI_PAGE_CUSTOMFUNCTION_SHOW InstFilesPageShow
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE InstFilesPageLeave
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION RunAvidemux
!define MUI_FINISHPAGE_RUN_TEXT "Run ${PRODUCT_NAME} now"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR/ChangeLog.html"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "View Change Log now"
!define MUI_FINISHPAGE_LINK "Visit the Avidemux website"
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.avidemux.org"
!define MUI_PAGE_CUSTOMFUNCTION_PRE ConfigureFinishPage
!insertmacro MUI_PAGE_FINISH

!define MUI_PAGE_CUSTOMFUNCTION_PRE un.ConfirmPagePre
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_PAGE_CUSTOMFUNCTION_PRE un.FinishPagePre
!insertmacro MUI_UNPAGE_FINISH

##########################
# Installer languages
##########################
!insertmacro MUI_LANGUAGE English

##########################
# Installer attributes
##########################
!if ${BUILD_BITS} == 64
    InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"
!else
    InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
!endif

CRCCheck on
XPStyle on
ShowInstDetails nevershow
ShowUninstDetails nevershow
VIProductVersion ${PRODUCT_VERSION}
VIAddVersionKey ProductName Avidemux
VIAddVersionKey ProductVersion "${PRODUCT_VERSION}"
VIAddVersionKey FileVersion ""
VIAddVersionKey FileDescription ""
VIAddVersionKey LegalCopyright ""
InstallDirRegKey HKLM "${REGKEY}" Path
BrandingText "Packaged by Mean/Gruntster"
InstType Standard
InstType Full
#########################################
#
#########################################
Function GetAfterChar
  Exch $0 ; chop char
  Exch
  Exch $1 ; input string
  Push $2
  Push $3
  StrCpy $2 0
  loop:
    IntOp $2 $2 - 1
    StrCpy $3 $1 1 $2
    StrCmp $3 "" 0 +3
      StrCpy $0 ""
      Goto exit2
    StrCmp $3 $0 exit1
    Goto loop
  exit1:
    IntOp $2 $2 + 1
    StrCpy $0 $1 "" $2
  exit2:
    Pop $3
    Pop $2
    Pop $1
    Exch $0 ; output
FunctionEnd
##################################################################### 
!define StrReplace '!insertmacro "_Name"'

##########################
# Uninstaller macros
##########################
!insertmacro un.GetOptions
!insertmacro un.GetParameters

!define UninstallLogPath "$INSTDIR\uninstall.log"
Var UninstallLogHandle

; Uninstall log file missing.
LangString UninstallLogMissing ${LANG_ENGLISH} "uninstall.log not found!$\r$\nUninstallation cannot proceed!"
# Regexep Does not work with cross
# Use only one file at a time
!macro InstallFile FILEZ
    File "${FILEZ}"
    Push "${FILEZ}"
    Push "/"
    Call GetAfterChar
    Pop $R0
    FileWrite $UninstallLogHandle "$OUTDIR\$R0$\r$\n"
!macroend
!define File "!insertmacro InstallFile"
 
!macro InstallFolder FILEREGEX
    File /r "${FILEREGEX}/*"
    Push "$OUTDIR"
    Call InstallFolderInternal
!macroend
!define Folder "!insertmacro InstallFolder"
 
Function InstallFolderInternal
    Pop $9
    !define Index 'Line${__LINE__}'
    FindFirst $0 $1 "$9/*"
    StrCmp $0 "" "${Index}-End"
"${Index}-Loop:"
    StrCmp $1 "" "${Index}-End"
    StrCmp $1 "." "${Index}-Next"
    StrCmp $1 ".." "${Index}-Next"
    IfFileExists "$9\$1\*" 0 "${Index}-Write"
        Push $0
        Push $9
        Push "$9\$1"
        Call InstallFolderInternal
        Pop $9
        Pop $0
        Goto "${Index}-Next"
"${Index}-Write:"
    FileWrite $UninstallLogHandle "$9\$1$\r$\n"
"${Index}-Next:"
    FindNext $0 $1
    Goto "${Index}-Loop"
"${Index}-End:"
    !undef Index
FunctionEnd

; WriteUninstaller macro
!macro WriteUninstaller Path
    WriteUninstaller "${Path}"
    FileWrite $UninstallLogHandle "${Path}$\r$\n"
!macroend
!define WriteUninstaller "!insertmacro WriteUninstaller"

##########################
# Macros
##########################

!macro InstallQtLanguage LANG_NAME LANG_CODE
    SetOverwrite on

    !insertmacro SectionFlagIsSet ${SecUiQt} ${SF_SELECTED} installQt${LANG_CODE} endQt${LANG_CODE}

installQt${LANG_CODE}:
    SetOutPath $INSTDIR\qt5\i18n
    ${File} qt5/i18n/avidemux_${LANG_CODE}.qm
    ${File} qt5/i18n/qt_${LANG_CODE}.qm

endQt${LANG_CODE}:
!macroend
#
!macro InstallDev xname
         SetOutPath $INSTDIR\include\avidemux\2.6\${xname}
         ${File} ${DEV_FOLDER}/include/avidemux/2.6/${xname}/*
!macroend
!macro InstallDevQt5 xname
         SetOutPath $INSTDIR\include\avidemux\2.6\qt5\${xname}
         ${File} ${DEV_FOLDER}/include/avidemux/2.6/qt5/${xname}/*
!macroend



##########################
# Installer sections
##########################
Section -OpenLogFile
    CreateDirectory "$INSTDIR"
    FileOpen $UninstallLogHandle ${UninstallLogPath} a
    FileSeek $UninstallLogHandle 0 END
SectionEnd
Section  "User interfaces" SecGrpUI
        SectionIn  1 2 RO
        SetOutPath $INSTDIR
        SetOverwrite on
        ${File} ${ROOT_FOLDER}/avidemux_cli.exe
        ${File} ${ROOT_FOLDER}/avidemux.exe
        ${File} ${ROOT_FOLDER}/avidemux_jobs.exe
        SetOutPath $INSTDIR\qt5\i18n
        ${File} ${ROOT_FOLDER}/qt5/i18n/*.qm
SectionEnd
# !!
Section "Plugins" SecGrpPlugins
        SectionIn 1 2 RO
        SetOverwrite on
        SetOutPath $INSTDIR\plugins\audioDevices
        ${File} ${ROOT_FOLDER}/plugins/audioDevices/*.dll
        SetOutPath $INSTDIR\plugins\audioDecoder
        ${File} ${ROOT_FOLDER}/plugins/audioDecoder/*.dll
        SetOutPath $INSTDIR\plugins\audioEncoders
        ${File} ${ROOT_FOLDER}/plugins/audioEncoders/*.dll
        SetOutPath $INSTDIR\plugins\demuxers
        ${File} ${ROOT_FOLDER}/plugins/demuxers/*.dll
        SetOutPath $INSTDIR\plugins\muxers
        ${File} ${ROOT_FOLDER}/plugins/muxers/*.dll
        SetOutPath $INSTDIR\plugins\scriptEngines
        ${File} ${ROOT_FOLDER}/plugins/scriptEngines/libADM_script_tinyPy.dll
        SetOutPath $INSTDIR\plugins\autoScripts
        ${Folder} ${ROOT_FOLDER}/plugins/autoScripts
        SetOutPath $INSTDIR\plugins\videoEncoders
        ${File} ${ROOT_FOLDER}/plugins/videoEncoders/*.dll
        SetOutPath $INSTDIR\plugins\videoEncoders\qt5
        ${File} ${ROOT_FOLDER}/plugins/videoEncoders/qt5/*.dll
        #SetOutPath $INSTDIR\plugins\videoEncoders\cli
        #${File} ${ROOT_FOLDER}/plugins/videoEncoders/cli/*.dll
        SetOutPath $INSTDIR\plugins\videoFilters
        ${File} ${ROOT_FOLDER}/plugins/videoFilters/*.dll
        SetOutPath $INSTDIR\plugins\videoFilters\qt5
        ${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/*.dll
        SetOutPath $INSTDIR\plugins\videoFilters\cli
        ${File} ${ROOT_FOLDER}/plugins/videoFilters/cli/*.dll
        # Fonts
        SetOutPath $INSTDIR\etc\fonts
        ${Folder} ${ROOT_FOLDER}/etc/fonts
SectionEnd
Section "PluginsSettings" SecGrpPluginsSettings
        SectionIn 1 2 RO
        SetOverwrite on
        SetOutPath $INSTDIR\plugins\pluginSettings\x264\3
        ${File} ${ROOT_FOLDER}/plugins/pluginSettings/x264/3/*.json
        #SetOutPath $INSTDIR\plugins\pluginSettings\x264\3
        #${File} ${ROOT_FOLDER}/plugins/pluginSettings/x264/3/*.json
SectionEnd
# !!
Section "Avisynth_VapourSynth" SecGrpAvisynth
        SectionIn 2
        SetOutPath $INSTDIR
        SetOverwrite on
        ${File} ${ROOT_FOLDER}/avsproxy.exe
        ${File} ${ROOT_FOLDER}/avsproxy_gui.exe
        ${File} ${ROOT_FOLDER}/vsProxy.exe
        ${File} ${ROOT_FOLDER}/vsProxy_gui_qt5.exe
SectionEnd
#
Section "ShaderDemo" SecShaderDemo
        SectionIn  1 2 RO
        SetOutPath $INSTDIR\shaderDemo
        SetOverwrite on
        ${File} ${ROOT_FOLDER}/plugins/shaderDemo/1/*.shader # FIXME
SectionEnd
#
#
Section "Support libs" SecGrpSupport
        SectionIn  1 2 RO
        SetOutPath $INSTDIR
        SetOverwrite on
        ${File} ${ROOT_FOLDER}/*.dll
SectionEnd
#
Section "SDK (to write plugins)" SecGrpDev
        SectionIn 2
        SetOverwrite on
        SetOutPath $INSTDIR
        ${File} ${DEV_FOLDER}/*.dll.a
        SetOutPath $INSTDIR\include\avidemux\2.6
        ${File} ${DEV_FOLDER}/include/avidemux/2.6/*
        SetOutPath $INSTDIR\include\avidemux\2.6
        ${File} ${DEV_FOLDER}/include/avidemux/2.6/*
        !insertmacro InstallDev ADM_core
        !insertmacro InstallDev ADM_coreAudio
        !insertmacro InstallDev ADM_coreAudioDevice
        !insertmacro InstallDev ADM_coreAudioEncoder
        !insertmacro InstallDev ADM_coreAudioFilter
        !insertmacro InstallDev ADM_coreAudioParser
        !insertmacro InstallDev ADM_coreDemuxer
        !insertmacro InstallDev ADM_coreDemuxerMpeg
        !insertmacro InstallDev ADM_coreImage
        !insertmacro InstallDev ADM_coreImageLoader
        !insertmacro InstallDev ADM_coreJobs
        !insertmacro InstallDev ADM_coreMuxer
        !insertmacro InstallDev ADM_coreScript
        !insertmacro InstallDev ADM_coreSocket
        !insertmacro InstallDev ADM_coreSqlLight3
        !insertmacro InstallDev ADM_coreSubtitles
        !insertmacro InstallDev ADM_coreUI
        !insertmacro InstallDev ADM_coreUtils
        !insertmacro InstallDev ADM_coreVideoCodec
        !insertmacro InstallDev ADM_coreVideoEncoder
        !insertmacro InstallDev ADM_coreVideoFilter

        !insertmacro InstallDev libavcodec
        !insertmacro InstallDev libavformat
        !insertmacro InstallDev libavutil
        !insertmacro InstallDev libswscale
        !insertmacro InstallDev libpostproc

        !insertmacro InstallDev libpostproc
        !insertmacro InstallDev qt5
        !insertmacro InstallDevQt5 ADM_openGL
        !insertmacro InstallDevQt5 ADM_UIs

SectionEnd


