INCLUDE(vf_plugin)


SET(ADM_vf_blackenBorders_SRCS ADM_vidBlackBorder.cpp )


# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ADM_vf_blackenBordersQT4_SRCS    qt4/Q_blackenBorders.cpp)
SET(ADM_vf_blackenBordersQT4_Headers qt4/Q_blackenBorders.h)
SET(ADM_vf_blackenBordersQT4_UI      qt4/blackenBorders)
IF(DO_QT4)
INIT_VIDEO_FILTER_QT4(ADM_vf_blackenBorders${QT_LIBRARY_EXTENSION} ${ADM_vf_blackenBordersQT4_SRCS} ${ADM_vf_blackenBordersQT4_Headers} ${ADM_vf_blackenBordersQT4_UI} ${ADM_vf_blackenBorders_SRCS} qt4/DIA_flyBlackenBorders.cpp)
ENDIF(DO_QT4)
# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(CropCli_SRCS cli/DIA_blackenBorders.cpp)
IF(DO_CLI)
INIT_VIDEO_FILTER_CLI(  ADM_vf_blackenBordersCli ${CropCli_SRCS} ${CropCommon_SRCS})
ENDIF(DO_CLI)
#
