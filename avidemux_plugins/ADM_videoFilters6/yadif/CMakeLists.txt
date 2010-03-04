INCLUDE(vf_plugin)

SET(ADM_vf_yadif_SRCS ADM_vidYadif.cpp ADM_vidYadif_asm.c)

ADD_VIDEO_FILTER(ADM_vf_yadif ${ADM_vf_yadif_SRCS})

INIT_VIDEO_FILTER(ADM_vf_yadif)
INSTALL_VIDEO_FILTER(ADM_vf_yadif)
