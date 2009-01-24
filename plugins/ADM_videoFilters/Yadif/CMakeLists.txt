INCLUDE(vf_plugin)


SET(ADM_vf_yadif_SRCS ADM_vidYadif_asm.c  ADM_vidYadif.cpp )

ADD_LIBRARY(ADM_vf_yadif SHARED ${ADM_vf_yadif_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_yadif)
INSTALL_VIDEOFILTER(ADM_vf_yadif)
