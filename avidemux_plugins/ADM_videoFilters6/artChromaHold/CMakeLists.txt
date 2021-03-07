INCLUDE(vf_plugin)


IF(DO_QT4)
    IF(ADM_QT_VERSION EQUAL 4)
        SET(QT_TAG qt4)
    ELSE(ADM_QT_VERSION EQUAL 4)
        SET(QT_TAG qt5)
        ADM_QT_ADD_RESOURCES(resource   qt4/artChromaHold.qrc)
    ENDIF(ADM_QT_VERSION EQUAL 4)
ENDIF(DO_QT4)


SET(ArtChromaHoldCommon_SRCS 
ADM_vidArtChromaHold.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ArtChromaHoldQT_SRCS  qt4/Q_artChromaHold.cpp)
SET(ArtChromaHoldQT_HEADERS  qt4/Q_artChromaHold.h)
SET(ArtChromaHoldQT_UI    qt4/artChromaHold)
INIT_VIDEO_FILTER_QT4(ADM_vf_artChromaHold${QT_LIBRARY_EXTENSION} ${ArtChromaHoldQT_SRCS} ${ArtChromaHoldQT_HEADERS} ${ArtChromaHoldQT_UI} ${ArtChromaHoldCommon_SRCS} qt4/DIA_flyArtChromaHold.cpp ${resource})

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ArtChromaHoldCli_SRCS cli/DIA_artChromaHold.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_artChromaHoldCli ${ArtChromaHoldCli_SRCS} ${ArtChromaHoldCommon_SRCS})
#
