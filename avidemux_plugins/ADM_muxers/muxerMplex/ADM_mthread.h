/***************************************************************************
                         Fake encoder used for copy mode

        We have to reorder !
        TODO FIXME

    begin                : Sun Jul 14 2002
    copyright            : (C) 2002/2003 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_MTHREADS_H
#define ADM_MTHREADS_H
typedef  void * (*THRINP)(void *p);

extern admMutex accessMutex;

typedef struct 
{
  Encoder                   *videoEncoder;
  AVDMGenericAudioStream    *audioEncoder;
  mplexMuxer                *muxer;
  ADMBitstream              *bitstream;
  uint32_t                  nbVideoFrame;
  uint32_t                  audioTargetSample;
  uint8_t                   *audioBuffer;
  uint32_t                  audioDone;
  uint32_t                  videoDone;
  uint32_t                  currentVideoFrame;
  uint32_t                  feedAudio;
  uint32_t                  feedVideo;
  uint32_t                  audioAbort;
  uint32_t                  videoAbort;
  void                      *opaque;
}muxerMT;

extern int defaultAudioSlave( muxerMT *context );
extern int defaultVideoSlave( muxerMT *context );


#endif
