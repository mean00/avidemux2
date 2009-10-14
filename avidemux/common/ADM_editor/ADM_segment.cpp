/***************************************************************************
     \file  ADM_segment.h
     \brief Handle segment

    (C) 2002-2009 Mean, fixounet@free.Fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_segment.h"
 #include "../ADM_codecs/ADM_codec.h"
 #include "ADM_image.h"
 #include "../ADM_editor/ADM_edCache.h"
 #include "ADM_pp.h"
 #include "ADM_colorspace.h"

#include "ADM_audiocodec/ADM_audiocodec.h"

ADM_EditorSegment::ADM_EditorSegment(void)
{

}
ADM_EditorSegment::~ADM_EditorSegment()
{
    deleteAll();
}
/**
    \fn addReferenceVideo
    \brief Add a new source video, fill in the missing info + create automatically the matching seg
*/
bool        ADM_EditorSegment::addReferenceVideo(_VIDEOS *ref)
{
  uint32_t    	l;
  uint8_t 	    *d;
  aviInfo       info;
  _SEGMENT      seg;

  memset(&seg,0,sizeof(seg));

  ref->_aviheader->getVideoInfo (&info);
  ref->_aviheader->getExtraHeaderData (&l, &d);
  ref->decoder = getDecoder (info.fcc,  info.width, info.height, l, d,info.bpp);
  ref->_videoCache   =   new EditorCache(32,info.width,info.height) ;

  float frameD=info.fps1000;
  frameD=frameD/1000;
  frameD=1/frameD;
  frameD*=1000000;
  ref->timeIncrementInUs=(uint64_t)frameD;
  ADM_info("[Editor] About %"LLU" microseconds per frame\n",ref->timeIncrementInUs);
  ref->_nb_video_frames = info.nb_frames;
  //
  //  And automatically create the segment
  //
  seg._reference=videos.size();
  if(!videos.size())
  {
        seg._startTimeUs=0;
  }else
  {
      #warning todo compute cumulative time
   }
   seg._durationUs=ref->_aviheader->getVideoDuration();

    // Update frametype if needed...
    _VIDEOS 	*vid=ref;
    decoders *decoder=vid->decoder;
//    rederiveFrameType(_videos[_nb_video]._aviheader);

    segments.push_back(seg);
    videos.push_back(*ref);
    updateStartTime();
    return true;
}


/**
	\fn deleteAll
    \brief delete datas associated with all video
*/
bool ADM_EditorSegment::deleteAll (void)
{
  ADM_info("[Editor] Deleting all video\n");
  int n=videos.size();
  for (uint32_t vid = 0; vid < n; vid++)
    {
        _VIDEOS *v=&(videos[vid]);
      // if there is a video decoder...
      if (v->decoder)
            delete v->decoder;
      if(v->color)
            delete v->color;
      v->_aviheader->close ();
      delete v->_aviheader;
      if(v->_videoCache)
      	delete  v->_videoCache;
      v->_videoCache=NULL;
      v->color=NULL;
      v->decoder=NULL;
      v->_aviheader=NULL;
     // Delete audio codec too
     // audioStream will be deleted by the demuxer
      if(v->audioTracks)
      {
            for(int i=0;i<v->nbAudioStream;i++)
            {
                delete v->audioTracks[i];
            }
            delete [] v->audioTracks;
            v->audioTracks=NULL;
      }
    }

    videos.clear();
    return true;
}


/**
    \fn resetSegment
    \brief Redo a 1:1 mapping between videos and segments
*/
bool        ADM_EditorSegment::resetSegment(void)
{
    //
    
    segments.clear();
    int n=videos.size();
    for(int i=0;i<n;i++)
    {
        _SEGMENT seg;
        _VIDEOS  *vid=&(videos[i]);
        memset(&seg,0,sizeof(seg));
        seg._durationUs=vid->_aviheader->getVideoDuration();
        seg._reference=i;
        segments.push_back(seg);
    }
    updateStartTime();
    return true;
}
/**
    \fn getSegment
    \brief getRefVideo
*/
_SEGMENT     *ADM_EditorSegment::getSegment(int i)
{
    ADM_assert(i<segments.size());
    return &(segments[i]);
}
/**
    \fn getRefVideo
    \brief getRefVideo
*/
_VIDEOS     *ADM_EditorSegment::getRefVideo(int i)
{
    ADM_assert(i<videos.size());
    return &(videos[i]);
}
/**
    \fn getNbRefVideo
    \brief getNbRefVideo
*/
int         ADM_EditorSegment::getNbRefVideos(void)
{
    return videos.size();
}
/**
    \fn getNbSegment
    \brief  getNbSegment
*/
int         ADM_EditorSegment::getNbSegments(void)
{   
    return segments.size();
}
/**
    \fn updateStartTime
    \brief Recompute the start time of the video 
*/
bool         ADM_EditorSegment::updateStartTime(void)
{   
    int n=segments.size();
    uint64_t t=0;
    for(int i=0;i<n;i++)
    {
        segments[i]._startTimeUs=t;
        t+=segments[i]._durationUs;
    }
    return true;
}
/**
    \fn getTotalDuration
    \brief 
*/
uint64_t ADM_EditorSegment::getTotalDuration(void)
{
    uint64_t dur=0;
    int n=segments.size();
    for(int i=0;i<n;i++)
        dur+=segments[i]._durationUs;
    return dur;

}
/**
    \fn getNbFrames
    \brief 
*/
uint32_t ADM_EditorSegment::getNbFrames(void)
{
    uint32_t dur;
    uint32_t nb=0;
    int n=videos.size();
    for(int i=0;i<n;i++)
        dur+=videos[i]._nb_video_frames;
    return nb;

}
/**
    \fn getTotalDuration
*/
bool        ADM_EditorSegment::getRefFromTime(uint64_t time,uint32_t *refVideo, uint64_t *offset)
{
    *offset=time;
    *refVideo=0;
    return true;
}
/**
    \fn getRefFromFrame
*/
bool        ADM_EditorSegment::getRefFromFrame(uint32_t frame,uint32_t *refVideo, uint32_t *frameOffset)
{
    *refVideo=0;
    *frameOffset=frame;
    return true;
}
/**
    \fn getFrameFromRef
*/
bool        ADM_EditorSegment::getFrameFromRef(uint32_t *frame,uint32_t refVideo, uint32_t frameOffset)
{
    *frame=frameOffset;
    return true;

}
/**
    \fn ~ADM_audioStreamTrack
*/
 ADM_audioStreamTrack::~ADM_audioStreamTrack()
    {
        stream=NULL;
        info=NULL;   // These 2 are destroyed by the demuxer itself
        if(codec) 
        {
            delete codec;
            codec=NULL;
        }
    }
/**
    \fn getCurrentSeg
*/
_SEGMENT   * ADM_EditorSegment::getCurrentSeg(void)
{
    return &(segments[0]);
}

bool        ADM_EditorSegment::convertLinearTimeToSeg(  uint64_t frameTime, uint32_t *seg, uint64_t *segTime)
{
    *seg=0;
    *segTime=frameTime;
    return true;
}
bool        ADM_EditorSegment::convertSegTimeToLinear(  uint32_t seg,uint64_t segTime, uint64_t *frameTime)
{
    *frameTime=segTime;
    return true;
}
//EOF
