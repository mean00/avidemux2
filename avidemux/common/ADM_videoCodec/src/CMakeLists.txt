MACRO(ADD_IF_ENABLED x)
                SET(ADM_videoCodec_SRCS ${ADM_videoCodec_SRCS} ${ARGN})
ENDMACRO(ADD_IF_ENABLED x )

SET(ADM_videoCodec_SRCS 
        ADM_videoCodec.cpp
        ADM_dynVideoDecoder.cpp)

IF(USE_VDPAU)
        ADD_CORE_INCLUDE(ADM_coreVdpau)
        ADD_IF_ENABLED(USE_VDPAU ADM_ffmpeg_vdpau.cpp)
ENDIF(USE_VDPAU)
                
IF(USE_LIBVA)
        ADD_DEFINITIONS(${LIBVA_CFLAGS})
        INCLUDE_DIRECTORIES(${LIBVA_INCLUDE_DIR})	
        ADD_CORE_INCLUDE(ADM_coreLibVA)
        ADD_IF_ENABLED(USE_LIBVA ADM_ffmpeg_libva.cpp)
ENDIF(USE_LIBVA)
IF(USE_DXVA2)
        ADD_DEFINITIONS(${LIBVA_CFLAGS})
        ADD_CORE_INCLUDE(ADM_coreDxva2)
        ADD_IF_ENABLED(USE_LIBVA ADM_ffmpeg_dxva2.cpp)
ENDIF(USE_DXVA2)
                 
ADD_LIBRARY(ADM_videocodec6 STATIC ${ADM_videoCodec_SRCS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
