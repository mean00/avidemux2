#define Join(x,y) x##_##y
#define AUDMEncoder_Lavcodec AUDMEncoder_Lavcodec_MP2
#define makeName(x) Join(x,MP2)
#define avMakeName          AV_CODEC_ID_MP2
#define ADM_LAV_VERSION     1,0,0
#define ADM_LAV_NAME        "LavMP2" 
#define ADM_LAV_MENU        "MP2 (lav)" 
#define ADM_LAV_DESC        "MP2 LavCodec encoder plugin Mean 2008/2009"
#define ADM_LAV_MAX_CHANNEL 6
#define ADM_LAV_SAMPLE_PER_P 1152

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
                              BITRATE(384)\
                          };

#include "audioencoder_lavcodec.cpp"
