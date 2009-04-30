/***************************************************************************
                        Mpeg2 in PS indexer                                            
                             
    
    copyright            : (C) 2005/2009 by mean
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

#include "ADM_default.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

#include "dmxTSPacket.h"

#include "avidemutils.h"
#include "ADM_quota.h"
#include "ADM_tsAudioProbe.h"
#include "DIA_working.h"
#include "ADM_tsPatPmt.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h264_tag.h"

extern "C"
{
#define ADM_NO_CONFIG_H
#include "libavutil/common.h"
#include "libavutil/bswap.h"
#define INT_MAX (0x7FFFFFFF)
#include "ADM_lavcodec/bitstream.h"
#include "ADM_lavcodec/golomb.h"
}


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

typedef struct
{
    uint32_t w;
    uint32_t h;
    uint32_t fps;
    uint32_t interlaced;
    uint32_t ar;
    uint32_t pid;
}PSVideo;

typedef enum
{
    idx_startAtImage,
    idx_startAtGopOrSeq
}indexerState;
typedef struct
{
    uint64_t pts,dts; //startAt;
    //uint32_t offset;
    uint32_t frameType;
    uint32_t nbPics;
    indexerState state;
    tsPacketLinear *pkt;
    int32_t        nextOffset;
}indexerData;

typedef enum
{
    markStart,
    markEnd,
    markNow
}markType;

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif

/**
    \class TsIndexer
*/
class TsIndexer
{
protected:
        uint32_t        currentFrameType;
        uint32_t        beginConsuming;
        indexerState    currentIndexState;
protected:
        FILE *index;
        tsPacketLinearTracker  *pkt;
        listOfTsAudioTracks    *audioTracks;
        DIA_workingBase  *ui;
public:
                TsIndexer(listOfTsAudioTracks *tr);
                ~TsIndexer();
        bool    runMpeg2(const char *file,ADM_TS_TRACK *videoTrac);
        bool    runH264(const char *file,ADM_TS_TRACK *videoTrac);
        bool    writeVideo(PSVideo *video,ADM_TS_TRACK_TYPE trkType);
        bool    writeAudio(void);
        bool    writeSystem(const char *filename,bool append);
        bool    Mark(indexerData *data,dmxPacketInfo *s,uint32_t overRead);

};
/**
      \fn TsIndexer 
      \brief main indexing loop for mpeg2 payload
*/
uint8_t   tsIndexer(const char *file)
{
bool r;

    ADM_TS_TRACK *tracks;
    uint32_t nbTracks;
    listOfTsAudioTracks audioTrack;

    if(TS_scanForPrograms(file,&nbTracks,&tracks)==false) 
    {
        printf("[Ts Indexer] Scan of pmt failed\n");
        return false;
    }
    ADM_assert(tracks);
    ADM_assert(nbTracks);
    //
    // Now extract the datas from audio tracks & verify they are here
    tsPacketLinear *p=new tsPacketLinear(0);
    p->open(file,FP_DONT_APPEND);
    for(int i=1;i<nbTracks;i++)
    {
        tsAudioTrackInfo trk;
        trk.esId=tracks[i].trackPid;
        trk.trackType=tracks[i].trackType;
        if(true==tsGetAudioInfo(p,&trk))
        {
              audioTrack.push_back(trk);  
        }
    }
    delete p;
    printf("[TsIndexer] Audio probed, %d found, doing video\n",(int)audioTrack.size());
    //
    TsIndexer *dx=new TsIndexer(&audioTrack);
    switch(tracks[0].trackType)
    {
            case ADM_TS_MPEG2: 
                            r=dx->runMpeg2(file,&(tracks[0]));
                            break;
            case ADM_TS_H264: 
                            r=dx->runH264(file,&(tracks[0]));
                            break;
            default:
                        r=0;
                        break;
    }
    delete dx;
    delete [] tracks;
    return r;
}

/**
    \fn TsIndexer
*/
TsIndexer::TsIndexer(listOfTsAudioTracks *trk)
{
    index=NULL;
    pkt=NULL;
    audioTracks=NULL;
    ui=createWorking ("Indexing");
    audioTracks=trk;
}

