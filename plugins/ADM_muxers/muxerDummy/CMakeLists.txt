include(mx_plugin)
SET(ADM_dummy_SRCS 
	ADM_dummy.cpp
	ADM_dummyPlugin.cpp
)

ADD_LIBRARY(ADM_mx_dummy SHARED ${ADM_dummy_SRCS})

#TARGET_LINK_LIBRARIES(ADM_dm_flv ADM_libavutil)

INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg")

INIT_MUXER(ADM_mx_dummy)
INSTALL_MUXER(ADM_mx_dummy)
