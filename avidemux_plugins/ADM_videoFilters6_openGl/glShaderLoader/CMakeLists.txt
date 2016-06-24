INCLUDE(vf_plugin)
INCLUDE(vf_plugin_qt4gl)


SET(ADM_vf_shaderLoaderGlFrag_SRCS shader.cpp)
INIT_VIDEO_FILTER_GLQT4(ADM_vf_shaderLoaderGl "${ADM_vf_shaderLoaderGlFrag_SRCS}" "" "" ${ADM_vf_shaderLoaderGl_SRCS})

