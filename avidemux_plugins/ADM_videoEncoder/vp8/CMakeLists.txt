INCLUDE(ve_plugin)

SET(vp8_SRCS
    ADM_vp8.cpp
    ADM_vp8Plugin.cpp
    ADM_vp8Dialog.cpp
)
include(admCheckVpx)
checkVpxEnc()
IF(USE_VP9_ENC)
    MESSAGE(STATUS "VP8 encoder plugin enabled, cflags=${VPX_ENC_CFLAGS}")
    ADD_VIDEO_ENCODER(ADM_ve_vp8 ${vp8_SRCS})
    ADD_DEFINITIONS(${VPX_ENC_CFLAGS})
    INCLUDE_DIRECTORIES(${VPX_ENC_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(ADM_ve_vp8 "${VPX_ENC_LIBRARY_DIR}")
    INIT_VIDEO_ENCODER(ADM_ve_vp8)
    INSTALL_VIDEO_ENCODER(ADM_ve_vp8)
ELSE(USE_VP9_ENC)
    MESSAGE(STATUS "VP8 encoder plugin disabled.")
ENDIF(USE_VP9_ENC)
MESSAGE("")
