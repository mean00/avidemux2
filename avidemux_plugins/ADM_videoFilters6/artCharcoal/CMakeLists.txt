INCLUDE(vf_plugin)


SET(ArtCharcoalCommon_SRCS 
ADM_vidArtCharcoal.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtCharcoalQT_SRCS  qt4/Q_artCharcoal.cpp)
SET(ArtCharcoalQT_HEADERS  qt4/Q_artCharcoal.h)
SET(ArtCharcoalQT_UI    qt4/artCharcoal)
INIT_VIDEO_FILTER_QT4(ADM_vf_artCharcoal${QT_LIBRARY_EXTENSION} ${ArtCharcoalQT_SRCS} ${ArtCharcoalQT_HEADERS} ${ArtCharcoalQT_UI} ${ArtCharcoalCommon_SRCS} qt4/DIA_flyArtCharcoal.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtCharcoalCli_SRCS cli/DIA_artCharcoal.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artCharcoalCli ${ArtCharcoalCli_SRCS} ${ArtCharcoalCommon_SRCS})
#
