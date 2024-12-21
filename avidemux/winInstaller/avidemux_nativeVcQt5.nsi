##########################
# Included files
##########################
!define BUILD_BITS 64
!include "avidemux_nativeQt5Common.nsi"

Section "Avidemux Core" SecCore
    SectionIn 1 2 RO
    SetOutPath $INSTDIR
    SetOverwrite on
    ${File} .\Build-Info.txt
    ${File} .\change.css
    ${File} .\ChangeLog.html

    
    ${File} ${BINARY_FOLDER}\avcodec-58.dll
    #${File} ${BINARY_FOLDER}\avdevice-58.dll
    ${File} ${BINARY_FOLDER}\avformat-58.dll
    ${File} ${BINARY_FOLDER}\avutil-56.dll    
    ${File} ${BINARY_FOLDER}\libwinpthread-1.dll
    ${File} ${BINARY_FOLDER}\libz-1.dll
    ${File} ${BINARY_FOLDER}\msvcp140.dll
	${File} ${BINARY_FOLDER}\swscale-5.dll    
    ${File} ${BINARY_FOLDER}\postproc-55.dll
	
    ${File} ${BINARY_FOLDER}\Qt5Core.dll
    ${File} ${BINARY_FOLDER}\Qt5Gui.dll
    ${File} ${BINARY_FOLDER}\Qt5Network.dll
    ${File} ${BINARY_FOLDER}\Qt5Widgets.dll
    ${File} ${BINARY_FOLDER}\Qt5WinExtras.dll
    ${File} ${BINARY_FOLDER}\sqlite3.dll
    ${File} ${BINARY_FOLDER}\vcruntime140.dll
	
	${File} ${BINARY_FOLDER}\freetype.dll
	${File} ${BINARY_FOLDER}\fribidi.dll
	${File} ${BINARY_FOLDER}\opus.dll

    ${File} ${BINARY_FOLDER}\twolame.dll
    
	${File} ${BINARY_FOLDER}\fdk-aac-1.dll
	${File} ${BINARY_FOLDER}\aften.dll
    ${File} ${BINARY_FOLDER}\zlibd.dll
    ${File} ${BINARY_FOLDER}\zlib.dll
	${File} ${BINARY_FOLDER}\libogg.dll
	${File} ${BINARY_FOLDER}\libvorbis.dll
	${File} ${BINARY_FOLDER}\libvorbisenc-2.dll
	${File} ${BINARY_FOLDER}\libvorbisfile.dll
    ${File} ${BINARY_FOLDER}\libx264-146.dll
    ${File} ${BINARY_FOLDER}\libx265.dll
    ${File} ${BINARY_FOLDER}\xvidcore.dll

# QT
    ${File} ${BINARY_FOLDER}\Qt5Core.dll
    ${File} ${BINARY_FOLDER}\Qt5Gui.dll
    ${File} ${BINARY_FOLDER}\Qt5Network.dll
#    ${File} ${BINARY_FOLDER}\Qt5OpenGL.dll
    ${File} ${BINARY_FOLDER}\Qt5Widgets.dll
    ${File} ${BINARY_FOLDER}\Qt5WinExtras.dll
    SetOutPath $INSTDIR\platforms
    ${File} ${BINARY_FOLDER}\platforms\qminimal.dll  
    ${File} ${BINARY_FOLDER}\platforms\qwindows.dll
    SetOutPath $INSTDIR\styles
    ${File} ${BINARY_FOLDER}\styles\qwindowsvistastyle.dll

    SetOutPath $INSTDIR
# adm
    ${File} ${BINARY_FOLDER}\ADM_audioParser6.dll
    ${File} ${BINARY_FOLDER}\ADM_core6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreAudio6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreAudioDevice6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreAudioEncoder6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreAudioFilterAPI6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreDemuxer6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreDemuxerMpeg6.dll
    #${File} ${BINARY_FOLDER}\ADM_coreDxva26.dll
    ${File} ${BINARY_FOLDER}\ADM_coreImage6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreImageLoader6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreJobs.dll
    ${File} ${BINARY_FOLDER}\ADM_coreMuxer6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreScript.dll
    ${File} ${BINARY_FOLDER}\ADM_coreSocket6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreSqlLight3.dll
    ${File} ${BINARY_FOLDER}\ADM_coreSubtitles6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreUI6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreUtils6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreVideoCodec6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreVideoEncoder6.dll
    ${File} ${BINARY_FOLDER}\ADM_coreVideoFilter6.dll
    ${File} ${BINARY_FOLDER}\*ADM_openGLQT56.dll
    ${File} ${BINARY_FOLDER}\*ADM_render6_cli.dll
    ${File} ${BINARY_FOLDER}\*ADM_render6_QT5.dll
    ${File} ${BINARY_FOLDER}\*ADM_UI_Cli6.dll
    ${File} ${BINARY_FOLDER}\*ADM_UIQT56.dll
    ${File} ${BINARY_FOLDER}\*ADM_coreDxva26.dll
    ${File} ${BINARY_FOLDER}\libADM_openGLQT56.dll
    ${File} ${BINARY_FOLDER}\libADM_render6_cli.dll
    ${File} ${BINARY_FOLDER}\libADM_render6_QT5.dll
    ${File} ${BINARY_FOLDER}\libADM_UI_Cli6.dll
    ${File} ${BINARY_FOLDER}\libADM_UIQT56.dll
    ${File} ${NSIDIR}\..\..\AUTHORS.
    ${File} ${NSIDIR}\..\..\COPYING.
    ${File} ${NSIDIR}\..\..\README.
    # Avs
    ${File} d:\avsProxy\avsproxy32.exe 
    ${File} d:\avsProxy\avsproxy64.exe 
   

	WriteRegStr HKLM "${REGKEY}" CreateDesktopIcon $CreateDesktopIcon
	WriteRegStr HKLM "${REGKEY}" CreateStartMenuGroup $CreateStartMenuGroup

	${If} ${AtMostWinVista}
		WriteRegStr HKLM "${REGKEY}" CreateQuickLaunchIcon $CreateQuickLaunchIcon
	${EndIf}
SectionEnd
!include avidemux_nativeQt5Tail.nsi

