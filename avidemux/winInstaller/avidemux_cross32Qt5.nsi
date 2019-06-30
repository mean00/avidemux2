##########################
# Included files
##########################
!define BUILD_BITS 32
#!define BINARY_FOLDER /mingw/avidemux_32
#!define DEV_FOLDER  /mingw_dev/mingw/Release/
!include avidemux_crossQt5.nsi

Section "Avidemux Core" SecCore
    SectionIn 1 2 RO
    SetOutPath $INSTDIR
    SetOverwrite on
    ${File} "./Build-Info.txt"
    ${File} "./ChangeLog.html"
    ${File} "./change.css"

    ${File} ${BINARY_FOLDER}/libcrypto-1_1.dll
    ${File} ${BINARY_FOLDER}/libssl-1_1.dll
    ${File} ${BINARY_FOLDER}/libpcre2-16-0.dll

    ${File} ${BINARY_FOLDER}/libexpat-1.dll
    ${File} ${BINARY_FOLDER}/libffi-6.dll
    ${File} ${BINARY_FOLDER}/libglib-2.0-0.dll
    ${File} ${BINARY_FOLDER}/libgobject-2.0-0.dll
    ${File} ${BINARY_FOLDER}/libharfbuzz-0.dll
    ${File} ${BINARY_FOLDER}/libiconv-2.dll # MXE libiconv.dll
    ${File} ${BINARY_FOLDER}/libpcre16-0.dll
    ${File} ${BINARY_FOLDER}/libpcre-1.dll
    #${File} ${BINARY_FOLDER}/libfaad-2.dll
    #${File} ${BINARY_FOLDER}/libfaac-0.dll
    ${File} ${BINARY_FOLDER}/libtwolame.dll
    ${File} ${BINARY_FOLDER}/libbz2.dll
    ${File} ${BINARY_FOLDER}/libpng16-16.dll
    ${File} ${BINARY_FOLDER}/libsqlite3-0.dll
    ${File} ${BINARY_FOLDER}/libstdc++-6.dll
    ${File} ${BINARY_FOLDER}/libwinpthread-1.dll
    ${File} ${BINARY_FOLDER}/libz-1.dll
    ${File} ${BINARY_FOLDER}/zlib1.dll
    ${File} ${BINARY_FOLDER}/libfreetype-6.dll
    ${File} ${BINARY_FOLDER}/libgcc_s_sjlj-1.dll
    ${File} ${BINARY_FOLDER}/SDL2.dll
# Qt
    ${File} ${BINARY_FOLDER}/Qt5Core.dll  
    ${File} ${BINARY_FOLDER}/Qt5Gui.dll  
    ${File} ${BINARY_FOLDER}/Qt5Widgets.dll
    ${File} ${BINARY_FOLDER}/Qt5Network.dll
    ${File} ${BINARY_FOLDER}/Qt5WinExtras.dll
    #${File} ${BINARY_FOLDER}/icudt56.dll 
    #${File} ${BINARY_FOLDER}/icuin56.dll  
    #${File} ${BINARY_FOLDER}/icuio56.dll  
    #${File} ${BINARY_FOLDER}/icule56.dll  
    #${File} ${BINARY_FOLDER}/iculx56.dll  
    #${File} ${BINARY_FOLDER}/icutu56.dll  
    #${File} ${BINARY_FOLDER}/icuuc56.dll
    ${File} ${BINARY_FOLDER}/libintl-8.dll
# SSL still needed ?
    ${File} ${BINARY_FOLDER}/libeay32.dll
    ${File} ${BINARY_FOLDER}/ssleay32.dll
        
    SetOutPath $INSTDIR\platforms
    ${File} ${BINARY_FOLDER}/platforms/qminimal.dll  
    ${File} ${BINARY_FOLDER}/platforms/qwindows.dll
    SetOutPath $INSTDIR\styles
    ${File} ${BINARY_FOLDER}/styles/qwindowsvistastyle.dll
    SetOutPath $INSTDIR
# adm
    ${File} ${BINARY_FOLDER}/libADM_audioParser6.dll
    ${File} ${BINARY_FOLDER}/libADM_core6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreAudio6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreAudioDevice6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreAudioEncoder6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreAudioFilterAPI6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreDemuxer6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreDemuxerMpeg6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreImage6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreImageLoader6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreJobs.dll
    ${File} ${BINARY_FOLDER}/libADM_coreMuxer6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreScript.dll
    ${File} ${BINARY_FOLDER}/libADM_coreSocket6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreSqlLight3.dll
    ${File} ${BINARY_FOLDER}/libADM_coreUI6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreUtils6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreVideoCodec6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreVideoEncoder6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreVideoFilter6.dll
    ${File} ${BINARY_FOLDER}/libADM_coreSubtitles6.dll
    ${File} ${SOURCE_FOLDER}/AUTHORS.
    ${File} ${SOURCE_FOLDER}/COPYING.
    ${File} ${SOURCE_FOLDER}/README.
    ${File} ${BINARY_FOLDER}/avcodec-*.dll
    ${File} ${BINARY_FOLDER}/avformat-*.dll
    ${File} ${BINARY_FOLDER}/avutil-*.dll
    ${File} ${BINARY_FOLDER}/postproc-*.dll
    ${File} ${BINARY_FOLDER}/swscale-*.dll

    ${File} ${BINARY_FOLDER}/libfribidi*.dll
    ${File} ${BINARY_FOLDER}/libfontconfig*.dll
    ${File} ${BINARY_FOLDER}/libfreetype*.dll
    ${File} ${BINARY_FOLDER}/libharfbuzz*.dll

	WriteRegStr HKLM "${REGKEY}" CreateDesktopIcon $CreateDesktopIcon
	WriteRegStr HKLM "${REGKEY}" CreateStartMenuGroup $CreateStartMenuGroup

	${If} ${AtMostWinVista}
		WriteRegStr HKLM "${REGKEY}" CreateQuickLaunchIcon $CreateQuickLaunchIcon
	${EndIf}
SectionEnd
!include avidemux_crossQt5Tail.nsi
