#define Join(x,y) x##_##y
#define AUDMEncoder_Lavcodec AUDMEncoder_Lavcodec_AC3
#define makeName(x) Join(x,AC3)
#define avMakeName          AV_CODEC_ID_AC3
#define ADM_LAV_VERSION     1,0,0
#define ADM_LAV_NAME        "LavAC3" 
#define ADM_LAV_MENU        "AC3 (lav)" 
#define ADM_LAV_DESC        "AC3 LavCodec encoder plugin Mean 2008/2009"
#define ADM_LAV_MAX_CHANNEL 6
#define ADM_LAV_SAMPLE_PER_P 1536

#define MENU_BITRATE     diaMenuEntry bitrateM[]={\
                              BITRATE(56),\
                              BITRATE(64),\
                              BITRATE(80),\
                              BITRATE(96),\
                              BITRATE(112),\
                              BITRATE(128),\
                              BITRATE(160),\
                              BITRATE(192),\
                              BITRATE(224),\
                              BITRATE(384),\
                              BITRATE(448),\
                              BITRATE(640)\
                          };

#include "audioencoder_lavcodec.cpp"
