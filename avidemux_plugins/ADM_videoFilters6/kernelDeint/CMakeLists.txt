INCLUDE(vf_plugin)


SET(ADM_vf_kernelDeint_SRCS ADM_vidKernelDeint.cpp)

ADD_VIDEO_FILTER(ADM_vf_kernelDeint ${ADM_vf_kernelDeint_SRCS})

INIT_VIDEO_FILTER(ADM_vf_kernelDeint)
INSTALL_VIDEO_FILTER(ADM_vf_kernelDeint)
