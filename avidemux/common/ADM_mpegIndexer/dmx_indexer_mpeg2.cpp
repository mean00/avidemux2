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

static const char Type[5]={'X','I','P','B','P'};

static const uint32_t FPS[16]={
                0,                      // 0
                23976,          // 1 (23.976 fps) - FILM
                24000,          // 2 (24.000 fps)
                25000,          // 3 (25.000 fps) - PAL
                29970,          // 4 (29.970 fps) - NTSC
                30000,          // 5 (30.000 fps)
                50000,          // 6 (50.000 fps) - PAL noninterlaced
                59940,          // 7 (59.940 fps) - NTSC noninterlaced
                60000,          // 8 (60.000 fps)
                0,                      // 9
                0,                      // 10
                0,                      // 11
                0,                      // 12
                0,                      // 13
                0,                      // 14
                0                       // 15
        };


dmx_videoIndexerMpeg2::dmx_videoIndexerMpeg2(dmx_runData *run) : dmx_videoIndexer(run)
{
  _frames=new IndFrame[MAX_PUSHED];
  firstPicPTS=ADM_NO_PTS;
  seq_found=0;
  grabbing=0;
  
}
dmx_videoIndexerMpeg2::~dmx_videoIndexerMpeg2()
{
          if(_frames) delete [] _frames;
          _frames=NULL;
}
/**
      \fn cleanup
      \brief do cleanup and purge non processed data at the end of the mpeg2 stream
*/
void dmx_videoIndexerMpeg2::cleanup(void)
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
uint8_t   dmx_videoIndexerMpeg2::run  (void)
{
dmx_demuxer *demuxer=_run->demuxer;
uint64_t syncAbs,syncRel;
uint8_t streamid;   
uint32_t temporal_ref,ftype,val;
uint64_t pts,dts;

      

      while(1)
      {

              if(!demuxer->sync(&streamid,&syncAbs,&syncRel,&pts,&dts)) return 0;   
              if((_run->totalFileSize>>16)>50)
              {
                    _run->work->update(syncAbs>>16,_run->totalFileSize>>16,
                               _run->nbImage,_run->lastStamp.hh,_run->lastStamp.mm,_run->lastStamp.ss);
              }else
              {
                    _run->work->update(syncAbs,_run->totalFileSize,_run->nbImage,
                            _run->lastStamp.hh,_run->lastStamp.mm,_run->lastStamp.ss);
              }
              if(_run->work->isAborted()) return 0;
              switch(streamid)
                      {
                      case 0xB3: // sequence start
                              aprintf("Seq %d\n",_run->nbGop);
                              if(grabbing) continue;
                              grabbing=1;    
                              
                              gopDump(syncAbs,syncRel);
                              if(seq_found)
                              {
                                      demuxer->forward(8);  // Ignore
                                      continue;
                              }
                              // Our firt frame is here
                              // Important to initialize the mpeg decoder !
                              _frames[0].abs=syncAbs;
                              _frames[0].rel=syncRel;
                              //
                              seq_found=1;
                              val=demuxer->read32i();
                              _run->imageW=val>>20;
                              _run->imageW=((_run->imageW+15)&~15);
                              _run->imageH= (((val>>8) & 0xfff)+15)& ~15;
                              _run->imageAR=(val>>4)&0xf;
                              _run->imageFPS= FPS[val & 0xf];
                              demuxer->forward(4);
                              break;
                      case 0xb8: // GOP
                              aprintf("GOP %d\n",_run->nbGop);
#ifdef SHOW_PTS
                              if(pts!=ADM_NO_PTS)
                              {
                              qfprintf(run->fd,"# %lu\n",pts/90);
                              }
#endif
                              uint32_t gop;   
                              if(!seq_found) continue;
                              if(grabbing) 
                              {         
                                      gopUpdate();
                                      continue;;
                              }
                              gopDump(syncAbs,syncRel);
                              gopUpdate();
                              
                              break;
                      case 0x00 : // picture
                            
                              aprintf("pic \n");
                              if(!seq_found)
                              { 
                                      continue;
                                      printf("No sequence start yet, skipping..\n");
                              }
                              if(firstPicPTS==ADM_NO_PTS && pts!=ADM_NO_PTS)
                                      firstPicPTS=pts;
                              grabbing=0;
                              _run->totalFrame++;
                              val=demuxer->read16i();
                              temporal_ref=val>>6;
                              ftype=7 & (val>>3);
                              //aprintf("Temporal ref:%lu\n",temporal_ref);
                              // skip illegal values
                              if(ftype<1 || ftype>3)
                              {
                                      printf("[Indexer]Met illegal pic at %"LLX" + %"LLX"\n",
                                                      syncAbs,syncRel);
                                      continue;
                              }
#define USING dts
#if 0
                              if(USING!=ADM_NO_PTS)
                              {
                                      if(olddts!=ADM_NO_PTS )
                                      {                
                                              if(USING>=olddts)
                                              {
                                              uint64_t delta,deltaRef;
                                                      delta=USING-olddts;
                                                      
                                                      delta/=90;
                                                    //  printf("Delta :%llu ms\n",delta);
                                                      // compute what we should be using
                                                      deltaRef=nbPushed+nbImage-run->lastRefPic;
                                                      // in ms
                                                      deltaRef=(deltaRef*1000*1000)/run->imageFPS;
                                                      if(abs(delta-deltaRef)>MIN_DELTA_PTS)
                                                      {
                                                              printf("Discontinuity %llu %llu!\n",delta,deltaRef);
                                                      }
                                              } 
                                      }
                                      olddts=USING;
                                      run->lastRefPic=nbPushed+nbImage;
                              }
#endif
                              
                              Push(ftype,syncAbs,syncRel);
                          
                              break;
                      default:
                        break;
                      }
      }
                      return 1; 
}



