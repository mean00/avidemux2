INCLUDE(ve_plugin)
#
include(admCheckLibVA)
checkLibVA()
include_directories(${ADM_HEADER_DIR}/ADM_coreLibVA)        
include_directories(${ADM_HEADER_DIR}/ADM_coreLibVAEnc/)
ADD_VIDEO_ENCODER(ADM_ve_libva  
                libvaencPlugin.cpp 
                ADM_libVaEncodingContextH264_misc.cpp
                ADM_libVaEncodingContextH264_render.cpp
                ADM_libVaEncodingContextH264_packed.cpp
                ADM_libvaEncoder.cpp  
                ADM_libVaEncodingContextH264.cpp
                    )

TARGET_LINK_LIBRARIES(ADM_ve_libva ADM_coreLibVAEnc6 ADM_coreLibVA6 va)
INIT_VIDEO_ENCODER(ADM_ve_libva)
INSTALL_VIDEO_ENCODER(ADM_ve_libva)
SET(ADM_vf_logo_SRCS ADM_vidLogo.cpp)

