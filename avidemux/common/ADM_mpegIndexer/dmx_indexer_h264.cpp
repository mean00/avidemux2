/***************************************************************************
                        2nd gen indexer                                                 
                             
    
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
#include "config.h"

#include "ADM_default.h"
#include "ADM_assert.h"

#include "DIA_fileSel.h"
#include "ADM_quota.h"
#include "ADM_commonUI/DIA_idx_pg.h"



#include "ADM_debugID.h"
#define MODULE_NAME MODULE_MPEG
#include "ADM_debug.h"
#include "dmx_demuxerEs.h"
#include "dmx_demuxerPS.h"
#include "dmx_demuxerTS.h"
#include "dmx_demuxerMSDVR.h"
#include "dmx_identify.h"

#define MIN_DELTA_PTS 150 // autofix in ms

#include "dmx_indexer_internal.h"

#include "ADM_h264_tag.h"
#undef aprintf
#define aprintf printf
static const char Type[5]={'X','I','P','B','P'};


extern uint8_t extractSPSInfo(uint8_t *data, uint32_t len,uint32_t *wwidth,uint32_t *hheight, uint32_t *fps1000);

dmx_videoIndexerH264::dmx_videoIndexerH264(dmx_runData *run) : dmx_videoIndexer(run)
{
  _frames=new IndFrame[MAX_PUSHED];
  firstPicPTS=ADM_NO_PTS;
  seq_found=0;
  grabbing=0;
  
}
dmx_videoIndexerH264::~dmx_videoIndexerH264()
{
          if(_frames) delete [] _frames;
          _frames=NULL;
}
/**
      \fn cleanup
      \brief do cleanup and purge non processed data at the end of the mpeg2 stream
*/
void dmx_videoIndexerH264::cleanup(void)
{
  uint64_t lastAbs,lastRel;
          _run->demuxer->getPos(&lastAbs,&lastRel);
          if(_run->nbPushed)  gopDump(lastAbs,lastRel);
          dumpPts(firstPicPTS); 
          
}

/**
      \fn run 
      \brief main indexing loop for mpeg2 payload
*/


// 12 : Filler
uint8_t   dmx_videoIndexerH264::run  (void)
{
dmx_demuxer *demuxer=_run->demuxer;
uint64_t syncAbs,syncRel;
uint8_t streamid;   
uint32_t temporal_ref,ftype,val;
uint64_t pts,dts;
uint8_t pic_started=0;
      
      
#if 0
    FILE *f=fopen("/tmp/foo.h264","wt");
    uint8_t buffer[80];
    ADM_assert(f);
    for(int i=0;i<1024*1024*2;i++)
    {
       _run->demuxer->read(buffer,60);
        fwrite(buffer,60,1,f);
    }
    fclose(f);
    exit(0);  

#endif
      
      printf("Starting H264 indexer\n");
      while(1)
      {

              if(!demuxer->syncH264(&streamid,&syncAbs,&syncRel,&pts,&dts)) return 0;
              streamid=streamid & 0x1f;
              //if(streamid>31) continue;
             // printf("Found NAL : %d 0x %x\n",streamid,streamid);
#define T(x) case NAL_##x: printf(#x" found\n");break;
#if 1
                switch(streamid)
                {
                    T(NON_IDR);
                    T(IDR);
                    T(SPS);
                    T(PPS);
                    T(SEI);
                    T(AU_DELIMITER);
                    T(FILLER);
                  default :printf("%02x ?\n",streamid);
                }
#endif
                 if((_run->totalFileSize>>16)>50)
              {
                    _run->work->update(syncAbs>>16,_run->totalFileSize>>16,
                               _run->nbImage,_run->lastStamp.hh,_run->lastStamp.mm,_run->lastStamp.ss);
              }else
              {
                    _run->work->update(syncAbs,_run->totalFileSize,_run->nbImage,
                            _run->lastStamp.hh,_run->lastStamp.mm,_run->lastStamp.ss);
              }
              /* Till we have a SPS no need to continue */
              if(!seq_found && streamid!=NAL_SPS) continue;
              if(!seq_found)
              {
                    // Our firt frame is here
                    // Important to initialize the mpeg decoder !
                    _run->imageAR = 1;	// 1:1 to suppress warning

                      uint8_t buffer[60] ; // should be enough
                      uint64_t xA,xR;
                      _run->demuxer->getPos(&xA,&xR);
                      _run->demuxer->read(buffer,60);
                      if(extractSPSInfo(buffer,60,&( _run->imageW),&( _run->imageH),&(_run->imageFPS)))
                      {
                            seq_found=1;
                            startFrame(1,syncAbs,syncRel);
                           _run->demuxer->setPos(xA,xR);
                           _run->demuxer->stamp();
                      }else
                        _run->demuxer->setPos(xA,xR);
                      
                      grabbing=1;
                      continue;
              }
              
              // Ignore multiple chunk of the same pic
              
              if((streamid==NAL_NON_IDR || streamid==NAL_IDR)&&pic_started) 
              {
                aprintf("Still capturing, ignore\n");
                continue;
              }
              if(_run->work->isAborted()) return 0;
              
              switch(streamid)
                      {
                      case NAL_AU_DELIMITER:
                              if(pic_started)
                                  pic_started=0;
                              grabbing=0;
                              break;
                      case NAL_SPS: // 
                              pic_started=0;
                              grabbing=1;
                              aprintf("Sps %d\n",_run->nbGop);
                              gopDump(syncAbs,syncRel);
                              break;
                      case NAL_IDR: // IDR
                              aprintf("IDR %d\n",_run->nbGop);
                              uint32_t gop;   
                              if(grabbing) 
                              {
                                aprintf("Grabbing, updating type\n");
                                updateFrameType(1);
                                pic_started=1;
                                grabbing=0;
                                continue;
                              }
                              aprintf("Dumping gop-\n");
                              gopDump(syncAbs,syncRel);
                              aprintf("New Frame-\n");
                              updateFrameType(1);
                              if(firstPicPTS==ADM_NO_PTS && pts!=ADM_NO_PTS)
                                      firstPicPTS=pts;
                              _run->totalFrame++;
                              pic_started=1;
                              break;
                      case NAL_NON_IDR : // picture
                              
                              if(grabbing) 
                              {
                                updateFrameType(2);
                                pic_started=1;
                                grabbing=0;
                                aprintf("Grabbing, updating type-2-\n");
                                continue;
                              }
                              _run->totalFrame++;
                              if(_run->nbPushed>MAX_PUSHED/2)
                              {
                                aprintf("Gop too big, dumping-\n");
                                gopDump(syncAbs,syncRel);
                                updateFrameType(2); 
                              }else
                              {
                                aprintf("Staring new frame-\n");
                                startFrame(2,syncAbs,syncRel);
                              }
                              pic_started=1;
                              break;
                      default:
                        break;
                      }
                
      }
    _frames[0].type=1; /* Always starts with an infra */
    return 1; 
}




