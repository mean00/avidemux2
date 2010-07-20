INCLUDE(vf_plugin)
IF(NOT WIN32 AND NOT APPLE)
        SET(ADM_vf_vdpauFilter_SRCS ADM_vidVdpauFilter.cpp )
        SET(ADM_vf_vdpauFilterDeint_SRCS ADM_vidVdpauFilterDeint.cpp )

        ADD_VIDEO_FILTER(ADM_vf_vdpauFilter ${ADM_vf_vdpauFilter_SRCS})
        ADD_VIDEO_FILTER(ADM_vf_vdpauFilterDeint ${ADM_vf_vdpauFilterDeint_SRCS})

        INIT_VIDEO_FILTER(ADM_vf_vdpauFilter)
        INIT_VIDEO_FILTER(ADM_vf_vdpauFilterDeint)
        INSTALL_VIDEO_FILTER(ADM_vf_vdpauFilter)
        INSTALL_VIDEO_FILTER(ADM_vf_vdpauFilterDeint)
ENDIF(NOT WIN32 AND NOT APPLE)
