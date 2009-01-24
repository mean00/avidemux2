
#ifndef CODECA_LIST
#define CODECA_LIST
#if 0
typedef struct {
        const char *name;
        const char *menuName;
        AUDIOENCODER codec;
        uint32_t     wavTag;
}CODECLIST;

 const CODECLIST myCodecList[]=
{
                {"copy","Copy", AUDIOENC_COPY,WAV_PCM},
#ifdef HAVE_LIBMP3LAME
                {"lame","MP3 (LAME)", AUDIOENC_MP3,WAV_MP3},
#endif
#ifdef USE_FAAC
                {"aac","AAC (FAAC)", AUDIOENC_FAAC,WAV_AAC},
#endif
#ifdef USE_VORBIS
                {"vorbis","Vorbis", AUDIOENC_VORBIS,WAV_OGG},
#endif
#ifdef USE_AFTEN
                {"aften","AC3 (Aften)", AUDIOENC_AFTEN,WAV_AC3},
#endif
                {"twolame","MP2 (TwoLAME)", AUDIOENC_2LAME,WAV_MP2},
                {"mp2", "MP2 (lavc)", AUDIOENC_MP2,WAV_MP2},
                {"ac3", "AC3 (lavc)",AUDIOENC_AC3,WAV_AC3},
                {"none", "WAV PCM",AUDIOENC_NONE,WAV_PCM},
                {"lpcm", "WAV LPCM",AUDIOENC_LPCM,WAV_LPCM}
};
#endif
#endif
