SET(ADM_codecs_SRCS 
ADM_codecs.cpp
ADM_png.cpp
ADM_ffmp43.cpp
ADM_codecFFsimple.cpp
ADM_rgb16.cpp
ADM_uyvy.cpp
ADM_ffmpeg_vdpau.cpp
ADM_png.cpp
ADM_codecSearch.cpp
)

ADD_LIBRARY(ADM_coreVideoCodec6 SHARED ${ADM_codecs_SRCS})
ADD_SOURCE_CFLAGS(ADM_png.cpp ${PNG_CFLAGS})
ADM_INSTALL_LIB(ADM_coreVideoCodec6)

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)
