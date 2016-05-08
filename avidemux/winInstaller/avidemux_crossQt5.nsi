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
#!include revision.nsh

SetCompressor /SOLID lzma
SetCompressorDictSize 96
RequestExecutionLevel user

##########################
# Defines
##########################
#!define NSIDIR "/home/fx/hudson/workspace/mingw_2.6.2_nsis/avidemux/winInstaller"
#!define SVN_VERSION 6
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
BrandingText "Packaged by Gruntster"
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

##########################
# Installer sections
##########################
Section -OpenLogFile
	CreateDirectory "$INSTDIR"
	FileOpen $UninstallLogHandle ${UninstallLogPath} a
	FileSeek $UninstallLogHandle 0 END
SectionEnd
SectionGroup /e "User interfaces" SecGrpUI
    ${MementoUnselectedSection} "Command Line" SecUiCli
        SectionIn 2
        SetOutPath $INSTDIR
        SetOverwrite on
        ${File} ${ROOT_FOLDER}/avidemux_cli.exe
        ${File} ${ROOT_FOLDER}/libADM_render6_cli.dll
        ${File} ${ROOT_FOLDER}/libADM_UI_Cli6.dll
    ${MementoSectionEnd}

    ${MementoSection} Qt SecUiQt
	SectionIn 1 2 RO
        SetOutPath $INSTDIR
        SetOverwrite on
        ${File} ${ROOT_FOLDER}/avidemux.exe
        ${File} ${ROOT_FOLDER}/avidemux_jobs.exe
        ${File} ${ROOT_FOLDER}/libADM_render6_QT5.dll
        ${File} ${ROOT_FOLDER}/libADM_UIQT56.dll
        ${File} ${ROOT_FOLDER}/libADM_openGLQT56.dll

        SetOutPath $INSTDIR\qt5\i18n
        ${File} ${ROOT_FOLDER}/qt5/i18n/*.qm
    ${MementoSectionEnd}
SectionGroupEnd

SectionGroup "Audio Decoders" SecGrpAudioDecoder
	${MementoSection} "µ-law" SecAudDecUlaw
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_ulaw.dll
	${MementoSectionEnd}
	${MementoSection} "AAC (FAAD)" SecAudDecFaad
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_faad.dll
		#SetOutPath $INSTDIR
		#${File} ${ROOT_FOLDER}/libfaad2.dll
	${MementoSectionEnd}
	${MementoSection} "Opus (libOpus)" SecAudDecOpus
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_opus.dll
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/libopus-0.dll
	${MementoSectionEnd}
	${MementoSection} "AAC, AC-3, ADPCM IMA AMV, DTS, E-AC-3, MP2, MP3, Nellymoser, QDesign, WMA (libavcodec)" SecAudDecAvcodec
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_lav.dll
	${MementoSectionEnd}
	${MementoSection} "AC-3 (liba52)" SecAudDecA52
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_a52.dll
	${MementoSectionEnd}
	${MementoSection} "ADPCM IMA" SecAudDecImaAdpcm
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_ima_adpcm.dll
	${MementoSectionEnd}
	${MementoSection} "ADPCM Microsoft" SecAudDecMsAdpcm
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_ms_adpcm.dll
	${MementoSectionEnd}
	${MementoSection} "AMR-NB" SecAudDecOpencoreAmrNb
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_opencore_amrnb.dll
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/libopencore-amrnb-*.dll
	${MementoSectionEnd}
	${MementoSection} "AMR-WB" SecAudDecOpencoreAmrWb
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_opencore_amrwb.dll
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/libopencore-amrwb-*.dll
	${MementoSectionEnd}
	${MementoSection} "MP2, MP3 (MAD)" SecAudDecMad
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_Mad.dll
	${MementoSectionEnd}
	${MementoSection} "Vorbis" SecAudDecVorbis
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDecoder
		${File} ${ROOT_FOLDER}/plugins/audioDecoder/libADM_ad_vorbis.dll
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/libogg-0.dll
		${File} ${ROOT_FOLDER}/libvorbis-0.dll
	${MementoSectionEnd}
SectionGroupEnd
SectionGroup "Audio Devices" SecGrpAudioDevice
	${MementoSection} "Waveform" SecAudDevWaveform
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioDevices
		${File} ${ROOT_FOLDER}/plugins/audioDevices/libADM_av_win32.dll
	${MementoSectionEnd}
SectionGroupEnd
SectionGroup "Audio Encoders" SecGrpAudioEncoder
	${MementoSection} "AAC (FAAC)" SecAudEncFaac
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioEncoders
		${File} ${ROOT_FOLDER}/plugins/audioEncoders/libADM_ae_faac.dll
		#SetOutPath $INSTDIR
		#${File} ${ROOT_FOLDER}libfaac.dll
	${MementoSectionEnd}
	${MementoSection} "AAC (libavcodec)" SecAudEncLavAac
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioEncoders
		${File} ${ROOT_FOLDER}/plugins/audioEncoders/libADM_ae_lav_aac.dll
	${MementoSectionEnd}
	${MementoSection} "AC-3 (Aften)" SecAudDecAften
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioEncoders
		${File} ${ROOT_FOLDER}/plugins/audioEncoders/libADM_ae_aften.dll
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/libaften.dll
	${MementoSectionEnd}
	${MementoSection} "AC-3 (libavcodec)" SecAudEncLavAc3
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioEncoders
		${File} ${ROOT_FOLDER}/plugins/audioEncoders/libADM_ae_lav_ac3.dll
	${MementoSectionEnd}
	#${MementoSection} "DTS (dcaenc)" SecAudDecDcaEnc
		#SectionIn 1 2
		#SetOverwrite on
		#SetOutPath $INSTDIR\plugins\audioEncoders
		#${File} ${ROOT_FOLDER}/plugins/audioEncoders/libADM_ae_dcaenc.dll
		#SetOutPath $INSTDIR
		#${File} libdcaenc-0.dll
	#${MementoSectionEnd}
	${MementoSection} "MP2 (libavcodec)" SecAudEncLavMp2
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioEncoders
		${File} ${ROOT_FOLDER}/plugins/audioEncoders/libADM_ae_lav_mp2.dll
		SetOutPath $INSTDIR
		#${File} ${ROOT_FOLDER}/libtwolame-*.dll
	${MementoSectionEnd}
	${MementoSection} "MP2 (TwoLAME)" SecAudEncTwoLame
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioEncoders
		${File} ${ROOT_FOLDER}/plugins/audioEncoders/libADM_ae_twolame.dll
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/libtwolame.dll
	${MementoSectionEnd}
	${MementoSection} "MP3" SecAudEncLame
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioEncoders
		${File} ${ROOT_FOLDER}/plugins/audioEncoders/libADM_ae_lame.dll
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/libmp3lame-0.dll
	${MementoSectionEnd}
	${MementoSection} "PCM" SecAudEncPcm
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioEncoders
		${File} ${ROOT_FOLDER}/plugins/audioEncoders/libADM_ae_pcm.dll
	${MementoSectionEnd}
	${MementoSection} "Vorbis" SecAudEncVorbis
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\audioEncoders
		${File} ${ROOT_FOLDER}/plugins/audioEncoders/libADM_ae_vorbis.dll
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/libogg-0.dll
		${File} ${ROOT_FOLDER}/libvorbis-0.dll
		${File} ${ROOT_FOLDER}/libvorbisenc-2.dll
		${File} ${ROOT_FOLDER}/libvorbisfile-3.dll
	${MementoSectionEnd}
SectionGroupEnd
SectionGroup "Avisynth_VapourSynth" SecGrpAvisynth
	${MementoUnselectedSection} "Avisynth Proxy" SecAvsProxy
		SectionIn 2
		SetOutPath $INSTDIR
		SetOverwrite on
		${File} ${ROOT_FOLDER}/avsproxy.exe
		${File} ${ROOT_FOLDER}/avsproxy_gui.exe
	${MementoSectionEnd}
	${MementoUnselectedSection} "Avisynth Proxy Demuxer" SecDemuxAvisynth
		SectionIn 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\demuxers
		${File} ${ROOT_FOLDER}/plugins/demuxers/libADM_dm_avsproxy.dll
	${MementoSectionEnd}
        ${MementoUnselectedSection} "VapourSynth Proxy (cli)" SecDemuxVS_cli
		SectionIn 2
		SetOverwrite on
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/vsProxy.exe
	${MementoSectionEnd}
        ${MementoUnselectedSection} "VapourSynth Proxy (Qt5)" SecDemuxVS_Qt5
		SectionIn 2
		SetOverwrite on
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/vsProxy_gui_qt5.exe
	${MementoSectionEnd}
SectionGroupEnd
SectionGroup "Demuxers" SecGrpDemuxers
	${MementoSection} "ASF" SecDemuxAsf
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\demuxers
		${File} ${ROOT_FOLDER}/plugins/demuxers/libADM_dm_asf.dll
	${MementoSectionEnd}
	${MementoSection} "BMP, JPEG, PNG Images" SecDemuxImage
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\demuxers
		${File} ${ROOT_FOLDER}/plugins/demuxers/libADM_dm_pic.dll
	${MementoSectionEnd}
	${MementoSection} "Flash Video" SecDemuxFlv
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\demuxers
		${File} ${ROOT_FOLDER}/plugins/demuxers/libADM_dm_flv.dll
	${MementoSectionEnd}
	${MementoSection} "Matroska" SecDemuxMatroska
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\demuxers
		${File} ${ROOT_FOLDER}/plugins/demuxers/libADM_dm_matroska.dll
	${MementoSectionEnd}
	${MementoSection} "MP4" SecDemuxMp4
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\demuxers
		${File} ${ROOT_FOLDER}/plugins/demuxers/libADM_dm_mp4.dll
	${MementoSectionEnd}
	${MementoSection} "MPEG-PS" SecDemuxMpegPs
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\demuxers
		${File} ${ROOT_FOLDER}/plugins/demuxers/libADM_dm_ps.dll
	${MementoSectionEnd}
	${MementoSection} "MPEG-TS" SecDemuxMpegTs
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\demuxers
		${File} ${ROOT_FOLDER}/plugins/demuxers/libADM_dm_ts.dll
	${MementoSectionEnd}
	${MementoSection} "MXF" SecDemuxMxf
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\demuxers
		${File} ${ROOT_FOLDER}/plugins/demuxers/libADM_dm_mxf.dll
	${MementoSectionEnd}
	${MementoSection} "OpenDML AVI" SecDemuxOpenDml
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\demuxers
		${File} ${ROOT_FOLDER}/plugins/demuxers/libADM_dm_opendml.dll
	${MementoSectionEnd}
SectionGroupEnd
SectionGroup "Muxers" SecGrpMuxers
	${MementoSection} "Dummy [Raw Audio/Video]" SecMuxDummy
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\muxers
		${File} ${ROOT_FOLDER}/plugins/muxers/libADM_mx_dummy.dll
	${MementoSectionEnd}
	${MementoSection} "Flash Video" SecMuxLavFlv
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\muxers
		${File} ${ROOT_FOLDER}/plugins/muxers/libADM_mx_flv.dll
	${MementoSectionEnd}
	${MementoSection} "Matroska" SecMuxLavMatroska
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\muxers
		${File} ${ROOT_FOLDER}/plugins/muxers/libADM_mx_Mkv.dll
	${MementoSectionEnd}
	${MementoSection} "MP4 (libavcodec)" SecMuxLavMp4
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\muxers
		${File} ${ROOT_FOLDER}/plugins/muxers/libADM_mx_mp4.dll
	${MementoSectionEnd}
	${MementoSection} "MP4 (MP4v2)" SecMuxMp4v2
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\muxers
		${File} ${ROOT_FOLDER}/plugins/muxers/libADM_mx_mp4v2.dll
	${MementoSectionEnd}
	${MementoSection} "MPEG-PS" SecMuxLavMpegPs
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\muxers
		${File} ${ROOT_FOLDER}/plugins/muxers/libADM_mx_ffPS.dll
	${MementoSectionEnd}
	${MementoSection} "MPEG-TS" SecMuxLavMpegTs
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\muxers
		${File} ${ROOT_FOLDER}/plugins/muxers/libADM_mx_ffTS.dll
	${MementoSectionEnd}
	${MementoSection} "OpenDML AVI" SecMuxOpenDml
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\muxers
		${File} ${ROOT_FOLDER}/plugins/muxers/libADM_mx_avi.dll
	${MementoSectionEnd}
	${MementoSection} "Raw Video" SecMuxRaw
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\muxers
		${File} ${ROOT_FOLDER}/plugins/muxers/libADM_mx_raw.dll
	${MementoSectionEnd}
	${MementoSection} "Raw Video" SecMuxWebm
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\muxers
		${File} ${ROOT_FOLDER}/plugins/muxers/libADM_mx_Webm.dll
	${MementoSectionEnd}
SectionGroupEnd
SectionGroup "Script Engines" SecGrpScriptEngine
	#${MementoSection} "QtScript" SecScriptQt
		#SectionIn 1 2
		#SetOverwrite on
		#SetOutPath $INSTDIR
		#${File} ${ROOT_FOLDER}/QtScript4.dll
		#${File} ${ROOT_FOLDER}/QtScriptTools4.dll
		#SetOutPath $INSTDIR\plugins\scriptEngines
		#${File} ${ROOT_FOLDER}/plugins/scriptEngines/libADM_script_qt.dll
		#SetOutPath $INSTDIR\help\QtScript
		#${Folder} help/QtScript
	#${MementoSectionEnd}
	${MementoSection} "Tinypy" SecScriptTinypy
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\scriptEngines
		${File} ${ROOT_FOLDER}/plugins/scriptEngines/libADM_script_tinyPy.dll
		SetOutPath $INSTDIR\plugins\autoScripts
		${Folder} ${ROOT_FOLDER}/plugins/autoScripts
	${MementoSectionEnd}
SectionGroupEnd
#SectionGroup "Video Decoders" SecGrpVideoDecoder
	#${MementoSection} "VP8" SecVidDecVpx
		#SectionIn 1 2
		#SetOverwrite on
		#SetOutPath $INSTDIR\plugins\videoDecoders
		#${File} ${ROOT_FOLDER}/plugins/videoDecoders/libADM_vd_vpx.dll
	#${MementoSectionEnd}
#SectionGroupEnd
SectionGroup "Video Encoders" SecGrpVideoEncoder
	${MementoSection} "[Null]" SecVidEncNull
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_null.dll
	${MementoSectionEnd}
	${MementoSection} "Huffyuv, FFVHuff" SecVidEncLavHuffyuv
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_huff.dll
	${MementoSectionEnd}
	${MementoSection} "JPEG" SecVidEncLavJpeg
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_jpeg.dll
	${MementoSectionEnd}
	${MementoSection} "MPEG-2" SecVidEncLavMpeg2
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_ffMpeg2.dll
	${MementoSectionEnd}
	${MementoSection} "MPEG-4 ASP (libavcodec)" SecVidEncLavMpeg4asp
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_ffMpeg4.dll
	${MementoSectionEnd}
	${MementoSection} "MPEG-4 ASP (Xvid)" SecVidEncXvid
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_xvid4.dll
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/xvidcore.dll
	${MementoSectionEnd}
	${MementoSection} "MPEG-4 AVC" SecVidEncX264
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders

		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_x264_other.dll

		SetOutPath $INSTDIR\plugins\videoEncoders\qt5
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/qt5/libADM_ve_x264_QT5.dll

		SetOutPath $INSTDIR\plugins\pluginSettings\x264
		${Folder} ${ROOT_FOLDER}/plugins/pluginSettings/x264
		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/libx264-*.dll
		#${File} ${ROOT_FOLDER}/pthreadGC2-w64.dll
	${MementoSectionEnd}
	${MementoSection} "MPEG-4 HEC" SecVidEncX265
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders

		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_x265_other.dll

		SetOutPath $INSTDIR\plugins\videoEncoders\qt5
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/qt5/libADM_ve_x265_QT5.dll

		SetOutPath $INSTDIR
		${File} ${ROOT_FOLDER}/libx265*.dll
		#${File} ${ROOT_FOLDER}/pthreadGC2-w64.dll
	${MementoSectionEnd}
${MementoSection} "PNG" SecVidEncLavPng
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_png.dll
	${MementoSectionEnd}
	${MementoSection} "Sorenson Spark" SecVidEncSorenson
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_ffFlv1.dll
	${MementoSectionEnd}
	${MementoSection} "YV12" SecVidEncYv12
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_yv12.dll
	${MementoSectionEnd}
	${MementoSection} "DV" SecVidEncDV
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_ffDv.dll
	${MementoSectionEnd}
	${MementoSection} "NVenc" SecVidEncffNvenc
		SectionIn 1 2
		SetOverwrite on
		SetOutPath $INSTDIR\plugins\videoEncoders
		${File} ${ROOT_FOLDER}/plugins/videoEncoders/libADM_ve_ffNvenc.dll
	${MementoSectionEnd}
SectionGroupEnd
SectionGroup "Video Filters" SecGrpVideoFilter
	SectionGroup "Transform Filters" SecGrpVideoFilterTransform
		${MementoSection} "Add Black Borders" SecVidFltBlackBorders
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_addBorders.dll
		${MementoSectionEnd}
		${MementoSection} "Add Logo" SecVidFltLogo
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_logo.dll
		${MementoSectionEnd}
                ${MementoSection} "Black" SecVidFltBlack
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_black.dll
		${MementoSectionEnd}

		${MementoSection} "Blacken Borders" SecVidFltBlackenBorders
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_blackenBorders.dll
		${MementoSectionEnd}
		${MementoSection} "Change FPS" SecVidFltChangeFps
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_changeFps.dll
		${MementoSectionEnd}
		${MementoSection} "Crop" SecVidFltCrop
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters\cli
			${File} ${ROOT_FOLDER}/plugins/videoFilters/cli/libADM_vf_CropCli.dll
			SetOutPath $INSTDIR\plugins\videoFilters\qt5
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_cropQT5.dll
		${MementoSectionEnd}
		${MementoSection} "Fade" SecVidFltFade
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_fadeToBlack.dll
		${MementoSectionEnd}
		${MementoSection} "Greyscale" SecVidFltLumaOnly
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_lumaOnly.dll
		${MementoSectionEnd}
		${MementoSection} "Horizontal Flip" SecVidFltHorizontalFlip
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_hf_hflip.dll
		${MementoSectionEnd}
		${MementoSection} "libswscale Resize" SecVidFltSwscaleResize
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters\cli
			${File} ${ROOT_FOLDER}/plugins/videoFilters/cli/libADM_vf_swscaleResize_cli.dll
			SetOutPath $INSTDIR\plugins\videoFilters\qt5
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_swscaleResizeQT5.dll
		${MementoSectionEnd}
		${MementoSection} "OpenGl test filters " SecVidFltOpenGl
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters\qt5
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_glBenchmark.dll
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_glResize.dll
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_sampleGlFrag2.dll
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_sampleGlVertex.dll
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_rotateGlFrag2.dll
		${MementoSectionEnd}
${MementoSection} "Resample FPS" SecVidFltResampleFps
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_resampleFps.dll
		${MementoSectionEnd}
		${MementoSection} "Rotate" SecVidFltRotate
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_rotate.dll
		${MementoSectionEnd}
		${MementoSection} "Vertical Flip" SecVidFltVerticalFlip
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_vflip.dll
		${MementoSectionEnd}
	SectionGroupEnd
	SectionGroup "Interlacing Filters" SecGrpVideoFilterInterlacing
		${MementoSection} "Ivtc Dupe Removal" SecVidFltIvtcDupeRemoval
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_ivtcDupeRemover.dll
		${MementoSectionEnd}
		${MementoSection} "Decomb Telecide" SecVidFltDecombTelecide
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_telecide.dll
		${MementoSectionEnd}
		${MementoSection} "DGBob" SecVidFltDgbob
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_DgBob.dll
		${MementoSectionEnd}
		${MementoSection} "Horizontal Stack Fields" SecVidFltHzStackFields
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_hzstackField.dll
		${MementoSectionEnd}
		${MementoSection} "Kernel Deint" SecVidFltKernelDeint
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_kernelDeint.dll
		${MementoSectionEnd}
		${MementoSection} "libavcodec Deinterlacers" SecVidFltLavDeinterlacers
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_lavDeint.dll
		${MementoSectionEnd}
		${MementoSection} "Merge Fields" SecVidFltMergeFields
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_mergeField.dll
		${MementoSectionEnd}
		${MementoSection} "Separate Fields" SecVidFltSeparateFields
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_separateField.dll
		${MementoSectionEnd}
		${MementoSection} "Stack Fields" SecVidFltStackFields
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_stackField.dll
		${MementoSectionEnd}
		${MementoSection} "Unstack Fields" SecVidFltUnstackFields
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_unstackField.dll
		${MementoSectionEnd}
		${MementoSection} "Yadif" SecVidFltYadif
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_yadif.dll
		${MementoSectionEnd}
	SectionGroupEnd
	SectionGroup "Colour Filters" SecGrpVideoFilterColour
		${MementoSection} "Avisynth Colour YUV" SecVidFltAvisynthColourYuv
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_colorYuv.dll
		${MementoSectionEnd}
		${MementoSection} "ChromaShift" SecVidFltChromaShift
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters\cli
			${File} ${ROOT_FOLDER}/plugins/videoFilters/cli/libADM_vf_chromaShiftCli.dll
			SetOutPath $INSTDIR\plugins\videoFilters\qt5
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_chromaShiftQT5.dll
		${MementoSectionEnd}
		${MementoSection} "Contrast" SecVidFltContrast
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters\cli
			${File} ${ROOT_FOLDER}/plugins/videoFilters/cli/libADM_vf_contrastCli.dll
			SetOutPath $INSTDIR\plugins\videoFilters\qt5
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_contrastQT5.dll
		${MementoSectionEnd}
		${MementoSection} "MPlater Eq2" SecVidFltMplayerEq2
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters\cli
			${File} ${ROOT_FOLDER}/plugins/videoFilters/cli/libADM_vf_eq2Cli.dll
			SetOutPath $INSTDIR\plugins\videoFilters\qt5
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_eq2QT5.dll
		${MementoSectionEnd}
		${MementoSection} "MPlater Hue" SecVidFltMplayerHue
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters\cli
			${File} ${ROOT_FOLDER}/plugins/videoFilters/cli/libADM_vf_HueCli.dll
			SetOutPath $INSTDIR\plugins\videoFilters\qt5
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_HueQT5.dll
		${MementoSectionEnd}
		${MementoSection} "Remove Plane" SecVidFltRemovePlane
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_removePlane.dll
		${MementoSectionEnd}
		${MementoSection} "Swap U and V" SecVidFltSwapUandV
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_swapUV.dll
		${MementoSectionEnd}
	SectionGroupEnd
	SectionGroup "Noise Filters" SecGrpVideoFilterNoise
		${MementoSection} "FluxSmooth" SecVidFltFluxSmooth
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_FluxSmooth.dll
		${MementoSectionEnd}
		${MementoSection} "Gaussian Convolution" SecVidFltGauss
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_gauss.dll
		${MementoSectionEnd}
		${MementoSection} "Large Median (5x5)" SecVidFltMediam5x5
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_largeMedian.dll
		${MementoSectionEnd}
		${MementoSection} "Mean Convolution" SecVidFltMean
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_mean.dll
		${MementoSectionEnd}
		${MementoSection} "Median Convolution" SecVidFltMedian
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_median.dll
		${MementoSectionEnd}
		${MementoSection} "MPlayer Denoise 3D" SecVidFltMPlayerDenoise3d
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_denoise3d.dll
		${MementoSectionEnd}
		${MementoSection} "MPlayer Denoise 3D HQ" SecVidFltMPlayerDenoise3dHq
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_denoise3dhq.dll
		${MementoSectionEnd}
		${MementoSection} "MSharpen" SecVidFltMSharpen
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_msharpen.dll
		${MementoSectionEnd}
	SectionGroupEnd
	SectionGroup "Sharpness Filters" SecGrpVideoFilterSharpness
		${MementoSection} "asharp" SecVidFltAsharp
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			SetOutPath $INSTDIR\plugins\videoFilters\qt5
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_asharpQT5.dll
		${MementoSectionEnd}
		${MementoSection} "MPlayer Delogo" SecVidFltMPlayerDelogo
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters\cli
			${File} ${ROOT_FOLDER}/plugins/videoFilters/cli/libADM_vf_mpdelogoCli.dll
			SetOutPath $INSTDIR\plugins\videoFilters\qt5
			${File} ${ROOT_FOLDER}/plugins/videoFilters/qt5/libADM_vf_mpdelogoQT5.dll
		${MementoSectionEnd}
		${MementoSection} "Sharpen" SecVidFltSharpen
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_sharpen.dll
		${MementoSectionEnd}
	SectionGroupEnd
	SectionGroup "Subtitle Filters" SecGrpVideoFilterSubtitle
		${MementoSection} "ASS, SSA" SecVidFltAssSsa
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_ssa.dll
			SetOutPath $INSTDIR
			${File} ${ROOT_FOLDER}/libfribidi-0.dll
			${File} ${ROOT_FOLDER}/libfontconfig-1.dll
			SetOutPath $INSTDIR\etc\fonts
			${Folder} ${ROOT_FOLDER}/etc/fonts
		${MementoSectionEnd}
	SectionGroupEnd
	#SectionGroup "OpenGL Filters" SecGrpVideoFilterOpenGl
		#${MementoSection} "Fragment Shader" SecVidFltOpenGlFragmentShader
			#SectionIn 1 2
			#SetOverwrite on
			#SetOutPath $INSTDIR\plugins\videoFilters
			#${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_sampleGlFrag2.dll
		#${MementoSectionEnd}
		#${MementoSection} "Read Back Benchmark" SecVidFltOpenGlReadBack
			#SectionIn 1 2
			#SetOverwrite on
			#SetOutPath $INSTDIR\plugins\videoFilters
			#${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_glBenchmark.dll
		#${MementoSectionEnd}
		#${MementoSection} "Resize" SecVidFltOpenGlResize
			#SectionIn 1 2
			#SetOverwrite on
			#SetOutPath $INSTDIR\plugins\videoFilters
			#${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_glResize.dll
		#${MementoSectionEnd}
		#${MementoSection} "Rotate" SecVidFltOpenGlRotate
			#SectionIn 1 2
			#SetOverwrite on
			#SetOutPath $INSTDIR\plugins\videoFilters
			#${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_rotateGlFrag2.dll
		#${MementoSectionEnd}
		#${MementoSection} "Wave" SecVidFltOpenGlWave
			#SectionIn 1 2
			#SetOverwrite on
			#SetOutPath $INSTDIR\plugins\videoFilters
			#${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_sampleGlVertex.dll
		#${MementoSectionEnd}
	#SectionGroupEnd
	SectionGroup "Miscellaneous Filters" SecGrpVideoFilterMiscellaneous
		${MementoSection} "Print Information" SecVidFltPrintInfo
			SectionIn 1 2
			SetOverwrite on
			SetOutPath $INSTDIR\plugins\videoFilters
			${File} ${ROOT_FOLDER}/plugins/videoFilters/libADM_vf_printInfo.dll
		${MementoSectionEnd}
	SectionGroupEnd
SectionGroupEnd

