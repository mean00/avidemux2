INCLUDE(vf_plugin)


SET(ADM_vf_DgBob_SRCS ADM_vidDGbob.cpp dgBob_json.cpp)

ADD_VIDEO_FILTER(ADM_vf_DgBob ${ADM_vf_DgBob_SRCS})

INIT_VIDEO_FILTER(ADM_vf_DgBob)
INSTALL_VIDEO_FILTER(ADM_vf_DgBob)
