INCLUDE(vf_plugin)
INCLUDE(vf_plugin_qt4gl)


SET(ADM_vf_sampleGlVertex_SRCS sampleGl.cpp)
INIT_VIDEO_FILTER_GLQT4(ADM_vf_sampleGlVertex "${ADM_vf_sampleGl_SRCS}" "sampleGl.h" "sampleGl" "${ADM_vf_sampleGl_SRCS}")

