INCLUDE(vf_plugin)


SET(eq2Common_SRCS 
ADM_vidEq2.cpp
DIA_flyEq2.cpp
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(eq2QT_SRCS     qt4/Q_eq2.cpp)
SET(eq2QT_HEADERS  qt4/Q_eq2.h)
SET(eq2QT_UI       qt4/eq2)
INIT_VIDEO_FILTER_QT4(ADM_vf_eq2${QT_LIBRARY_EXTENSION} ${eq2QT_SRCS} ${eq2QT_HEADERS} ${eq2QT_UI} ${eq2Common_SRCS})
# /QT4


#------------- Gtk Version ---------------
INCLUDE(vf_plugin_gtk)
SET(eq2Gtk_SRCS gtk/DIA_eq2.cpp)
INIT_VIDEO_FILTER_GTK(ADM_vf_eq2Gtk ${eq2Gtk_SRCS} ${eq2Common_SRCS})

# ----------------- CLI Version -----------------------
INCLUDE(vf_plugin_cli)
SET(eq2_CLI_SRCS    cli/DIA_eq2.cpp)
INIT_VIDEO_FILTER_CLI(ADM_vf_eq2Cli ${eq2_CLI_SRCS} ${eq2Common_SRCS})

# EOF


