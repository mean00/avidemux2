INCLUDE(vf_plugin)
INCLUDE(vf_plugin_qt4gl)


SET(ADM_vf_benchGlFrag_SRCS benchGl.cpp)
INIT_VIDEO_FILTER_GLQT4(ADM_vf_glBenchmark "${ADM_vf_benchGlFrag_SRCS}" "benchGl.h" "" "${ADM_vf_benchGl_SRCS}")

