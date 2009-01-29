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

#include "dmxPSPacket.h"

#include "avidemutils.h"
#include "ADM_quota.h"
#include "ADM_psAudioProbe.h"
#include "DIA_encoding.h"

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
}PSVideo;

typedef enum
{
    idx_startAtImage,
    idx_startAtGopOrSeq
}indexerState;
typedef struct
{
    uint64_t pts,dts,startAt;
    uint32_t offset;
    uint32_t frameType;
    uint32_t nbPics;
    indexerState state;
    psPacketLinear *pkt;
    int32_t        nextOffset;
}indexerData;

typedef enum
{
    markStart,
    markEnd,
    markNow
}markType;

/**
    \class PsIndexer
*/
class PsIndexer
{
protected:
        FILE *index;
        psPacketLinearTracker *pkt;
        listOfPsAudioTracks *audioTracks;
        DIA_encodingBase  *ui;
public:
                PsIndexer(void);
                ~PsIndexer();
        bool    run(const char *file);
        bool    writeVideo(PSVideo *video);
        bool    writeAudio(void);
        bool    writeSystem(const char *filename,bool append);
        bool    Mark(indexerData *data,psPacketInfo *s,markType update);

};
/**
      \fn psIndexer 
      \brief main indexing loop for mpeg2 payload
*/
uint8_t   psIndexer(const char *file)
{
bool r;
    PsIndexer *dx=new PsIndexer;
    r=dx->run(file);
    delete dx;
    return r;
}

/**
    \fn PsIndexer
*/
PsIndexer::PsIndexer(void)
{
    index=NULL;
    pkt=NULL;
    audioTracks=NULL;
    ui=createEncoding (25000);
}

