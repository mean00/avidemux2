INCLUDE(vf_plugin)


SET(chromaShiftCommon_SRCS 
ADM_vidChromaShift.cpp
DIA_flyChromaShift.cpp
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(chromaShiftQT_SRCS     qt4/Q_chromashift.cpp)
SET(chromaShiftQT_HEADERS  qt4/Q_chromashift.h)
SET(chromaShiftQT_UI       qt4/chromashift)
INIT_VIDEO_FILTER_QT4(ADM_vf_chromaShiftQt4 ${chromaShiftQT_SRCS} ${chromaShiftQT_HEADERS} ${chromaShiftQT_UI} ${chromaShiftCommon_SRCS})
# /QT4


#------------- Gtk Version ---------------
INCLUDE(vf_plugin_gtk)
SET(chromaShiftGtk_SRCS gtk/DIA_chromaShift.cpp)
INIT_VIDEO_FILTER_GTK(ADM_vf_chromaShiftGtk ${chromaShiftGtk_SRCS} ${chromaShiftCommon_SRCS})

# ----------------- CLI Version -----------------------
INCLUDE(vf_plugin_cli)
SET(chromaShift_CLI_SRCS    cli/DIA_chromaShift.cpp)
INIT_VIDEO_FILTER_CLI(ADM_vf_chromaShiftCli ${chromaShift_CLI_SRCS} ${chromaShiftCommon_SRCS})

# EOF

