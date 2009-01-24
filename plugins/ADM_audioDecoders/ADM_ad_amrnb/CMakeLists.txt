# AMRNB decoder
INCLUDE(admCheckAmrnb)
checkAmrnb()

IF (AMRNB_FOUND)
	INCLUDE(ad_plugin)
	
	SET(ADM_ad_amrnb_SRCS ADM_ad_amrnb.cpp)

	ADD_LIBRARY(ADM_ad_amrnb SHARED ${ADM_ad_amrnb_SRCS})
	TARGET_LINK_LIBRARIES(ADM_ad_amrnb ADM_core ${AMRNB_LIBRARY_DIR})

	ADD_TARGET_CFLAGS(ADM_ad_amrnb "-I${AMRNB_INCLUDE_DIR}")

	INIT_AUDIO_PLUGIN(ADM_ad_amrnb)
	INSTALL_AUDIODECODER(ADM_ad_amrnb)
ENDIF (AMRNB_FOUND)
