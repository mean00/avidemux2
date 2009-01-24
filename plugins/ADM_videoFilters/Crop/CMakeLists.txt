INCLUDE(vf_plugin)


SET(CropCommon_SRCS 
ADM_vidCrop.cpp  DIA_flyCrop.cpp
)

INCLUDE(vf_plugin_gtk)
INCLUDE(vf_plugin_qt4)

SET(CropGtk_SRCS gtk/DIA_crop.cpp)
SET(CropQT_SRCS  qt4/Q_crop.cpp)
SET(CropQT_HEADERS  qt4/Q_crop.h)
SET(CropQT_UI    qt4/crop)

INIT_VIDEOFILTER_PLUGIN_QT4(ADM_vf_crop_qt4 
                ${CropQT_SRCS} ${CropQT_HEADERS} ${CropQT_UI} ${CropCommon_SRCS})
INIT_VIDEOFILTER_PLUGIN_GTK(  ADM_vf_Crop_gtk 
                ${CropGtk_SRCS} ${CropCommon_SRCS})

