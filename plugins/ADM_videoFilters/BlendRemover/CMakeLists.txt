INCLUDE(vf_plugin)


SET(ADM_vf_blendRemoval_SRCS ADM_vidBlendRemoval.cpp)

ADD_LIBRARY(ADM_vf_blendRemoval SHARED ${ADM_vf_blendRemoval_SRCS})
TARGET_LINK_LIBRARIES(ADM_vf_blendRemoval ADM_coreImage)

INIT_VIDEOFILTER_PLUGIN(ADM_vf_blendRemoval)
INSTALL_VIDEOFILTER(ADM_vf_blendRemoval)
