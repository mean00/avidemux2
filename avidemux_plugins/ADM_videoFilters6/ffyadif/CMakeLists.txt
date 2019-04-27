INCLUDE(vf_plugin)
SET(ADM_vf_ffyadif_SRCS ADM_vidffYadif.cpp vf_yadif.asm)

YASMIFY(asm vf_yadif)
ADD_VIDEO_FILTER(ADM_vf_ffyadif ${ADM_vf_ffyadif_SRCS} ${asm})

INIT_VIDEO_FILTER(ADM_vf_ffyadif)
INSTALL_VIDEO_FILTER(ADM_vf_ffyadif)
