SET(ADM_codecs_SRCS 
ADM_codecs.cpp
ADM_png.cpp
ADM_theora_dec.cpp
ADM_ffmp43.cpp
ADM_mjpeg.cpp
ADM_rgb16.cpp
ADM_uyvy.cpp
ADM_ffmpeg_vdpau.cpp
)

ADD_LIBRARY(ADM_codecs STATIC ${ADM_codecs_SRCS})

IF (USE_PNG)
	 ADD_SOURCE_CFLAGS(ADM_png.cpp ${PNG_CFLAGS})
ENDIF (USE_PNG)

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)
