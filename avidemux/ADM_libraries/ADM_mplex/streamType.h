#ifndef STREAM_TYPE_H
#define STREAM_TYPE_H
enum StreamKind
  {
      MPEG_AUDIO=1,
      AC3_AUDIO,
      LPCM_AUDIO,
      DTS_AUDIO,
      MPEG_VIDEO
#ifdef ZALPHA
        ,
      Z_ALPHA
#endif
  };

typedef struct mplexStreamDescriptor
{
  int channel;
  int frequency;
  StreamKind kind;
}mplexStreamDescriptor;
#endif
