/***************************************************************************
                        3nd gen indexer                                                 
                             
    
    copyright            : (C) 2005 by mean
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
#ifndef DMX_INDEXER_INTERNAL_H
#define DMX_INDEXER_INTERNAL_H

#define MAX_PUSHED 200


typedef struct TimeStamp
{
        uint16_t hh,mm,ss,ff;
}TimeStamp;


typedef struct IndFrame
{
	uint32_t type;
	uint32_t size;
	uint64_t abs;
	uint64_t rel;
	
}IndFrame;
/*****************************************/
typedef struct dmx_runData
{
  
      FILE     *fd;
      dmx_demuxer *demuxer;
      DIA_progressIndexing *work;

      uint64_t totalFileSize; 
      uint32_t totalFrame;
      uint32_t nbImage;
      uint32_t nbPushed,nbGop;      
      uint32_t nbTrack;
      uint32_t imageW,imageH,imageFPS,imageAR;
      TimeStamp firstStamp,lastStamp; /* Time code hh:mm:ss */
      
}dmx_runData;

/*********************************************************************************************/
/* Base Class for indexer , depending of the payload type, new derivated classes are created */
/*********************************************************************************************/
class dmx_videoIndexer
{
protected:
   FILE                 *_fd;
   uint64_t             _totalFileSize; 
   uint32_t             _nbTrack;
   dmx_demuxer          *_demuxer;
   DIA_progressIndexing *_work;
   IndFrame             *_frames;
   dmx_runData          *_run;
   
public:
          dmx_videoIndexer(dmx_runData *run)
          {
              _run=run;
          }
  virtual uint8_t  run(void)=0;
  virtual void     cleanup(void)=0;
  virtual           ~dmx_videoIndexer() 
              {
                  if(_frames)
                  {
                    delete [] _frames;
                    _frames=NULL; 
                  }
              }
};
/**************************************************/
/* Class to index mpeg2 payload in ES/PS/TS/MSDVR */
/**************************************************/
class dmx_videoIndexerMpeg2 :public dmx_videoIndexer
{
protected:
          uint64_t firstPicPTS;
          uint32_t grabbing;
          uint32_t seq_found;
          
          
          uint8_t gopDump(uint64_t abs,uint64_t rel);
          uint8_t gopUpdate(void);
          uint8_t dumpPts(uint64_t firstPts);
          uint8_t Push(uint32_t ftype,uint64_t abs,uint64_t rel);
public:
                  dmx_videoIndexerMpeg2(dmx_runData *run);
 
          uint8_t run(void);
          void    cleanup(void);
  virtual         ~dmx_videoIndexerMpeg2();
             
};
/**************************************************/
/* Class to index H264 payload in ES/PS/TS/MSDVR */
/**************************************************/
class dmx_videoIndexerH264 :public dmx_videoIndexer
{
protected:
          uint64_t firstPicPTS;
          uint32_t grabbing;
          uint32_t seq_found;
          
          
          uint8_t gopDump(uint64_t abs,uint64_t rel);
          uint8_t gopUpdate(void);
          uint8_t dumpPts(uint64_t firstPts);
          uint8_t startFrame(uint32_t ftype,uint64_t abs,uint64_t rel);
          uint8_t updateFrameType(uint32_t ftypel);
public:
                  dmx_videoIndexerH264(dmx_runData *run);
 
          uint8_t run(void);
          void    cleanup(void);
  virtual         ~dmx_videoIndexerH264();
             
};

/********************************************************/

#endif
//EOF


