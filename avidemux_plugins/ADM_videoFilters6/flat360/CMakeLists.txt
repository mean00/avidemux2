INCLUDE(vf_plugin)

SET(Flat360Common_SRCS ADM_vidFlat360.cpp)
IF(DO_QT4)
    include(admCheckThreads)
ENDIF(DO_QT4)
IF(DO_CLI)
    include(admCheckThreads)
ENDIF(DO_CLI)

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(Flat360QT_SRCS  qt4/Q_flat360.cpp)
SET(Flat360QT_HEADERS  qt4/Q_flat360.h)
SET(Flat360QT_UI    qt4/flat360)
INIT_VIDEO_FILTER_QT4(ADM_vf_flat360${QT_LIBRARY_EXTENSION} ${Flat360QT_SRCS} ${Flat360QT_HEADERS} ${Flat360QT_UI} ${Flat360Common_SRCS} qt4/DIA_flyFlat360.cpp)
IF (DO_QT4)
    ADM_LINK_THREAD(ADM_vf_flat360${QT_LIBRARY_EXTENSION} )
ENDIF (DO_QT4)
# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(Flat360Cli_SRCS cli/DIA_flat360.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_flat360Cli ${Flat360Cli_SRCS} ${Flat360Common_SRCS})
IF (DO_CLI)
    ADM_LINK_THREAD(ADM_vf_flat360Cli)
ENDIF (DO_CLI)
#

