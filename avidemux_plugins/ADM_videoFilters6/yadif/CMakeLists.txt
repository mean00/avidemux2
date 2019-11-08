INCLUDE(vf_plugin)

SET(ADM_vf_yadif_SRCS ADM_vidYadif.cpp )

YASMIFY(azm vf_yadif)

ADD_VIDEO_FILTER(ADM_vf_yadif ${ADM_vf_yadif_SRCS} ${azm})

INIT_VIDEO_FILTER(ADM_vf_yadif)
INSTALL_VIDEO_FILTER(ADM_vf_yadif)
