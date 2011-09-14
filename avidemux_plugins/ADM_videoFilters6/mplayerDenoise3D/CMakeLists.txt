INCLUDE(vf_plugin)


#SET(ADM_vf_denoise3dhq_SRCS ADM_vidMPLD3D.cpp)
#ADD_VIDEO_FILTER(ADM_vf_denoise3dhq SHARED ${ADM_vf_denoise3dhq_SRCS})
#INIT_VIDEO_FILTER(ADM_vf_denoise3dhq)
#INSTALL_VIDEO_FILTER(ADM_vf_denoise3dhq)

SET(ADM_vf_denoise3d_SRCS ADM_vidMPLD3Dlow.cpp)
ADD_VIDEO_FILTER(ADM_vf_denoise3d SHARED ${ADM_vf_denoise3d_SRCS})
INIT_VIDEO_FILTER(ADM_vf_denoise3d)
INSTALL_VIDEO_FILTER(ADM_vf_denoise3d)
