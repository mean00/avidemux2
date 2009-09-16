/**
    PlaceHolder
*/
#include "ADM_default.h"
#include "ADM_audioCodecEnum.h"

/**
        \fn getStrFromAudioCodec
        \brief Return a plain string from the codec_id
*/
const char *getStrFromAudioCodec( uint32_t codec)
{
      switch(codec)
      {
              case WAV_DTS: return QT_TR_NOOP("DTS");
              case WAV_PCM: return QT_TR_NOOP("PCM");
              case WAV_MP2: return QT_TR_NOOP("MP2");
              case WAV_MP3: return QT_TR_NOOP("MP3");
              case WAV_WMA:  return QT_TR_NOOP("WMA");
              case WAV_LPCM: return QT_TR_NOOP("LPCM");
              case WAV_AC3:  return QT_TR_NOOP("AC3");
              case WAV_EAC3:  return QT_TR_NOOP("E-AC3");
              case WAV_OGG_VORBIS: return QT_TR_NOOP("Ogg Vorbis");
              case WAV_MP4: return QT_TR_NOOP("MP4");
              case WAV_AAC: return QT_TR_NOOP("AAC");
              case WAV_QDM2: return QT_TR_NOOP("QDM2");
              case WAV_AMRNB: return QT_TR_NOOP("AMR-NB");
              case WAV_MSADPCM: return QT_TR_NOOP("MSADPCM");
              case WAV_ULAW: return QT_TR_NOOP("ULAW");
              case WAV_IMAADPCM: return QT_TR_NOOP("IMA ADPCM");
              case WAV_8BITS_UNSIGNED:return QT_TR_NOOP("8-bit PCM");
      }
      return QT_TR_NOOP("Unknown codec");
}
