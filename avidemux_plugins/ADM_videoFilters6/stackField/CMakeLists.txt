INCLUDE(vf_plugin)


SET(ADM_vf_stackField_SRCS stackField.cpp)
ADD_VIDEO_FILTER(ADM_vf_stackField ${ADM_vf_stackField_SRCS})
INIT_VIDEO_FILTER(ADM_vf_stackField)
INSTALL_VIDEO_FILTER(ADM_vf_stackField)

SET(ADM_vf_unstackField_SRCS unstackField.cpp)
ADD_VIDEO_FILTER(ADM_vf_unstackField ${ADM_vf_unstackField_SRCS})
INIT_VIDEO_FILTER(ADM_vf_unstackField)
INSTALL_VIDEO_FILTER(ADM_vf_unstackField)

