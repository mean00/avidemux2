INCLUDE(vf_plugin)
INCLUDE(vf_plugin_qt4)


SET(ADM_vf_sampleGl_SRCS sampleGl.cpp)

ADD_SOURCE_CFLAGS(sampleGl.cpp " -I${QT_HEADERS_DIR} ")

INIT_VIDEO_FILTER_QT4(ADM_vf_sampleGl "${ADM_vf_sampleGl_SRCS}" "sampleGl.h" "sampleGl" "${ADM_vf_sampleGl_SRCS}")
TARGET_LINK_LIBRARIES(ADM_vf_sampleGl ${OPENGL_LIBRARIES} ${QT_QTOPENGL_LIBRARY})

