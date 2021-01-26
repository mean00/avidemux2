INCLUDE(vf_plugin)


SET(ArtColorEffectCommon_SRCS 
ADM_vidArtColorEffect.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtColorEffectQT_SRCS  qt4/Q_artColorEffect.cpp)
SET(ArtColorEffectQT_HEADERS  qt4/Q_artColorEffect.h)
SET(ArtColorEffectQT_UI    qt4/artColorEffect)
INIT_VIDEO_FILTER_QT4(ADM_vf_artColorEffect${QT_LIBRARY_EXTENSION} ${ArtColorEffectQT_SRCS} ${ArtColorEffectQT_HEADERS} ${ArtColorEffectQT_UI} ${ArtColorEffectCommon_SRCS} qt4/DIA_flyArtColorEffect.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtColorEffectCli_SRCS cli/DIA_artColorEffect.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artColorEffectCli ${ArtColorEffectCli_SRCS} ${ArtColorEffectCommon_SRCS})
#
