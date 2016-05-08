INCLUDE(vf_plugin)


SET(ADM_vf_ivtcDupeRemover_SRCS 
                ADM_ivtcDupeRemover.cpp   
        )

ADD_VIDEO_FILTER(ADM_vf_ivtcDupeRemover ${ADM_vf_telecide_SRCS})

INIT_VIDEO_FILTER(ADM_vf_ivtcDupeRemover)
INSTALL_VIDEO_FILTER(ADM_vf_ivtcDupeRemover)
