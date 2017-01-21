#
INCLUDE(vf_plugin)

MACRO(addDeps tgt)
        TARGET_LINK_LIBRARIES(${tgt} ADM_libswscale)
        TARGET_LINK_LIBRARIES(${tgt} ADM_libavcodec)
        TARGET_LINK_LIBRARIES(${tgt} ADM_libavutil)
ENDMACRO(addDeps tgt)

IF(DO_QT4)
    IF(ADM_QT_VERSION EQUAL 4)
        SET(QT_TAG qt4)
    ELSE(ADM_QT_VERSION EQUAL 4)
        SET(QT_TAG qt5)
        ADM_QT_ADD_RESOURCES(resource   qt5/delogo.qrc)
    ENDIF(ADM_QT_VERSION EQUAL 4)
ENDIF(DO_QT4)


SET(mpdelogoCommon_SRCS 
ADM_vidMPdelogo.cpp
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(mpdelogoQT_SRCS  ${QT_TAG}/Q_mpdelogo.cpp )
SET(mpdelogoQT_HEADERS  ${QT_TAG}/Q_mpdelogo.h)
SET(mpdelogoQT_UI    ${QT_TAG}/mpdelogo)
IF(DO_QT4)
  INIT_VIDEO_FILTER_QT4(ADM_vf_mpdelogo${QT_LIBRARY_EXTENSION} ${mpdelogoQT_SRCS} ${mpdelogoQT_HEADERS} ${mpdelogoQT_UI} ${mpdelogoCommon_SRCS} ${QT_TAG}/DIA_flyMpDelogo.cpp ${resource})
  addDeps(ADM_vf_mpdelogo${QT_LIBRARY_EXTENSION})
ENDIF(DO_QT4)
# /QT4


#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(mpdelogoCli_SRCS cli/DIA_mpdelogo.cpp)
IF(DO_CLI)
  INIT_VIDEO_FILTER_CLI(  ADM_vf_mpdelogoCli ${mpdelogoCli_SRCS} ${mpdelogoCommon_SRCS})
  addDeps(ADM_vf_mpdelogoCli)
ENDIF(DO_CLI)
##%%%%%%%%%%%%%%%%%%

