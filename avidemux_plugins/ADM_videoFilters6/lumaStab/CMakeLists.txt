INCLUDE(vf_plugin)


SET(LumaStabCommon_SRCS 
ADM_vidLumaStab.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(LumaStabQT_SRCS  qt4/Q_lumaStab.cpp)
SET(LumaStabQT_HEADERS  qt4/Q_lumaStab.h)
SET(LumaStabQT_UI    qt4/lumaStab)
INIT_VIDEO_FILTER_QT4(ADM_vf_lumaStab${QT_LIBRARY_EXTENSION} ${LumaStabQT_SRCS} ${LumaStabQT_HEADERS} ${LumaStabQT_UI} ${LumaStabCommon_SRCS} qt4/DIA_flyLumaStab.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(LumaStabCli_SRCS cli/DIA_lumaStab.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_lumaStabCli ${LumaStabCli_SRCS} ${LumaStabCommon_SRCS})
#
