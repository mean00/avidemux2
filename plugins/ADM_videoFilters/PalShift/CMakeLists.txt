INCLUDE(vf_plugin)


SET(ADM_vf_palShift_SRCS ADM_vidPalShift.cpp)
ADD_LIBRARY(ADM_vf_palShift SHARED ${ADM_vf_palShift_SRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_palShift)
INSTALL_VIDEOFILTER(ADM_vf_palShift)


SET(ADM_vf_smartPalShift_SRCS ADM_vidPalSmart.cpp)
ADD_LIBRARY(ADM_vf_smartPalShift SHARED ${ADM_vf_smartPalShift_SRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_smartPalShift)
INSTALL_VIDEOFILTER(ADM_vf_smartPalShift)

