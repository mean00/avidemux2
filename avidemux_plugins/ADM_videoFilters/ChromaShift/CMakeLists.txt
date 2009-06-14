INCLUDE(vf_plugin)


SET(chromashiftCommon_SRCS 
ADM_vidChromaShift.cpp
DIA_flyChromaShift.cpp
)

INCLUDE(vf_plugin_gtk)
INCLUDE(vf_plugin_qt4)

SET(chromashiftGtk_SRCS gtk/DIA_chromaShift.cpp)
SET(chromashiftQT_SRCS  qt4/Q_chromashift.cpp)
SET(chromashiftQT_HEADERS  qt4/Q_chromashift.h)
SET(chromashiftQT_UI    qt4/chromashift)

INIT_VIDEOFILTER_PLUGIN_QT4(ADM_vf_chromaShift_qt4 
                ${chromashiftQT_SRCS} ${chromashiftQT_HEADERS} ${chromashiftQT_UI} ${chromashiftCommon_SRCS})
INIT_VIDEOFILTER_PLUGIN_GTK(  ADM_vf_chromaShift_gtk 
                ${chromashiftGtk_SRCS} ${chromashiftCommon_SRCS})

