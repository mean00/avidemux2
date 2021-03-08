INCLUDE(vf_plugin)


SET(ArtMirrorCommon_SRCS 
ADM_vidArtMirror.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtMirrorQT_SRCS  qt4/Q_artMirror.cpp)
SET(ArtMirrorQT_HEADERS  qt4/Q_artMirror.h)
SET(ArtMirrorQT_UI    qt4/artMirror)
INIT_VIDEO_FILTER_QT4(ADM_vf_artMirror${QT_LIBRARY_EXTENSION} ${ArtMirrorQT_SRCS} ${ArtMirrorQT_HEADERS} ${ArtMirrorQT_UI} ${ArtMirrorCommon_SRCS} qt4/DIA_flyArtMirror.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtMirrorCli_SRCS cli/DIA_artMirror.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artMirrorCli ${ArtMirrorCli_SRCS} ${ArtMirrorCommon_SRCS})
#
