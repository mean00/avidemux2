SET(ADM_codecs_SRCS 
	ADM_codecs.cpp  ADM_ffmpeg.cpp  ADM_mjpegEncode.cpp  ADM_png.cpp    ADM_theora_dec.cpp   ADM_xvid.cpp
	ADM_ffmp43.cpp  ADM_mjpeg.cpp   ADM_rgb16.cpp  ADM_uyvy.cpp        ADM_xvid4.cpp
        ADM_ffmpeg_vdpau.cpp
)

ADD_ADM_LIB_ALL_TARGETS(ADM_codecs ${ADM_codecs_SRCS})

IF (USE_PNG)
	 ADD_SOURCE_CFLAGS(ADM_png.cpp ${PNG_CFLAGS})
ENDIF (USE_PNG)

IF (USE_XVID_4)
	ADD_SOURCE_CFLAGS(ADM_xvid4.cpp -I${XVID_INCLUDE_DIR})
ENDIF (USE_XVID_4)

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)
