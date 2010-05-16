INCLUDE(vf_plugin)
IF(NOT WIN32 AND NOT APPLE)
        SET(ADM_vf_vdpauFilter_SRCS ADM_vidVdpauFilter.cpp )

        ADD_VIDEO_FILTER(ADM_vf_vdpauFilter ${ADM_vf_vdpauFilter_SRCS})

        INIT_VIDEO_FILTER(ADM_vf_vdpauFilter)
        INSTALL_VIDEO_FILTER(ADM_vf_vdpauFilter)
ENDIF(NOT WIN32 AND NOT APPLE)
