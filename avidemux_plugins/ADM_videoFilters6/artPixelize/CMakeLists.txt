INCLUDE(vf_plugin)


SET(ArtPixelizeCommon_SRCS 
ADM_vidArtPixelize.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtPixelizeQT_SRCS  qt4/Q_artPixelize.cpp)
SET(ArtPixelizeQT_HEADERS  qt4/Q_artPixelize.h)
SET(ArtPixelizeQT_UI    qt4/artPixelize)
INIT_VIDEO_FILTER_QT4(ADM_vf_artPixelize${QT_LIBRARY_EXTENSION} ${ArtPixelizeQT_SRCS} ${ArtPixelizeQT_HEADERS} ${ArtPixelizeQT_UI} ${ArtPixelizeCommon_SRCS} qt4/DIA_flyArtPixelize.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtPixelizeCli_SRCS cli/DIA_artPixelize.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artPixelizeCli ${ArtPixelizeCli_SRCS} ${ArtPixelizeCommon_SRCS})
#
