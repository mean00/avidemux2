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