/**
    \fn ~TsIndexer
*/
TsIndexer::~TsIndexer()
{
    if(index) qfclose(index);
    if(pkt) delete pkt;
    if(ui) delete ui;
    ui=NULL;
}
/**
    \fn runH264
    \brief Index H264 stream
*/  
bool TsIndexer::runH264(const char *file,ADM_TS_TRACK *videoTrac)
{
bool    pic_started=false;
bool    seq_found=false;

PSVideo video;
indexerData  data;    
uint64_t fullSize;
dmxPacketInfo info;
#if 0
    printf("Starting H264 indexer\n");
    if(!videoTrac) return false;
    if(videoTrac[0].trackType!=ADM_TS_H264)
    {
        printf("[Ts Indexer] Only H264 video supported\n");
        return false;
    }
    video.pid=videoTrac[0].trackPid;

    memset(&data,0,sizeof(data));
    char indexName[strlen(file)+5];
    sprintf(indexName,"%s.idx",file);
    index=qfopen(indexName,"wt");
    if(!index)
    {
        printf("[PsIndex] Cannot create %s\n",indexName);
        return false;
    }

    writeSystem(file,true);
    pkt=new tsPacketLinearTracker(videoTrac->trackPid, audioTracks);

    FP_TYPE append=FP_APPEND;
    pkt->open(file,append);
    data.pkt=pkt;
    fullSize=pkt->getSize();
      while(1)
      {

        uint32_t code=0xffff+0xffff0000;
        while((code!=1) && pkt->stillOk())
        {
            code=(code<<8)+pkt->readi8();
        }
        if(!pkt->stillOk()) break;
        uint8_t startCode=pkt->readi8();

        pkt->getInfo(&info);
        info.offset-=5;

//  1:0 2:Nal ref idc 5:Nal Type
        if(startCode&0x80) continue; // Marker missing
        

#define T(x) case NAL_##x: aprintf(#x" found\n");break;
        startCode&=0x1f;
#ifdef ADM_H264_VERBOSE
        switch(startCode)
        {
            T(NON_IDR);
            T(IDR);
            T(SPS);
            T(PPS);
            T(SEI);
            T(AU_DELIMITER);
            T(FILLER);
          default :aprintf("0x%02x ?\n",startCode);
        }
#endif
          
              /* Till we have a SPS no need to continue */
              if(!seq_found && startCode!=NAL_SPS) continue;
              if(!seq_found) // It is a SPS
              {
                    // Our firt frame is here
                    // Important to initialize the H264 decoder !
                      uint8_t buffer[60] ; // should be enough
                      uint32_t xA,xR;
                      // Get info
                      pkt->read(16,buffer);
                      if (extractSPSInfo(buffer, 16, &video.w,&video.h,&video.fps,&xA,&xR))
                      {
						  
                          printf("[TsIndexer] Found video %"LU"x%"LU", fps=%"LU"\n",video.w,video.h,video.fps);
                          seq_found=1;
                          Mark(&data,&info,markStart,4+16);
                          data.state=idx_startAtGopOrSeq;
                          writeVideo(&video,ADM_TS_H264);
                          writeAudio();
                          qfprintf(index,"[Data]");
                          // Rewind
#warning TODO
                      }
                      continue;
              }
              
              // Ignore multiple chunk of the same pic
              if((startCode==NAL_NON_IDR || startCode==NAL_IDR)&&pic_started) 
              {
                aprintf("Still capturing, ignore\n");
                continue;
              }
           
              
              switch(startCode)
                      {
                      case NAL_AU_DELIMITER:
							  pic_started = false;
                              break;
                      case NAL_SPS:
                              pic_started = false;
                              aprintf("Sps \n");
							  Mark(&data,&info,markStart,4);
                              data.state=idx_startAtGopOrSeq;
							  break;
                      case NAL_IDR:
                        {
                          markType update=markNow;
						  uint32_t frameType = 1 ;
                          if(data.state==idx_startAtGopOrSeq) 
                          {
                                update=markEnd;
                          }
						 
						  data.frameType=frameType;
                          pkt->readi8();
                          pkt->readi8();
                          pkt->readi8();
                          pkt->readi8();
                          Mark(&data,&info,update,4);
                          data.state=idx_startAtImage;
                          data.nbPics++;
						  pic_started = true;
                        }
					  case NAL_NON_IDR:
                        {
                            #define NON_IDR_PRE_READ 8
                          uint8_t header[NON_IDR_PRE_READ+4];
                          GetBitContext s;

                          markType update=markNow;
						  uint32_t frameType = 2;
                          if(data.state==idx_startAtGopOrSeq) 
                          {
                                update=markEnd;
                          }
#warning : need un-escaping!!!						 
						  data.frameType=frameType;
                          // Let's refine a bit the frame type...
                          pkt->read(4,header);
                          init_get_bits(&s, header, NON_IDR_PRE_READ*8);
                          int first_mb_in_slice,slice_type;

                            first_mb_in_slice= get_ue_golomb(&s);
                            slice_type= get_ue_golomb_31(&s);
                            if(slice_type>4) slice_type-=5;
                            switch(slice_type)
                            {

                                case 0 : data.frameType=2;break; // P
                                case 1 : data.frameType=3;break; // B
                                case 2 : data.frameType=1;break; // I
                                default : data.frameType=2;break; // SP/SI
                            }
                          Mark(&data,&info,update,4);
                          data.state=idx_startAtImage;
                          data.nbPics++;
						  pic_started = true;
                        }
						  break;
					  default:
						  break;
			  }
      }
        printf("\n");
        Mark(&data,&info,markStart,0);
        qfprintf(index,"\n[End]\n");
        qfclose(index);
        index=NULL;
        audioTracks=NULL;
        delete pkt;
        pkt=NULL;
#endif
        return true; 
}
//***********************************************************************
/**
    \fn runMpeg2
*/  
bool TsIndexer::runMpeg2(const char *file,ADM_TS_TRACK *videoTrac)
{
uint32_t temporal_ref,val;
uint64_t fullSize;
uint8_t buffer[50*1024];
bool seq_found=false;

PSVideo video;
indexerData  data;    
dmxPacketInfo info;

    if(!videoTrac) return false;
    if(videoTrac[0].trackType!=ADM_TS_MPEG2)
    {
        printf("[Ts Indexer] Only Mpeg2 video supported\n");
        return false;
    }
    video.pid=videoTrac[0].trackPid;

    memset(&data,0,sizeof(data));


    char indexName[strlen(file)+5];
    sprintf(indexName,"%s.idx",file);
    index=qfopen(indexName,"wt");
    if(!index)
    {
        printf("[PsIndex] Cannot create %s\n",indexName);
        return false;
    }
    writeSystem(file,true);
    pkt=new tsPacketLinearTracker(videoTrac->trackPid, audioTracks);

    FP_TYPE append=FP_APPEND;
    pkt->open(file,append);
    data.pkt=pkt;
    fullSize=pkt->getSize();
      while(1)
      {
        uint32_t code=0xffff+0xffff0000;
        while((code&0x00ffffff)!=1 && pkt->stillOk())
        {
            code=(code<<8)+pkt->readi8();
        }
        if(!pkt->stillOk()) break;
        uint8_t startCode=pkt->readi8();


          switch(startCode)
                  {
                  case 0xB3: // sequence start
                          if(seq_found)
                          {
                                  pkt->getInfo(&info);
                                  data.frameType=1;
                                  Mark(&data,&info,4);
                                  data.state=idx_startAtGopOrSeq;
                                  pkt->forward(8);  // Ignore
                                  continue;
                          }
                          //
                          seq_found=1;
                          val=pkt->readi32();
                          video.w=val>>20;
                          video.w=((video.w+15)&~15);
                          video.h= (((val>>8) & 0xfff)+15)& ~15;

                          video.ar = (val >> 4) & 0xf;
                          video.fps= FPS[val & 0xf];
                          pkt->forward(4);
                          writeVideo(&video,ADM_TS_MPEG2);
                          writeAudio();
                          qfprintf(index,"[Data]");
                          pkt->getInfo(&info);
                          data.frameType=1;
                          Mark(&data,&info,4+8);
                          data.state=idx_startAtGopOrSeq;
                          continue;

                          break;
                  case 0xb8: // GOP
                          // Update ui
                            {
                                uint64_t p;
                                p=pkt->getPos();
                                float pos=p;
                                pos=pos/(float)fullSize;
                                pos*=100;
                                ui->update( (uint32_t)pos);

                            }

                          if(!seq_found) continue;
                          if(data.state==idx_startAtGopOrSeq) 
                          {         
                                  continue;;
                          }
                          pkt->getInfo(&info);
                          Mark(&data,&info,4);
                          data.state=idx_startAtGopOrSeq;
                          break;
                  case 0x00 : // picture
                        {
                          int type;
                          markType update=markNow;
                          if(!seq_found)
                          { 
                                  continue;
                                  printf("[TsIndexer]No sequence start yet, skipping..\n");
                          }
                          
                          val=pkt->readi16();
                          temporal_ref=val>>6;
                          type=7 & (val>>3);
                          if( type<1 ||  type>3)
                          {
                                  printf("[Indexer]Met illegal pic at %"LLX" + %"LX"\n",
                                                  info.startAt,info.offset);
                                  continue;
                          }
                          
                          
                          if(data.state==idx_startAtGopOrSeq) 
                          {
                                  currentFrameType=type;
                          }else
                            {
                                  data.frameType=type;
                                  pkt->getInfo(&info);
                                  Mark(&data,&info,4+2);


                            }
                            data.state=idx_startAtImage;
                            data.nbPics++;
                        }
                          break;
                  default:
                    break;
                  }
      }
    
        printf("\n");
        Mark(&data,&info,2);
        qfprintf(index,"\n[End]\n");
        qfclose(index);
        index=NULL;
        audioTracks=NULL;
        delete pkt;
        pkt=NULL;
        return 1; 
}
/**
    \fn   Mark
    \brief update the file

    The offset part is due to the fact that we read 2 bytes from the pic header to know the pic type.
    So when going from a pic to a pic, it is self cancelling.
    If the beginning is not a pic, but a gop start for example, we had to add/remove those.

*/
bool  TsIndexer::Mark(indexerData *data,dmxPacketInfo *info,uint32_t overRead)
{
      
        uint32_t consumed=pkt->getConsumed()-overRead;
        if(data->nbPics)
        {
            qfprintf(index," %c:%06"LX,Type[currentFrameType],consumed-beginConsuming);
            beginConsuming=consumed;
        }else
        {
            beginConsuming=overRead;
            pkt->setConsumed(beginConsuming);
        }
            
        // If audio, also dump audio
        if(data->frameType==1)
        {
            if(audioTracks)
            {
                qfprintf(index,"\nAudio bf:%08"LLX" ",info->startAt);
                packetTSStats *s;
                uint32_t na;
                pkt->getStats(&na,&s);      
                ADM_assert(na==audioTracks->size());
                for(int i=0;i<na;i++)
                {   
                    packetTSStats *current=s+i;
                    qfprintf(index,"Pes:%x:%08"LLX":%"LD":%"LLD" ",
                                current->pid,current->startAt,current->startSize,current->startDts);
                }                
            }
            // start a new line
            qfprintf(index,"\nVideo at:%08"LLX":%04"LX" Pts:%08"LLD":%08"LLD" ",info->startAt,info->offset-overRead,info->pts,info->dts);
        }
        currentFrameType=data->frameType;
        
    return true;
}

