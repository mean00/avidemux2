INCLUDE(vf_plugin)


SET(ADM_vf_Deinterlace_SRCS ADM_vidDeinterlace.cpp)

ADD_LIBRARY(ADM_vf_Deinterlace SHARED ${ADM_vf_Deinterlace_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_Deinterlace)
INSTALL_VIDEOFILTER(ADM_vf_Deinterlace)
