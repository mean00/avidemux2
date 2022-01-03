INCLUDE(ad_plugin)
SET(ADM_ad_alaw_SRCS ADM_ad_alaw.cpp)

ADD_AUDIO_DECODER(ADM_ad_alaw ${ADM_ad_alaw_SRCS})
INIT_AUDIO_PLUGIN(ADM_ad_alaw)
INSTALL_AUDIODECODER(ADM_ad_alaw)