/**** Push a frame
There is a +- 2 correction when we switch gop
as in the frame header we read 2 bytes
Between frames, the error is self cancelling.

**/
/**
      \fn Push
      \brief Add a frame to the current gop

*/
uint8_t dmx_videoIndexerMpeg2::Push(uint32_t ftype,uint64_t abs,uint64_t rel)
{
                                            
        _frames[_run->nbPushed].type=ftype;
        
        if(_run->nbPushed)
        {                
                _frames[_run->nbPushed-1].size=_run->demuxer->elapsed();
                if(_run->nbPushed==1) _frames[_run->nbPushed-1].size-=2;
                _frames[_run->nbPushed].abs=abs;
                _frames[_run->nbPushed].rel=rel;        
                _run->demuxer->stamp();
        
        }
        //aprintf("\tpushed %d %"LLX"\n",nbPushed,abs);
        _run->nbPushed++;
        
        ADM_assert(_run->nbPushed<MAX_PUSHED);
        return 1;

}
/**
    \fn dumpPts
    \brief

*/
uint8_t dmx_videoIndexerMpeg2::dumpPts(uint64_t firstPts)
{
uint64_t stats[_run->nbTrack],p;
double d;
dmx_demuxer *demuxer=_run->demuxer;

        if(!demuxer->getAllPTS(stats)) return 0;
        qfprintf(_run->fd,"# Video 1st PTS : %07"LLU"\n",firstPts);
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
                        qfprintf(_run->fd,"# track %d PTS : %07"LLU" ",i,stats[i]);
                        qfprintf(_run->fd," delta=%04d ms\n",(int)d);
                }

        }
        return 1;
}
/**
      \fn       gopDump
      \brief    Dump the content of a gop into the index file
*/
uint8_t dmx_videoIndexerMpeg2::gopDump(uint64_t abs,uint64_t rel)
{
  dmx_demuxer *demuxer=_run->demuxer;
        if(!_run->nbPushed && !_run->nbImage) demuxer->stamp();
        if(!_run->nbPushed) return 1;

uint64_t stats[_run->nbTrack];

        _frames[_run->nbPushed-1].size=demuxer->elapsed()+2;
        qfprintf(_run->fd,"V %03u %06u %02u ",_run->nbGop,_run->nbImage,_run->nbPushed);

        // For each picture Type : abs position : relat position : size
        for(uint32_t i=0;i<_run->nbPushed;i++) 
        {

                qfprintf(_run->fd,"%c:%08"LLX",%05"LLX,
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
                
        _frames[0].abs=abs;
        _frames[0].rel=rel;
        demuxer->stamp();
        return 1;
        
}
/**
      \fn gopUpdate( 
      \brief Update the timecode hh:mm:ss.xx inside a gop header
*/
uint8_t dmx_videoIndexerMpeg2::gopUpdate(void)
{
uint32_t a1,a2,a3,a4;
uint32_t hh,mm,ss,ff;
TimeStamp *ts;
dmx_demuxer *demuxer=_run->demuxer;

        a1=demuxer->read8i();
        a2=demuxer->read8i();
        a3=demuxer->read8i();
        a4=demuxer->read8i();
        hh=(a1>>2)&0x1f;
        mm=((a1&3)<<4)+(a2>>4);
        ss=((a2&7)<<3)+(a3>>5);
        ff=((a3&0x1f)<<1)+(a4>>7);
        if(!_run->nbGop)  ts=&(_run->firstStamp);
                else ts=&(_run->lastStamp);
        
        ts->hh=hh;
        ts->mm=mm;
        ts->ss=ss;
        ts->ff=ff;
        if(!_run->nbGop) memcpy(&(_run->lastStamp),&(_run->firstStamp),sizeof(_run->firstStamp));
        return 1;
}
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//
