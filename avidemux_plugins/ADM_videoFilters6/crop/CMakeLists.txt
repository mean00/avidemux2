# Crop filter
INCLUDE(vf_plugin)

SET(CropCommon_SRCS ADM_vidCrop.cpp  DIA_flyCrop.cpp)

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ADM_vf_cropQT4_SRCS    qt4/Q_crop.cpp)
SET(ADM_vf_cropQT4_Headers qt4/Q_crop.h)
SET(ADM_vf_cropQT4_UI      qt4/crop)
INIT_VIDEO_FILTER_QT4(ADM_vf_crop${QT_LIBRARY_EXTENSION} ${ADM_vf_cropQT4_SRCS} ${ADM_vf_cropQT4_Headers} ${ADM_vf_cropQT4_UI} ${CropCommon_SRCS})
# /QT4


#------------- Gtk Version ---------------
INCLUDE(vf_plugin_gtk)
SET(CropGtk_SRCS gtk/DIA_crop.cpp)
INIT_VIDEO_FILTER_GTK(ADM_vf_cropGtk ${CropGtk_SRCS} ${CropCommon_SRCS})

#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(CropCli_SRCS cli/DIA_crop.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_CropCli ${CropCli_SRCS} ${CropCommon_SRCS})
#

