INCLUDE(vf_plugin)


SET(ArtVHSCommon_SRCS 
ADM_vidArtVHS.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtVHSQT_SRCS  qt4/Q_artVHS.cpp)
SET(ArtVHSQT_HEADERS  qt4/Q_artVHS.h)
SET(ArtVHSQT_UI    qt4/artVHS)
INIT_VIDEO_FILTER_QT4(ADM_vf_artVHS${QT_LIBRARY_EXTENSION} ${ArtVHSQT_SRCS} ${ArtVHSQT_HEADERS} ${ArtVHSQT_UI} ${ArtVHSCommon_SRCS} qt4/DIA_flyArtVHS.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtVHSCli_SRCS cli/DIA_artVHS.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artVHSCli ${ArtVHSCli_SRCS} ${ArtVHSCommon_SRCS})
#
