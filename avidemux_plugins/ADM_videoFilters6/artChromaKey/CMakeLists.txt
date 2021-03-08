INCLUDE(vf_plugin)


IF(DO_QT4)
    IF(ADM_QT_VERSION EQUAL 4)
        SET(QT_TAG qt4)
    ELSE(ADM_QT_VERSION EQUAL 4)
        SET(QT_TAG qt5)
        ADM_QT_ADD_RESOURCES(resource   qt4/artChromaKey.qrc)
    ENDIF(ADM_QT_VERSION EQUAL 4)
ENDIF(DO_QT4)


SET(ArtChromaKeyCommon_SRCS 
ADM_vidArtChromaKey.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtChromaKeyQT_SRCS  qt4/Q_artChromaKey.cpp)
SET(ArtChromaKeyQT_HEADERS  qt4/Q_artChromaKey.h)
SET(ArtChromaKeyQT_UI    qt4/artChromaKey)
IF(DO_QT4)
    INIT_VIDEO_FILTER_QT4(ADM_vf_artChromaKey${QT_LIBRARY_EXTENSION} ${ArtChromaKeyQT_SRCS} ${ArtChromaKeyQT_HEADERS} ${ArtChromaKeyQT_UI} ${ArtChromaKeyCommon_SRCS} qt4/DIA_flyArtChromaKey.cpp ${resource})
    TARGET_LINK_LIBRARIES(ADM_vf_artChromaKey${QT_LIBRARY_EXTENSION} ADM_coreImageLoader6)
ENDIF(DO_QT4)
# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtChromaKeyCli_SRCS cli/DIA_artChromaKey.cpp)
IF(DO_CLI)
    INIT_VIDEO_FILTER_CLI(  ADM_vf_artChromaKeyCli ${ArtChromaKeyCli_SRCS} ${ArtChromaKeyCommon_SRCS})
    TARGET_LINK_LIBRARIES(ADM_vf_artChromaKeyCli ADM_coreImageLoader6)
ENDIF(DO_CLI)
#
