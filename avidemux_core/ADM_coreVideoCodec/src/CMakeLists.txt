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
TARGET_LINK_LIBRARIES(ADM_coreVideoCodec6 
                                          ADM_core6
                                          ADM_coreImage6 
                                          ADM_coreUI6
                                          ADM_coreUtils6
                                          ADM_libavcodec6
                                          ADM_libavformat6
                                          ADM_libavutil6
)
ADM_INSTALL_LIB(ADM_coreVideoCodec6)

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)
