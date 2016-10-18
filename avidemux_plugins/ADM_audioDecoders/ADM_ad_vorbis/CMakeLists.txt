	
# Vorbis decoder
INCLUDE(admCheckVorbisDec)
checkVorbis()
IF(USE_LIBVORBIS)
        INCLUDE(ad_plugin)

        SET(ADM_ad_vorbis_SRCS ADM_ad_vorbis.cpp)

	ADD_AUDIO_DECODER(ADM_ad_vorbis ${ADM_ad_vorbis_SRCS})

        TARGET_LINK_LIBRARIES(ADM_ad_vorbis ${LIBVORBIS_LIBRARY_DIR} ADM_coreUtils6 ADM_coreAudio6)

        include_directories(${LIBVORBIS_INCLUDE_DIR})

        INIT_AUDIO_PLUGIN(ADM_ad_vorbis)
        INSTALL_AUDIODECODER(ADM_ad_vorbis)
ENDIF(USE_LIBVORBIS)
