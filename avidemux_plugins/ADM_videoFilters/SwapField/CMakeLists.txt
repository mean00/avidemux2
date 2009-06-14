INCLUDE(vf_plugin)


SET(ADM_vf_swapField_SRCS ADM_vidSwapFields.cpp)
ADD_LIBRARY(ADM_vf_swapField SHARED ${ADM_vf_swapField_SRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_swapField)
INSTALL_VIDEOFILTER(ADM_vf_swapField)


SET(ADM_vf_smartSwapField_SRCS ADM_vidSwapSmart.cpp)
ADD_LIBRARY(ADM_vf_smartSwapField SHARED ${ADM_vf_smartSwapField_SRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_smartSwapField)
INSTALL_VIDEOFILTER(ADM_vf_smartSwapField)


SET(ADM_vf_keepOddField_SRCS ADM_vidKeepOddFields.cpp)
ADD_LIBRARY(ADM_vf_keepOddField SHARED ${ADM_vf_keepOddField_SRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_keepOddField)
INSTALL_VIDEOFILTER(ADM_vf_keepOddField)

SET(ADM_vf_keepEvenField_SRCS ADM_vidKeepEvenFields.cpp)
ADD_LIBRARY(ADM_vf_keepEvenField SHARED ${ADM_vf_keepEvenField_SRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_keepEvenField)
INSTALL_VIDEOFILTER(ADM_vf_keepEvenField)

