INCLUDE(vf_plugin)


SET(ADM_vf_kernelDeint_SRCS ADM_vidKernelDeint.cpp)

ADD_LIBRARY(ADM_vf_kernelDeint SHARED ${ADM_vf_kernelDeint_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_kernelDeint)
INSTALL_VIDEOFILTER(ADM_vf_kernelDeint)
