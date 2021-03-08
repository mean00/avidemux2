INCLUDE(vf_plugin)


SET(ArtDynThresholdCommon_SRCS 
ADM_vidArtDynThreshold.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtDynThresholdQT_SRCS  qt4/Q_artDynThreshold.cpp)
SET(ArtDynThresholdQT_HEADERS  qt4/Q_artDynThreshold.h)
SET(ArtDynThresholdQT_UI    qt4/artDynThreshold)
INIT_VIDEO_FILTER_QT4(ADM_vf_artDynThreshold${QT_LIBRARY_EXTENSION} ${ArtDynThresholdQT_SRCS} ${ArtDynThresholdQT_HEADERS} ${ArtDynThresholdQT_UI} ${ArtDynThresholdCommon_SRCS} qt4/DIA_flyArtDynThreshold.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtDynThresholdCli_SRCS cli/DIA_artDynThreshold.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artDynThresholdCli ${ArtDynThresholdCli_SRCS} ${ArtDynThresholdCommon_SRCS})
#
