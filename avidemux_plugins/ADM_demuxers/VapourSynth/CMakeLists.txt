include(dm_plugin)
ADD_DEFINITIONS("-std=c++11")
SET(ADM_vs_SRCS 
	ADM_vs.cpp
	ADM_vsPlugin.cpp
        )

ADD_DEMUXER(ADM_dm_vs ${ADM_vs_SRCS})

TARGET_LINK_LIBRARIES(ADM_dm_vs vapoursynth-script vapoursynth)

INIT_DEMUXER(ADM_dm_vs)
INSTALL_DEMUXER(ADM_dm_vs)
