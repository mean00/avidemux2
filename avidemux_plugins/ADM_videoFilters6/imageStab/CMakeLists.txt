INCLUDE(vf_plugin)

SET(ImageStabCommon_SRCS ADM_vidImageStab.cpp motest.cpp)
IF(DO_QT4)
    include(admCheckThreads)
ENDIF(DO_QT4)
IF(DO_CLI)
    include(admCheckThreads)
ENDIF(DO_CLI)

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ImageStabQT_SRCS  qt4/Q_imageStab.cpp)
SET(ImageStabQT_HEADERS  qt4/Q_imageStab.h)
SET(ImageStabQT_UI    qt4/imageStab)
INIT_VIDEO_FILTER_QT4(ADM_vf_imageStab${QT_LIBRARY_EXTENSION} ${ImageStabQT_SRCS} ${ImageStabQT_HEADERS} ${ImageStabQT_UI} ${ImageStabCommon_SRCS} qt4/DIA_flyImageStab.cpp)
IF (DO_QT4)
    ADM_LINK_THREAD(ADM_vf_imageStab${QT_LIBRARY_EXTENSION} )
ENDIF (DO_QT4)
# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ImageStabCli_SRCS cli/DIA_imageStab.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_imageStabCli ${ImageStabCli_SRCS} ${ImageStabCommon_SRCS})
IF (DO_CLI)
    ADM_LINK_THREAD(ADM_vf_imageStabCli)
ENDIF (DO_CLI)
#


