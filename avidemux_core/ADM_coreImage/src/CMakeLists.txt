SET(ADM_coreImage_SRCS 
#        ADM_videoFilter.cpp  
        ADM_image.cpp  
        ADM_imageUtils.cpp
        ADM_imageResizer.cpp
#        yuv.cpp
#        rgb2yuv.cpp
        ADM_colorspace.cpp
#        ADM_rgb.cpp
        ADM_pp.cpp
        ADM_print.cpp
        #ADM_vidFieldUtil.cpp
        #ADM_vidFields.cpp
        #ADM_vidFieldASM.cpp
        #ADM_vidFieldBlend.cpp
#        ADM_interlaceUtil.cpp
#        DIA_flyDialog.cpp
        ADM_imageSave.cpp
        ADM_imageOperation.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreImage6 ${ADM_coreImage_SRCS})
ADD_SOURCE_CFLAGS(DIA_flyDialog.cpp " -DADM_UI_TYPE_BUILD=99")
IF (APPLE)
    SET_SOURCE_FILES_PROPERTIES("ADM_imageUtils.cpp" PROPERTIES COMPILE_FLAGS -O0)
    IF (NOT ADM_CPU_X86_64)
        TARGET_LINK_LIBRARIES(ADM_coreImage6 -Wl,-read_only_relocs,suppress)
    ENDIF (NOT ADM_CPU_X86_64)
ENDIF (APPLE)

TARGET_LINK_LIBRARIES(ADM_coreImage6 ADM_core6 ADM_coreUI6 ADM_libswscale ADM_libpostproc ADM_coreUtils6)
TARGET_LINK_LIBRARIES(ADM_coreImage6 ADM_libavcodec ADM_libavutil)

if (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)
	add_dependencies(ADM_coreImage6 libavcodec libavutil libpostproc libswscale)
endif (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)

ADM_INSTALL_LIB(ADM_coreImage6)
