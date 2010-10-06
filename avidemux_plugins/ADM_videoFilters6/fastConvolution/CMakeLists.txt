INCLUDE(vf_plugin)

MACRO(CONV_FILTER FILTERNAME FILTERSRC)
ADD_VIDEO_FILTER( ${FILTERNAME} ${FILTERSRC} ADM_vidFastConvolution.cpp )
INIT_VIDEO_FILTER(${FILTERNAME})
INSTALL_VIDEO_FILTER(${FILTERNAME})
ENDMACRO(CONV_FILTER FILTERNAME FILTERSRC)

CONV_FILTER(ADM_vf_mean    Mean.cpp )
CONV_FILTER(ADM_vf_gauss   Gauss.cpp )
CONV_FILTER(ADM_vf_median  Median.cpp )
CONV_FILTER(ADM_vf_sharpen Sharpen.cpp )


#SET(ADM_vf_fastconvolutionsharpen_SRCS ADM_vidFastConvolution.cpp Sharpen.cpp )
#ADD_LIBRARY(ADM_vf_fastconvolutionsharpen SHARED ${ADM_vf_fastconvolutionsharpen_SRCS})
#INIT_VIDEOFILTER_PLUGIN(ADM_vf_fastconvolutionsharpen)
#INSTALL_VIDEOFILTER(ADM_vf_fastconvolutionsharpen)

