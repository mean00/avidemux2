	
# Vorbis decoder
INCLUDE(admCheckVorbisDec)
checkVorbis()
IF(USE_LIBVORBIS)
        INCLUDE(ad_plugin)

        SET(ADM_ad_vorbis_SRCS ADM_ad_vorbis.cpp)

        ADD_LIBRARY(ADM_ad_vorbis SHARED ${ADM_ad_vorbis_SRCS})

        TARGET_LINK_LIBRARIES(ADM_ad_vorbis ${VORBIS_LIBRARY_DIR})
        TARGET_LINK_LIBRARIES(ADM_ad_vorbis ${VORBISENC_LIBRARY_DIR})

        include_directories(${VORBIS_INCLUDE_DIR})

        INIT_AUDIO_PLUGIN(ADM_ad_vorbis)
        INSTALL_AUDIODECODER(ADM_ad_vorbis)
ENDIF(USE_LIBVORBIS)
