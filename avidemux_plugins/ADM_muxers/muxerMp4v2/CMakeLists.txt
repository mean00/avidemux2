include(mx_plugin)
SET(ADM_mp4v2_SRCS
	muxerMp4v2Plugin.cpp
	muxerMp4v2.cpp
	muxerMp4v2Audio.cpp
	muxerMp4v2Video.cpp
	muxerMp4v2Config.cpp
)
SUBDIRS(libmp4v2)

ADD_LIBRARY(ADM_mx_mp4v2 SHARED ${ADM_mp4v2_SRCS})
TARGET_LINK_LIBRARIES(ADM_mx_mp4v2 ADM_libmp4v2)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libmp4v2/include/)
INIT_MUXER(ADM_mx_mp4v2)
INSTALL_MUXER(ADM_mx_mp4v2)
