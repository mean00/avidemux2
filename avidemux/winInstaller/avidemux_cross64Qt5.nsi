##########################
# Included files
##########################
!define BUILD_BITS 64
!define ROOT_FOLDER /mingw/avidemux_64
!define DEV_FOLDER  /mingw_dev/mingw/Release/
!include avidemux_crossQt5.nsi

Section "Avidemux Core" SecCore
    SectionIn 1 2 RO
    SetOutPath $INSTDIR
    SetOverwrite on
    ${File} "./Build Info.txt"
    ${File} "./change.css"
    ${File} "./ChangeLog.html"
    ${File} ${ROOT_FOLDER}/libexpat-1.dll
    ${File} ${ROOT_FOLDER}/libffi-6.dll
    ${File} ${ROOT_FOLDER}/libglib-2.0-0.dll
    ${File} ${ROOT_FOLDER}/libgobject-2.0-0.dll
    ${File} ${ROOT_FOLDER}/libharfbuzz-0.dll
    ${File} ${ROOT_FOLDER}/libiconv.dll
    ${File} ${ROOT_FOLDER}/libpcre16-0.dll
    ${File} ${ROOT_FOLDER}/libpng14-14.dll
    ${File} ${ROOT_FOLDER}/libsqlite3.dll
    ${File} ${ROOT_FOLDER}/libstdc++-6.dll
    ${File} ${ROOT_FOLDER}/libwinpthread-1.dll
    ${File} ${ROOT_FOLDER}/libz-1.dll
    ${File} ${ROOT_FOLDER}/libfreetype-6.dll
    ${File} ${ROOT_FOLDER}/libgcc_s_seh-1.dll
    ${File} ${ROOT_FOLDER}/libintl-8.dll
    ${File} ${ROOT_FOLDER}/SDL2.dll
# Qt
    ${File} ${ROOT_FOLDER}/Qt5Core.dll  
    ${File} ${ROOT_FOLDER}/Qt5Gui.dll  
    ${File} ${ROOT_FOLDER}/Qt5OpenGL.dll
    ${File} ${ROOT_FOLDER}/Qt5Widgets.dll
    ${File} ${ROOT_FOLDER}/Qt5Network.dll
    ${File} ${ROOT_FOLDER}/Qt5WinExtras.dll
    ${File} ${ROOT_FOLDER}/libicudt51.dll 
    ${File} ${ROOT_FOLDER}/libicuin51.dll  
    ${File} ${ROOT_FOLDER}/libicuio51.dll  
    ${File} ${ROOT_FOLDER}/libicule51.dll  
    ${File} ${ROOT_FOLDER}/libiculx51.dll  
    ${File} ${ROOT_FOLDER}/libicutu51.dll  
    ${File} ${ROOT_FOLDER}/libicuuc51.dll
    SetOutPath $INSTDIR\platforms
    ${File} ${ROOT_FOLDER}/platforms/qminimal.dll  
    ${File} ${ROOT_FOLDER}/platforms/qwindows.dll
    SetOutPath $INSTDIR
# adm
    ${File} ${ROOT_FOLDER}/libADM_audioParser6.dll
    ${File} ${ROOT_FOLDER}/libADM_core6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreAudio6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreAudioDevice6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreAudioEncoder6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreAudioFilterAPI6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreDemuxer6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreDemuxerMpeg6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreImage6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreImageLoader6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreJobs.dll
    ${File} ${ROOT_FOLDER}/libADM_coreMuxer6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreScript.dll
    ${File} ${ROOT_FOLDER}/libADM_coreSocket6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreSqlLight3.dll
    ${File} ${ROOT_FOLDER}/libADM_coreUI6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreUtils6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreVideoCodec6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreVideoEncoder6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreVideoFilter6.dll
    ${File} ${ROOT_FOLDER}/libADM_coreSubtitle.dll
    ${File} ${NSIDIR}/../../AUTHORS.
    ${File} ${NSIDIR}/../../COPYING.
    ${File} ${NSIDIR}/../../README.
    ${File} ${ROOT_FOLDER}/avcodec-*.dll
    ${File} ${ROOT_FOLDER}/avformat-*.dll
    ${File} ${ROOT_FOLDER}/avutil-*.dll
    ${File} ${ROOT_FOLDER}/postproc-*.dll
    ${File} ${ROOT_FOLDER}/swscale-*.dll

	WriteRegStr HKLM "${REGKEY}" CreateDesktopIcon $CreateDesktopIcon
	WriteRegStr HKLM "${REGKEY}" CreateStartMenuGroup $CreateStartMenuGroup

	${If} ${AtMostWinVista}
		WriteRegStr HKLM "${REGKEY}" CreateQuickLaunchIcon $CreateQuickLaunchIcon
	${EndIf}
SectionEnd
!include avidemux_crossQt5Tail.nsi
