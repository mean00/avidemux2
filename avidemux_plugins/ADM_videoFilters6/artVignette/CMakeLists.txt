INCLUDE(vf_plugin)


SET(ArtVignetteCommon_SRCS 
ADM_vidArtVignette.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtVignetteQT_SRCS  qt4/Q_artVignette.cpp)
SET(ArtVignetteQT_HEADERS  qt4/Q_artVignette.h)
SET(ArtVignetteQT_UI    qt4/artVignette)
INIT_VIDEO_FILTER_QT4(ADM_vf_artVignette${QT_LIBRARY_EXTENSION} ${ArtVignetteQT_SRCS} ${ArtVignetteQT_HEADERS} ${ArtVignetteQT_UI} ${ArtVignetteCommon_SRCS} qt4/DIA_flyArtVignette.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtVignetteCli_SRCS cli/DIA_artVignette.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artVignetteCli ${ArtVignetteCli_SRCS} ${ArtVignetteCommon_SRCS})
#
