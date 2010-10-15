INCLUDE(vf_plugin)


SET(ADM_vf_largeMedian_SRCS ADM_largeMedian.cpp)

ADD_VIDEO_FILTER(ADM_vf_largeMedian ${ADM_vf_largeMedian_SRCS})

INIT_VIDEO_FILTER(ADM_vf_largeMedian)
INSTALL_VIDEO_FILTER(ADM_vf_largeMedian)
