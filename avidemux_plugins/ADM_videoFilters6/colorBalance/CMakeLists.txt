INCLUDE(vf_plugin)


SET(ColorBalanceCommon_SRCS 
ADM_vidColorBalance.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ColorBalanceQT_SRCS  qt4/Q_colorBalance.cpp )
SET(ColorBalanceQT_HEADERS  qt4/Q_colorBalance.h )
SET(ColorBalanceQT_UI    qt4/colorBalance)
INIT_VIDEO_FILTER_QT4(ADM_vf_colorBalance${QT_LIBRARY_EXTENSION} ${ColorBalanceQT_SRCS} ${ColorBalanceQT_CPSRC} ${ColorBalanceQT_HEADERS} ${ColorBalanceQT_UI} ${ColorBalanceCommon_SRCS} qt4/DIA_flyColorBalance.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ColorBalanceCli_SRCS cli/DIA_colorBalance.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_colorBalanceCli ${ColorBalanceCli_SRCS} ${ColorBalanceCommon_SRCS})
#
