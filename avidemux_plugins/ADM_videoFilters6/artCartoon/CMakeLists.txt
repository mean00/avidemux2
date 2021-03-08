INCLUDE(vf_plugin)


SET(ArtCartoonCommon_SRCS 
ADM_vidArtCartoon.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtCartoonQT_SRCS  qt4/Q_artCartoon.cpp)
SET(ArtCartoonQT_HEADERS  qt4/Q_artCartoon.h)
SET(ArtCartoonQT_UI    qt4/artCartoon)
INIT_VIDEO_FILTER_QT4(ADM_vf_artCartoon${QT_LIBRARY_EXTENSION} ${ArtCartoonQT_SRCS} ${ArtCartoonQT_HEADERS} ${ArtCartoonQT_UI} ${ArtCartoonCommon_SRCS} qt4/DIA_flyArtCartoon.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtCartoonCli_SRCS cli/DIA_artCartoon.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artCartoonCli ${ArtCartoonCli_SRCS} ${ArtCartoonCommon_SRCS})
#
