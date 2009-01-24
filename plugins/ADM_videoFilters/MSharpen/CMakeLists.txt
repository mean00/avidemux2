INCLUDE(vf_plugin)


SET(ADM_vf_mSharpen_SRCS ADM_vidMSharpen.cpp)

ADD_LIBRARY(ADM_vf_mSharpen SHARED ${ADM_vf_mSharpen_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_mSharpen)
INSTALL_VIDEOFILTER(ADM_vf_mSharpen)
