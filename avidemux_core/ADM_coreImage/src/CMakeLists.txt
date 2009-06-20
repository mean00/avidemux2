SET(ADM_coreImage_SRCS 
	ADM_confCouple.cpp  
        ADM_videoFilter.cpp  
        ADM_videoFilterCache.cpp
        ADM_image.cpp  
        ADM_imageUtils.cpp
        ADM_imageResizer.cpp
        yuv.cpp
        rgb2yuv.cpp
        ADM_colorspace.cpp
        ADM_rgb.cpp
        ADM_pp.cpp
        ADM_print.cpp
        ADM_vidFieldUtil.cpp
        ADM_vidFields.cpp
        ADM_vidFieldASM.cpp
        ADM_vidFieldBlend.cpp
        ADM_interlaceUtil.cpp
        DIA_flyDialog.cpp
)

ADD_LIBRARY(ADM_coreImage SHARED ${ADM_coreImage_SRCS})
ADD_SOURCE_CFLAGS(DIA_flyDialog.cpp " -DADM_UI_TYPE_BUILD=99")
IF (APPLE)
	TARGET_LINK_LIBRARIES(ADM_coreImage -Wl,-read_only_relocs,suppress)
ENDIF (APPLE)

TARGET_LINK_LIBRARIES(ADM_coreImage ADM_core ADM_coreUI ADM_libswscale ADM_libpostproc)
ADM_INSTALL_LIB(ADM_coreImage)