/**
      \fn Push
      \brief Add a frame to the current gop

*/
uint8_t dmx_videoIndexerH264::startFrame(uint32_t ftype,uint64_t abs,uint64_t rel)
{
                                            
        _frames[_run->nbPushed].type=ftype;
        
        if(_run->nbPushed) // Update previous if it exists
        {
                aprintf("Start frame : empty\n");
                _frames[_run->nbPushed-1].size=_run->demuxer->elapsed();
        
        }else
        {
         aprintf("Start frame %u  in gop\n",_run->nbPushed); 
        }
        _frames[_run->nbPushed].abs=abs;
        _frames[_run->nbPushed].rel=rel;        
        _run->demuxer->stamp();
        _run->nbPushed++;
        
        ADM_assert(_run->nbPushed<MAX_PUSHED);
        return 1;

}
/**
    \fn updateFrameType
    \brief update the current frame type.Needed as we start from SPS if present
*/
uint8_t dmx_videoIndexerH264::updateFrameType(uint32_t ftype)
{
  ADM_assert(_run->nbPushed);
  _frames[_run->nbPushed-1].type=ftype;
  aprintf("updateFrameType %u for frame %u -1 in gop\n",ftype,_run->nbPushed); 
  return 1;
}
/**
    \fn dumpPts
    \brief

*/
uint8_t dmx_videoIndexerH264::dumpPts(uint64_t firstPts)
{
uint64_t stats[_run->nbTrack],p;
double d;
dmx_demuxer *demuxer=_run->demuxer;

        if(!demuxer->getAllPTS(stats)) return 0;
        qfprintf(_run->fd,"# Video 1st PTS : %07u\n",firstPts);
        if(firstPts==ADM_NO_PTS) return 1;
        for(int i=1;i<_run->nbTrack;i++)
        {
                p=stats[i];
                if(p==ADM_NO_PTS)
                {
                        qfprintf(_run->fd,"# track %d no pts\n",i);
                }
                else
                {
                        
                        d=firstPts; // it is in 90 khz tick
                        d-=stats[i];
                        d/=90.;
                        qfprintf(_run->fd,"# track %d PTS : %07u ",i,stats[i]);
                        qfprintf(_run->fd," delta=%04d ms\n",(int)d);
                }

        }
        return 1;
}
/**
      \fn       gopDump
      \brief    Dump the content of a gop into the index file
*/
uint8_t dmx_videoIndexerH264::gopDump(uint64_t abs,uint64_t rel)
{
  dmx_demuxer *demuxer=_run->demuxer;
  
        /* No frame */
        if(!_run->nbPushed) return 1;

uint64_t stats[_run->nbTrack];

        _frames[_run->nbPushed-1].size=demuxer->elapsed();    // Update previous frame
        qfprintf(_run->fd,"V %03u %06u %02u ",_run->nbGop,_run->nbImage,_run->nbPushed);

        /* First frame must be IDR */
        if(!_run->nbGop) _frames[0].type=1;
        
        for(uint32_t i=0;i<_run->nbPushed;i++) 
        {

                qfprintf(_run->fd,"%c:%08"LLX",%05x",
                        Type[_frames[i].type],
                        _frames[i].abs,
                        _frames[i].rel);
                qfprintf(_run->fd,",%05x ",
                        _frames[i].size);
        }
        
        qfprintf(_run->fd,"\n");

        // audio if any
        //*******************************************
        // Nb image abs_pos audio seen
        // The Nb image is used to compute a drift
        //*******************************************
        if(demuxer->hasAudio() & _run->nbTrack>1)
        {
                demuxer->getStats(stats);
                
                qfprintf(_run->fd,"A %u %"LLX" ",_run->nbImage,abs);
                for(uint32_t i=1;i<_run->nbTrack;i++)
                {
                        qfprintf(_run->fd,":%"LLX" ",stats[i]);
                }
                qfprintf(_run->fd,"\n");
                
        }
        // Print some gop stamp infos, does not hurt
        qfprintf(_run->fd,"# Timestamp %02d:%02d:%02d,%03d\n",
                 _run->lastStamp.hh,_run->lastStamp.mm,_run->lastStamp.ss,_run->lastStamp.ff);
        _run->nbGop++;
        _run->nbImage+=_run->nbPushed;
        _run->nbPushed=0;
        startFrame(2,abs,rel);
        return 1;
        
}
/**
      \fn gopUpdate( 
      \brief Update the timecode hh:mm:ss.xx inside a gop header
*/
uint8_t dmx_videoIndexerH264::gopUpdate(void)
{

        return 1;
}
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//
