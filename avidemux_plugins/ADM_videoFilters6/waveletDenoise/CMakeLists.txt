INCLUDE(vf_plugin)


SET(WaveletDenoiseCommon_SRCS 
ADM_vidWaveletDenoise.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(WaveletDenoiseQT_SRCS  qt4/Q_waveletDenoise.cpp)
SET(WaveletDenoiseQT_HEADERS  qt4/Q_waveletDenoise.h)
SET(WaveletDenoiseQT_UI    qt4/waveletDenoise)
INIT_VIDEO_FILTER_QT4(ADM_vf_waveletDenoise${QT_LIBRARY_EXTENSION} ${WaveletDenoiseQT_SRCS} ${WaveletDenoiseQT_HEADERS} ${WaveletDenoiseQT_UI} ${WaveletDenoiseCommon_SRCS} qt4/DIA_flyWaveletDenoise.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(WaveletDenoiseCli_SRCS cli/DIA_waveletDenoise.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_waveletDenoiseCli ${WaveletDenoiseCli_SRCS} ${WaveletDenoiseCommon_SRCS})
#
