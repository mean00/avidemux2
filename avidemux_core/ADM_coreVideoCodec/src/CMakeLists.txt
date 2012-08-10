SET(ADM_codecs_SRCS 
ADM_codecs.cpp
ADM_ffmp43.cpp
ADM_codecFFsimple.cpp
ADM_rgb16.cpp
ADM_uyvy.cpp
ADM_codecSearch.cpp
DIA_lavDecoder.cpp
)

add_compiler_export_flags()
ADM_ADD_SHARED_LIBRARY(ADM_coreVideoCodec6 ${ADM_codecs_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreVideoCodec6 
                                          ADM_core6
                                          ADM_coreImage6 
                                          ADM_coreUI6
                                          ADM_coreUtils6
                                          ADM_libavcodec
                                          ADM_libavformat
                                          ADM_libavutil)

if (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)
	add_dependencies(ADM_coreVideoCodec6 libavcodec libavformat libavutil)
endif (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)

ADM_INSTALL_LIB(ADM_coreVideoCodec6)

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)
