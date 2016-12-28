INCLUDE(vf_plugin)
ADD_CORE_INCLUDE(ADM_coreLibVA)

SET(ADM_vf_vaapiFilter_SRCS ADM_vidVaapiFilter.cpp )

ADD_VIDEO_FILTER(ADM_vf_vaapiFilter ${ADM_vf_vaapiFilter_SRCS})
IF(DO_COMMON)        
TARGET_LINK_LIBRARIES( ADM_vf_vaapiFilter ADM_coreLibVA6)
ENDIF(DO_COMMON)        

INIT_VIDEO_FILTER(ADM_vf_vaapiFilter)
INSTALL_VIDEO_FILTER(ADM_vf_vaapiFilter)
