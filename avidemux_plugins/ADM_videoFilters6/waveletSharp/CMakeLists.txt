INCLUDE(vf_plugin)


SET(WaveletSharpCommon_SRCS 
ADM_vidWaveletSharp.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(WaveletSharpQT_SRCS  qt4/Q_waveletSharp.cpp)
SET(WaveletSharpQT_HEADERS  qt4/Q_waveletSharp.h)
SET(WaveletSharpQT_UI    qt4/waveletSharp)
INIT_VIDEO_FILTER_QT4(ADM_vf_waveletSharp${QT_LIBRARY_EXTENSION} ${WaveletSharpQT_SRCS} ${WaveletSharpQT_HEADERS} ${WaveletSharpQT_UI} ${WaveletSharpCommon_SRCS} qt4/DIA_flyWaveletSharp.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(WaveletSharpCli_SRCS cli/DIA_waveletSharp.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_waveletSharpCli ${WaveletSharpCli_SRCS} ${WaveletSharpCommon_SRCS})
#
