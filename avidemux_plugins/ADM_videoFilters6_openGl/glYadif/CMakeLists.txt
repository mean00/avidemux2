INCLUDE(vf_plugin)
INCLUDE(vf_plugin_qt4gl)


SET(ADM_vf_glYadif_SRCS glYadif.cpp)
INIT_VIDEO_FILTER_GLQT4(ADM_vf_glYadif "${ADM_vf_glYadif_SRCS}" "glYadif.h" "glYadif" "${ADM_vf_glYadif_SRCS}")

