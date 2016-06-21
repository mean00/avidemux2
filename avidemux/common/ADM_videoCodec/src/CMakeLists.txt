MACRO(ADD_IF_ENABLED x)
                SET(ADM_videoCodec_SRCS ${ADM_videoCodec_SRCS} ${ARGN})
ENDMACRO(ADD_IF_ENABLED x )

SET(ADM_videoCodec_SRCS 
        ADM_videoCodec.cpp
        ADM_dynVideoDecoder.cpp)

IF(USE_VDPAU)
        ADD_IF_ENABLED(USE_VDPAU ADM_ffmpeg_vdpau.cpp)
ENDIF(USE_VDPAU)
                
IF(USE_LIBVA)
        ADD_IF_ENABLED(USE_LIBVA ADM_ffmpeg_libva.cpp)
ENDIF(USE_LIBVA)
                
ADD_LIBRARY(ADM_videocodec6 STATIC ${ADM_videoCodec_SRCS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
