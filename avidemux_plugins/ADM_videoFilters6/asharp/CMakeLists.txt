INCLUDE(vf_plugin)


SET(asharpCommon_SRCS 
ADM_vidAsharp.cpp  DIA_flyAsharp.cpp
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(asharpQT_SRCS  qt4/Q_asharp.cpp)
SET(asharpQT_HEADERS  qt4/Q_asharp.h)
SET(asharpQT_UI    qt4/asharp)
INIT_VIDEO_FILTER_QT4(ADM_vf_asharp${QT_LIBRARY_EXTENSION} ${asharpQT_SRCS} ${asharpQT_HEADERS} ${asharpQT_UI} ${asharpCommon_SRCS})
# /QT4


#------------- Gtk Version ---------------
INCLUDE(vf_plugin_gtk)
SET(asharpGtk_SRCS gtk/DIA_asharp.cpp)
INIT_VIDEO_FILTER_GTK(ADM_vf_asharpGtk ${asharpGtk_SRCS} ${asharpCommon_SRCS})

#------------ Cli Version ----------------
#INCLUDE(vf_plugin_cli)
#SET(CropCli_SRCS cli/DIA_crop.cpp)
#INIT_VIDEO_FILTER_CLI(  ADM_vf_CropCli ${CropCli_SRCS} ${CropCommon_SRCS})
#%%%%%%%%%%%%%%%%%%

