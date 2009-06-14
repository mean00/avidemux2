INCLUDE(vf_plugin)


SET(ADM_vf_Denoise_SRCS ADM_vidDenoise.cpp)

ADD_LIBRARY(ADM_vf_Denoise SHARED ${ADM_vf_Denoise_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_Denoise)
INSTALL_VIDEOFILTER(ADM_vf_Denoise)
