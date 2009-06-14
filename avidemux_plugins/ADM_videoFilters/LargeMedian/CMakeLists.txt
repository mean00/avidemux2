INCLUDE(vf_plugin)


SET(ADM_vf_largemedian_SRCS ADM_vidLargeMedian.cpp)

ADD_LIBRARY(ADM_vf_largemedian SHARED ${ADM_vf_largemedian_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_largemedian)
INSTALL_VIDEOFILTER(ADM_vf_largemedian)
