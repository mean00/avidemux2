INCLUDE(vf_plugin)


SET(ADM_vf_fieldsAlign_SRCS ADM_fieldsAlign.cpp)

ADD_VIDEO_FILTER(ADM_vf_fieldsAlign ${ADM_vf_fieldsAlign_SRCS})

INIT_VIDEO_FILTER(ADM_vf_fieldsAlign)
INSTALL_VIDEO_FILTER(ADM_vf_fieldsAlign)