/**
    \fn ~PsIndexer
*/
PsIndexer::~PsIndexer()
{
    if(index) qfclose(index);
    if(pkt) delete pkt;
    if( audioTracks) DestroyListOfPsAudioTracks(audioTracks);
    if(ui) delete ui;
    ui=NULL;
}
/**
    \fn run
*/  
bool PsIndexer::run(const char *file)
{
uint32_t temporal_ref,val;
uint64_t fullSize;
uint8_t buffer[50*1024];
bool seq_found=false;

PSVideo video;
indexerData  data;    
psPacketInfo info;

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
    pkt=new psPacketLinearTracker(0xE0);

    audioTracks=psProbeAudio(file);
    if(audioTracks)
    {
        for(int i=0;i<audioTracks->size();i++)
        {
                printf("[PsProbe] Found audio Track %d, pid=%x\n",i,(*audioTracks)[i]->esID);
                WAVHeader *hdr=&((*audioTracks)[i]->header);
                printf("[PsProbe] codec    : 0x%x \n",hdr->encoding);
                printf("[PsProbe] frequency: %"LU" Hz\n",hdr->frequency);
                printf("[PsProbe] channel  : %"LU" \n",hdr->channels);
                printf("[PsProbe] byterate : %"LU" Byte/s\n",hdr->byterate);

        }

    }
    pkt->open(file,false);
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

        pkt->getInfo(&info);
        info.offset-=4;

          switch(startCode)
                  {
                  case 0xB3: // sequence start
                          Mark(&data,&info,markStart);
                          data.state=idx_startAtGopOrSeq;
                          if(seq_found)
                          {
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
                          writeVideo(&video);
                          writeAudio();
                          pkt->resetStats();
                          qfprintf(index,"[Data]");
                          break;
                  case 0xb8: // GOP
                          // Update ui
                            {
                                float pos=data.startAt;
                                pos=pos/(float)fullSize;
                                pos*=100;
                                ui->setPercent( (uint32_t)pos);

                            }

                          if(!seq_found) continue;
                          if(data.state==idx_startAtGopOrSeq) 
                          {         
                                  continue;;
                          }
                          
                          Mark(&data,&info,markStart);
                          data.state=idx_startAtGopOrSeq;
                          break;
                  case 0x00 : // picture
                        {
                          int type;
                          markType update=markNow;
                          if(!seq_found)
                          { 
                                  continue;
                                  printf("[psIndexer]No sequence start yet, skipping..\n");
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
                                update=markEnd;
                          }
                          data.frameType=type;
                          Mark(&data,&info,update);
                          data.state=idx_startAtImage;
                          data.nbPics++;
                        }
                          break;
                  default:
                    break;
                  }
      }
    
        printf("\n");
        Mark(&data,&info,markStart);
        qfprintf(index,"\n[End]\n");
        qfclose(index);
        index=NULL;
        if(audioTracks) DestroyListOfPsAudioTracks( audioTracks);
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
bool  PsIndexer::Mark(indexerData *data,psPacketInfo *info,markType update)
{
    int offset=data->nextOffset;
    data->nextOffset=0;
    
     if( update==markStart)
     {
                offset=2;
     }
    if(update==markStart || update==markNow)
    {
        if(data->nbPics)
        {
            // Write previous image data (size) : TODO
            qfprintf(index,":%06"LX" ",data->pkt->getConsumed()+offset); // Size
        }
        else data->pkt->getConsumed();
    }
    if(update==markEnd || update==markNow)
    {
        if(data->frameType==1)
        {
            // If audio, also dump audio
            if(audioTracks)
            {
                qfprintf(index,"\nAudio bf:%08"LLX" ",data->startAt);
                for(int i=0;i<audioTracks->size();i++)
                {
                    uint8_t e=(*audioTracks)[i]->esID;
                    packetStats *s=pkt->getStat(e);
                    
                    qfprintf(index,"Pes:%x:%08"LLX":%"LD":%"LLD" ",e,s->startAt,s->startSize,s->startDts);
                }
                
            }
            // start a new line
            qfprintf(index,"\nVideo at:%08"LLX":%04"LX" Pts:%08"LLD":%08"LLD" ",data->startAt,data->offset,info->pts,info->dts);
            data->nextOffset=-2;
        }
    
        qfprintf(index,"%c",Type[data->frameType]);
    }
    if(update==markEnd || update==markNow)
    {
        data->pts=info->pts;
        data->dts=info->dts;
    }
    if(update==markStart || update==markNow)
    {
        data->startAt=info->startAt;
        data->offset=info->offset;
    }
}

/**
    \fn writeVideo
    \brief Write Video section of index file
*/
bool PsIndexer::writeVideo(PSVideo *video)
{
    qfprintf(index,"[Video]\n");
    qfprintf(index,"Width=%d\n",video->w);
    qfprintf(index,"Height=%d\n",video->h);
    qfprintf(index,"Fps=%d\n",video->fps);
    qfprintf(index,"Interlaced=%d\n",video->interlaced);
    qfprintf(index,"AR=%d\n",video->ar);
    return true;
}
/**
    \fn writeSystem
    \brief Write system part of index file
*/
bool PsIndexer::writeSystem(const char *filename,bool append)
{
    qfprintf(index,"PSD1\n");
    qfprintf(index,"[System]\n");
    qfprintf(index,"Type=P\n");
    qfprintf(index,"File=%s\n",filename);
    qfprintf(index,"Append=%d\n",append);
    return true;
}
/**
    \fn     writeAudio
    \brief  Write audio headers
*/
bool PsIndexer::writeAudio(void)
{
    if(!audioTracks) return false;
    qfprintf(index,"[Audio]\n");
    qfprintf(index,"Tracks=%d\n",audioTracks->size());
    for(int i=0;i<audioTracks->size();i++)
    {
        char head[30];
        psAudioTrackInfo *t=(*audioTracks)[i];
        sprintf(head,"Track%1d",i);
        qfprintf(index,"%s.pid=%x\n",head,t->esID);
        qfprintf(index,"%s.codec=%d\n",head,t->header.encoding);
        qfprintf(index,"%s.fq=%d\n",head,t->header.frequency);
        qfprintf(index,"%s.chan=%d\n",head,t->header.channels);
        qfprintf(index,"%s.br=%d\n",head,t->header.byterate);
    }
    return true;
}
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//

//EOF