/**
    \fn writeVideo
    \brief Write Video section of index file
*/
bool TsIndexer::writeVideo(PSVideo *video,ADM_TS_TRACK_TYPE trkType)
{
    qfprintf(index,"[Video]\n");
    qfprintf(index,"Width=%d\n",video->w);
    qfprintf(index,"Height=%d\n",video->h);
    qfprintf(index,"Fps=%d\n",video->fps);
    qfprintf(index,"Interlaced=%d\n",video->interlaced);
    qfprintf(index,"AR=%d\n",video->ar);
    qfprintf(index,"Pid=%d\n",video->pid);
 switch(trkType)
    {
        case ADM_TS_MPEG2: qfprintf(index,"VideoCodec=Mpeg2\n");break;;
        case ADM_TS_H264: qfprintf(index,"VideoCodec=H264\n");break;
        default: printf("[TsIndexer] Unsupported video codec\n");return false;

    }
    return true;
}
/**
    \fn writeSystem
    \brief Write system part of index file
*/
bool TsIndexer::writeSystem(const char *filename,bool append)
{
    qfprintf(index,"PSD1\n");
    qfprintf(index,"[System]\n");
    qfprintf(index,"Type=T\n");
    qfprintf(index,"File=%s\n",filename);
    qfprintf(index,"Append=%d\n",append);
    return true;
}
/**
    \fn     writeAudio
    \brief  Write audio headers
*/
bool TsIndexer::writeAudio(void)
{
    if(!audioTracks) return false;
    qfprintf(index,"[Audio]\n");
    qfprintf(index,"Tracks=%d\n",audioTracks->size());
    for(int i=0;i<audioTracks->size();i++)
    {
        char head[30];
        tsAudioTrackInfo *t=&(*audioTracks)[i];
        sprintf(head,"Track%1d",i);
        qfprintf(index,"%s.pid=%x\n",head,t->esId);
        qfprintf(index,"%s.codec=%d\n",head,t->wav.encoding);
        qfprintf(index,"%s.fq=%d\n",head,t->wav.frequency);
        qfprintf(index,"%s.chan=%d\n",head,t->wav.channels);
        qfprintf(index,"%s.br=%d\n",head,t->wav.byterate);
    }
    return true;
}
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//

//EOF
