SET(ADM_encoder_SRCS 
	adm_encConfig.cpp  adm_encffmpeg1.cpp  adm_encmjpeg.cpp     adm_encoder.cpp   adm_encxvid.cpp
	adm_encCopy.cpp    adm_encdivx.cpp    adm_encffmpeg.cpp   adm_encmpeg2enc.cpp  adm_encRequant.cpp  adm_encXvid4.cpp  adm_encyv12.cpp
	ADM_pluginLoad.cpp ADM_externalEncoder.cpp)

ADD_LIBRARY(ADM_encoder STATIC ${ADM_encoder_SRCS})

IF (USE_XVID_4)
	ADD_SOURCE_CFLAGS(adm_encxvid.cpp -I${XVID_INCLUDE_DIR})
	ADD_SOURCE_CFLAGS(adm_encXvid4.cpp -I${XVID_INCLUDE_DIR})
ENDIF (USE_XVID_4)
