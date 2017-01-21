INCLUDE(vf_plugin)


SET(chromaShiftCommon_SRCS 
ADM_vidChromaShift.cpp
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(chromaShiftQT_SRCS     qt4/Q_chromashift.cpp)
SET(chromaShiftQT_HEADERS  qt4/Q_chromashift.h)
SET(chromaShiftQT_UI       qt4/chromashift)
INIT_VIDEO_FILTER_QT4(ADM_vf_chromaShift${QT_LIBRARY_EXTENSION} ${chromaShiftQT_SRCS} ${chromaShiftQT_HEADERS} ${chromaShiftQT_UI} ${chromaShiftCommon_SRCS} DIA_flyChromaShift.cpp)
# /QT4


# ----------------- CLI Version -----------------------
INCLUDE(vf_plugin_cli)
SET(chromaShift_CLI_SRCS    cli/DIA_chromaShift.cpp)
INIT_VIDEO_FILTER_CLI(ADM_vf_chromaShiftCli ${chromaShift_CLI_SRCS} ${chromaShiftCommon_SRCS})

# EOF

