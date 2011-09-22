INCLUDE(vf_plugin)
INCLUDE(vf_plugin_qt4gl)


SET(ADM_vf_rotateGlFrag_SRCS rotateGl.cpp)
INIT_VIDEO_FILTER_GLQT4(ADM_vf_rotateGlFrag2 "${ADM_vf_rotateGlFrag_SRCS}" "rotateGl.h" "rotateGl" "${ADM_vf_rotateGl_SRCS}")

