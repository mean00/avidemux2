INCLUDE(vf_plugin)


SET(ADM_vf_denoise3dhq_SRCS ADM_vidMPLD3D.cpp)
ADD_LIBRARY(ADM_vf_denoise3dhq SHARED ${ADM_vf_denoise3dhq_SRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_denoise3dhq)
INSTALL_VIDEOFILTER(ADM_vf_denoise3dhq)

SET(ADM_vf_denoise3d_SRCS ADM_vidMPLD3Dlow.cpp)
ADD_LIBRARY(ADM_vf_denoise3d SHARED ${ADM_vf_denoise3d_SRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_denoise3d)
INSTALL_VIDEOFILTER(ADM_vf_denoise3d)
