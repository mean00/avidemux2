INCLUDE(vf_plugin)


SET(ADM_vf_decimate_SRCS decimate.cpp   
                         decimate_util.cpp
                         decimate_sse.cpp
                        )

ADD_VIDEO_FILTER(ADM_vf_decimate ${ADM_vf_decimate_SRCS})

INIT_VIDEO_FILTER(ADM_vf_decimate)
INSTALL_VIDEO_FILTER(ADM_vf_decimate)
