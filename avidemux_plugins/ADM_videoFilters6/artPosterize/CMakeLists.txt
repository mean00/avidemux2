INCLUDE(vf_plugin)


SET(ArtPosterizeCommon_SRCS 
ADM_vidArtPosterize.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtPosterizeQT_SRCS  qt4/Q_artPosterize.cpp)
SET(ArtPosterizeQT_HEADERS  qt4/Q_artPosterize.h)
SET(ArtPosterizeQT_UI    qt4/artPosterize)
INIT_VIDEO_FILTER_QT4(ADM_vf_artPosterize${QT_LIBRARY_EXTENSION} ${ArtPosterizeQT_SRCS} ${ArtPosterizeQT_HEADERS} ${ArtPosterizeQT_UI} ${ArtPosterizeCommon_SRCS} qt4/DIA_flyArtPosterize.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtPosterizeCli_SRCS cli/DIA_artPosterize.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artPosterizeCli ${ArtPosterizeCli_SRCS} ${ArtPosterizeCommon_SRCS})
#
