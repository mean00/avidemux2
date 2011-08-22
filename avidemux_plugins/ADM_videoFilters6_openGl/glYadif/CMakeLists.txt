INCLUDE(vf_plugin)
INCLUDE(vf_plugin_qt4gl)


SET(ADM_vf_glSmooth_SRCS glSmooth.cpp)
INIT_VIDEO_FILTER_GLQT4(ADM_vf_glSmooth "${ADM_vf_glSmooth_SRCS}" "glSmooth.h" "glSmooth" "${ADM_vf_glSmooth_SRCS}")

