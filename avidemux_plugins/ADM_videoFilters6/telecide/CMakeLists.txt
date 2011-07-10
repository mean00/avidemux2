INCLUDE(vf_plugin)


SET(ADM_vf_telecide_SRCS Telecide.cpp   
                         Telecide_utils.cpp
                         Telecide_getFrame.cpp
                        )

ADD_VIDEO_FILTER(ADM_vf_telecide ${ADM_vf_telecide_SRCS})

INIT_VIDEO_FILTER(ADM_vf_telecide)
INSTALL_VIDEO_FILTER(ADM_vf_telecide)
