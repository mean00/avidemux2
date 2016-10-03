INCLUDE(ve_plugin)

SET(LIBVA_SRCS 
        libvaencPlugin.cpp
        ADM_libvaEncoder.cpp
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_VIDEO_ENCODER(ADM_ve_libva  ${LIBVA_SRCS})
TARGET_LINK_LIBRARIES(ADM_ve_libva ADM_coreLibVA6)
INIT_VIDEO_ENCODER(ADM_ve_libva)
INSTALL_VIDEO_ENCODER(ADM_ve_libva)
