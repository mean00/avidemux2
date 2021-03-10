INCLUDE(vf_plugin)


SET(ArtGridCommon_SRCS 
ADM_vidArtGrid.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtGridQT_SRCS  qt4/Q_artGrid.cpp)
SET(ArtGridQT_HEADERS  qt4/Q_artGrid.h)
SET(ArtGridQT_UI    qt4/artGrid)
INIT_VIDEO_FILTER_QT4(ADM_vf_artGrid${QT_LIBRARY_EXTENSION} ${ArtGridQT_SRCS} ${ArtGridQT_HEADERS} ${ArtGridQT_UI} ${ArtGridCommon_SRCS} qt4/DIA_flyArtGrid.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtGridCli_SRCS cli/DIA_artGrid.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artGridCli ${ArtGridCli_SRCS} ${ArtGridCommon_SRCS})
#
