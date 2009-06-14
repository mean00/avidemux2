INCLUDE(vf_plugin)


SET(ADM_vf_tdeint_SRCS ADM_vidTdeint.cpp  DIA_tdeint.cpp )

ADD_LIBRARY(ADM_vf_tdeint SHARED ${ADM_vf_tdeint_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_tdeint)
INSTALL_VIDEOFILTER(ADM_vf_tdeint)
