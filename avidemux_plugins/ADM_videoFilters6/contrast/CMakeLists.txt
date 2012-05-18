#%%
INCLUDE(vf_plugin)

SET(contrastCommon_SRCS 
ADM_vidContrast.cpp  DIA_flyContrast.cpp
)

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(contrastQT_SRCS     qt4/Q_contrast.cpp)
SET(contrastQT_HEADERS  qt4/Q_contrast.h)
SET(contrastQT_UI       qt4/contrast)
INIT_VIDEO_FILTER_QT4(ADM_vf_contrastQt4 ${contrastQT_SRCS} ${contrastQT_HEADERS} ${contrastQT_UI} ${contrastCommon_SRCS})
# /QT4


# ----------------- CLI Version -----------------------
INCLUDE(vf_plugin_cli)
SET(contrast_CLI_SRCS    cli/DIA_contrast.cpp)
INIT_VIDEO_FILTER_CLI(ADM_vf_contrastCli ${contrast_CLI_SRCS} ${contrastCommon_SRCS})

# ----------------- Gtk Version -----------------------
INCLUDE(vf_plugin_gtk)
SET(contrast_GTK_SRCS    cli/DIA_contrast.cpp)
INIT_VIDEO_FILTER_GTK(ADM_vf_contrastGtk ${contrast_GTK_SRCS} ${contrastCommon_SRCS})
# EOF

