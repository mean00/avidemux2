INCLUDE(vf_plugin)


SET(DelogoHQCommon_SRCS 
ADM_vidDelogoHQ.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(DelogoHQQT_SRCS  qt4/Q_delogoHQ.cpp)
SET(DelogoHQQT_HEADERS  qt4/Q_delogoHQ.h)
SET(DelogoHQQT_UI    qt4/delogoHQ)
IF(DO_QT4)
    INIT_VIDEO_FILTER_QT4(ADM_vf_delogoHQ${QT_LIBRARY_EXTENSION} ${DelogoHQQT_SRCS} ${DelogoHQQT_HEADERS} ${DelogoHQQT_UI} ${DelogoHQCommon_SRCS} qt4/DIA_flyDelogoHQ.cpp)
    TARGET_LINK_LIBRARIES(ADM_vf_delogoHQ${QT_LIBRARY_EXTENSION} ADM_coreImageLoader6)
ENDIF(DO_QT4)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(DelogoHQCli_SRCS cli/DIA_delogoHQ.cpp)
IF(DO_CLI)
    INIT_VIDEO_FILTER_CLI(  ADM_vf_delogoHQCli ${DelogoHQCli_SRCS} ${DelogoHQCommon_SRCS})
    TARGET_LINK_LIBRARIES(ADM_vf_delogoHQCli ADM_coreImageLoader6)
ENDIF(DO_CLI)
#
