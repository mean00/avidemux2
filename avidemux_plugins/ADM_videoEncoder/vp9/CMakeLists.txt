INCLUDE(ve_plugin)

SET(vp9_SRCS
    ADM_vp9.cpp
    ADM_vp9Plugin.cpp
    ADM_vp9Dialog.cpp
)
include(admCheckVpx)
checkVpxEnc()
IF(USE_VP9_ENC)
    MESSAGE(STATUS "VP9 encoder plugin enabled, cflags=${VPX_ENC_CFLAGS}")
    ADD_VIDEO_ENCODER(ADM_ve_vp9 ${vp9_SRCS})
    ADD_DEFINITIONS(${VPX_ENC_CFLAGS})
    INCLUDE_DIRECTORIES(${VPX_ENC_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(ADM_ve_vp9 "${VPX_ENC_LIBRARY_DIR}")
    INIT_VIDEO_ENCODER(ADM_ve_vp9)
    INSTALL_VIDEO_ENCODER(ADM_ve_vp9)
ELSE(USE_VP9_ENC)
    MESSAGE(STATUS "VP9 encoder plugin disabled.")
ENDIF(USE_VP9_ENC)
MESSAGE("")
