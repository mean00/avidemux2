INCLUDE(vf_plugin)


SET(CubicLUTCommon_SRCS ADM_vidCubicLUT.cpp)
IF(DO_QT4)
    include(admCheckThreads)
ENDIF(DO_QT4)
IF(DO_CLI)
    include(admCheckThreads)
ENDIF(DO_CLI)

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(CubicLUTQT_SRCS  qt4/Q_cubicLUT.cpp)
SET(CubicLUTQT_HEADERS  qt4/Q_cubicLUT.h)
SET(CubicLUTQT_UI    qt4/cubicLUT)
IF(DO_QT4)
    INIT_VIDEO_FILTER_QT4(ADM_vf_cubicLUT${QT_LIBRARY_EXTENSION} ${CubicLUTQT_SRCS} ${CubicLUTQT_HEADERS} ${CubicLUTQT_UI} ${CubicLUTCommon_SRCS} qt4/DIA_flyCubicLUT.cpp)
    TARGET_LINK_LIBRARIES(ADM_vf_cubicLUT${QT_LIBRARY_EXTENSION} ADM_coreImageLoader6 ADM_coreVideoCodec6)
    ADM_LINK_THREAD(ADM_vf_cubicLUT${QT_LIBRARY_EXTENSION})
ENDIF(DO_QT4)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(CubicLUTCli_SRCS cli/DIA_cubicLUT.cpp)
IF(DO_CLI)
    INIT_VIDEO_FILTER_CLI(  ADM_vf_cubicLUTCli ${CubicLUTCli_SRCS} ${CubicLUTCommon_SRCS})
    TARGET_LINK_LIBRARIES(ADM_vf_cubicLUTCli ADM_coreImageLoader6 ADM_coreVideoCodec6)
    ADM_LINK_THREAD(ADM_vf_cubicLUTCli)
ENDIF(DO_CLI)
#
