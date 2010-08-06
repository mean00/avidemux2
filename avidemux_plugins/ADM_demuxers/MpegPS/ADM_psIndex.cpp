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

#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

#include "dmxPSPacket.h"

#include "avidemutils.h"
#include "ADM_quota.h"
#include "ADM_psAudioProbe.h"
#include "DIA_working.h"
#include "ADM_indexFile.h"
#include "ADM_vidMisc.h"

static const char Type[5]={'X','I','P','B','P'};  // Frame type
static const char Structure[4]={'X','T','B','F'}; // X Top Bottom Frame

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
    uint32_t frameCount;
    uint32_t fieldCount;
    uint32_t ar;
}PSVideo;

typedef enum
{
    idx_startAtImage,
    idx_startAtGopOrSeq
}indexerState;

typedef enum
{
    pictureFrame=3,
    pictureTopField=1, 
    pictureBottomField=2
}pictureStructure;

typedef struct
{
    uint64_t        pts,dts,startAt;
    uint32_t        offset;
    uint32_t        frameType;
    pictureStructure picStructure;
    uint32_t        nbPics;
    indexerState    state;
    psPacketLinear *pkt;
    int32_t         nextOffset;
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
        DIA_workingBase  *ui;
        bool             headerDumped;
        uint64_t         lastValidVideoDts;
       
public:
                PsIndexer(void);
                ~PsIndexer();
        bool    run(const char *file);
        bool    writeVideo(PSVideo *video);
        bool    writeAudio(void);
        bool    writeSystem(const char *filename,bool append);
        bool    Mark(indexerData *data,dmxPacketInfo *s,markType update);
        uint64_t timeConvert(uint64_t x) // 90 kHz tick -> us
                    {
                         if(x==ADM_NO_PTS) return ADM_NO_PTS;
                            x=x*1000;
                            x/=90;
                            return x;
                    }
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
    ui=createWorking ("Indexing");
    headerDumped=false;
    lastValidVideoDts=ADM_NO_PTS;
    
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
dmxPacketInfo info;
    
    memset(&video,0,sizeof(video));
    memset(&data,0,sizeof(data));
    data.picStructure=pictureFrame;
    char *indexName=(char *)alloca(strlen(file)+5);
    sprintf(indexName,"%s.idx2",file);
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

    FP_TYPE append=FP_APPEND;
    pkt->open(file,append);
    data.pkt=pkt;
    fullSize=pkt->getSize();
      while(1)
      {
        
        uint8_t startCode=pkt->findStartCode();
        if(!pkt->stillOk()) break;
        pkt->getInfo(&info);
        info.offset-=4;

          switch(startCode)
                  {
                  

                            ;
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
                          video.interlaced=0; // how to detect ?
                          video.w=val>>20;
                          video.w=((video.w+15)&~15);
                          video.h= (((val>>8) & 0xfff)+15)& ~15;

                          video.ar = (val >> 4) & 0xf;

                          
                          video.fps= FPS[val & 0xf];
                          pkt->forward(4);
                          pkt->resetStats();
                          break;
                  case 0xB5: //  extension
                                { 
                                    uint8_t id=pkt->readi8()>>4;
                                    uint8_t two;
                                    switch(id)
                                    {
                                        case 1: // Sequence extension
                                            val=(val>>3)&1; // gop type progressive, unreliable, not used
                                            break;
                                        case 8: // picture coding extension (mpeg2)
                                        {
                                            // skip motion vector
                                            uint8_t picture_structure;
                                            pkt->forward(1); // 4*4 bits
                                            two=pkt->readi8();
                                            picture_structure=(two)&3;
                                            
                                            //printf("Picture type %02x struct:%x\n",two,picture_structure);
                                            switch(picture_structure)
                                            {
                                            case 3: video.frameCount++;
                                                    data.picStructure=pictureFrame;
                                                    break;
                                            case 1:  data.picStructure=pictureTopField;
                                                     video.fieldCount++;
                                                     break;
                                            case 2:  data.picStructure=pictureBottomField;
                                                     video.fieldCount++;
                                                     break;
                                            default: ADM_warning("frame type 0 met, this is illegal\n");
                                            }
                                        }
                                        default:break;
                                    }
                                }
#if 0
#define EXT_SIZE 6
                                    uint8_t ext[EXT_SIZE];
                                    pkt->read(EXT_SIZE,ext);
                                    printf("Sequence Extension :");
                                    for(int i=0;i<EXT_SIZE;i++) printf("%02x ",ext[i]);
                                    printf("\n");
#endif
                                
                                break;
                  case 0xb8: // GOP
                          // Update ui
                            {
                                float pos=data.startAt;
                                pos=pos/(float)fullSize;
                                pos*=100;
                                ui->update( (uint32_t)pos);

                            }

                          if(!seq_found) continue;
                          if(headerDumped==false)
                          {
                               
                                qfprintf(index,"[Data]");
                                headerDumped=true;
                          }
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
                          if(lastValidVideoDts!=ADM_NO_PTS && info.dts!=ADM_NO_PTS)
                          {
                                    if(lastValidVideoDts>info.dts)
                                    {
                                            ADM_warning("DTS are going back, aborting, maybe several video appended ?");
                                            ADM_warning("last Valid Dts %s\n",ADM_us2plain(timeConvert(lastValidVideoDts)));
                                            ADM_warning("current    Dts %s\n",ADM_us2plain(timeConvert(info.dts)));
                                            goto theEnd;
                                     }
                            }
                            if(info.dts!=ADM_NO_PTS)
                            {
                                    lastValidVideoDts=info.dts;
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
theEnd:    
        printf("\n");
        // Dump progressive/frame gop
        Mark(&data,&info,markStart);
        
        qfprintf(index,"\n# Found %"LU" images \n",data.nbPics); // Size
        qfprintf(index,"# Found %"LU" frame pictures\n",video.frameCount); // Size
        qfprintf(index,"# Found %"LU" field pictures\n",video.fieldCount); // Size
        // Now write the header
        writeVideo(&video);
        writeAudio();
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
bool  PsIndexer::Mark(indexerData *data,dmxPacketInfo *info,markType update)
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
    
        qfprintf(index,"%c%c",Type[data->frameType],Structure[data->picStructure&3]);
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
    return true;
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
    if(video->interlaced)
        qfprintf(index,"Fps=%d\n",video->fps*2);
    else
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
    qfprintf(index,"Version=%d\n",ADM_INDEX_FILE_VERSION);
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
