# Crop filter
INCLUDE(vf_plugin)

SET(CropCommon_SRCS ADM_vidCrop.cpp  )

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
IF(DO_QT4)
  IF(ADM_QT_VERSION EQUAL 4)
    SET(QT_TAG qt4)
  ELSE(ADM_QT_VERSION EQUAL 4)
      SET(QT_TAG qt5)
  ENDIF(ADM_QT_VERSION EQUAL 4)
  SET(ADM_vf_cropQT4_SRCS    ${QT_TAG}/Q_crop.cpp)
  SET(ADM_vf_cropQT4_Headers ${QT_TAG}/Q_crop.h)
  SET(ADM_vf_cropQT4_UI      ${QT_TAG}/crop)
  INIT_VIDEO_FILTER_QT4(ADM_vf_crop${QT_LIBRARY_EXTENSION} ${ADM_vf_cropQT4_SRCS} ${ADM_vf_cropQT4_Headers} ${ADM_vf_cropQT4_UI} ${CropCommon_SRCS} 
        ${QT_TAG}/DIA_flyCrop.cpp)
ENDIF(DO_QT4)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(CropCli_SRCS cli/DIA_crop.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_CropCli ${CropCli_SRCS} ${CropCommon_SRCS})
#

