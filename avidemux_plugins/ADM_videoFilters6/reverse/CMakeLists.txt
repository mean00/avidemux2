INCLUDE(vf_plugin)

SET(ReverseCommon_SRCS ADM_vidReverse.cpp)


# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ReverseQT4_SRCS    qt4/Q_reverse.cpp)
SET(ReverseQT4_Headers qt4/Q_reverse.h)
SET(ReverseQT4_UI      qt4/reverse)
INIT_VIDEO_FILTER_QT4(ADM_vf_reverse${QT_LIBRARY_EXTENSION} ${ReverseQT4_SRCS} ${ReverseQT4_Headers} ${ReverseQT4_UI} ${ReverseCommon_SRCS} )
IF(DO_QT4)
    TARGET_LINK_LIBRARIES(ADM_vf_reverse${QT_LIBRARY_EXTENSION} ADM_libswscale)
ENDIF(DO_QT4)
# /QT4

# ----------------- CLI Version -----------------------
INCLUDE(vf_plugin_cli)
SET(ReverseCli_SRCS    cli/DIA_reverse.cpp)
INIT_VIDEO_FILTER_CLI(ADM_vf_reverseCli ${ReverseCli_SRCS} ${ReverseCommon_SRCS})
IF(DO_CLI)
    TARGET_LINK_LIBRARIES(ADM_vf_reverseCli ADM_libswscale)
ENDIF(DO_CLI)

# /QT4
