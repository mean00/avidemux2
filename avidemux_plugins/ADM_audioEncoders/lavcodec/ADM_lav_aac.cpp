#define Join(x,y) x##_##y
#define AUDMEncoder_Lavcodec AUDMEncoder_Lavcodec_AAC
#define makeName(x) Join(x,AAC)
#define avMakeName          AV_CODEC_ID_AAC
#define ADM_LAV_VERSION     1,0,0
#define ADM_LAV_NAME        "LavAAC" 
#define ADM_LAV_MENU        "AAC (lav)" 
#define ADM_LAV_DESC        "AAC LavCodec encoder plugin Mean 2008/2016"
#define ADM_LAV_MAX_CHANNEL 6
#define ADM_LAV_SAMPLE_PER_P 1024

#define ADM_LAV_GLOBAL_HEADER 1

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
