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
#include "ADM_quota.h"
#include "ADM_ps.h"
#include "ADM_psAudioProbe.h"
#include "DIA_working.h"
#include "ADM_vidMisc.h"
#include "DIA_coreToolkit.h"
#include "ADM_coreUtils.h"

static const char Type[5]={'X','I','P','B','P'};  // Frame type
static const char Structure[6]={'X','T','B','F','C','S'}; // Invalid, Top, Bottom, Frame, Frame+TFF, Frame+BFF

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
    pictureFieldTop=1,
    pictureFieldBottom=2,
    pictureTopFirst=4,
    pictureBottomFirst=5
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
    uint64_t        gopStartDts;
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
        uint64_t         timeOffset; // In 90 khz Tick
        bool             handleScrReset(uint64_t dts);
        BVector <scrGap> listOfScrReset;
        bool             writeScrReset(void);
public:
                PsIndexer(void);
                ~PsIndexer();
        uint8_t run(const char *file);
        bool    writeVideo(PSVideo *video);
        bool    writeAudio(void);
        bool    writeSystem(const char *filename,bool append);
        bool    Mark(indexerData *data,dmxPacketInfo *s,bool noptsdts,uint32_t size,markType update);
        uint64_t timeConvert(uint64_t x) // 90 kHz tick -> us
                {
                    if(x==ADM_NO_PTS) return ADM_NO_PTS;
                    double f=x;
                    f*=100.;
                    f/=9.;
                    f+=0.49;
                    return (uint64_t)f;
                }
};
/**
      \fn psIndexer 
      \brief main indexing loop for mpeg2 payload
*/
uint8_t   psIndexer(const char *file)
{
uint8_t r;
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
    headerDumped=false;
    lastValidVideoDts=ADM_NO_PTS;
    timeOffset=0;
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
uint8_t PsIndexer::run(const char *file)
{
    uint32_t temporal_ref,val;
    uint64_t fullSize;
    uint8_t buffer[50*1024];
    uint8_t res=0;
    bool seq_found=false;

    PSVideo video;
    indexerData  data;
    dmxPacketInfo info,lastInfo;

    memset(&video,0,sizeof(video));
    memset(&data,0,sizeof(data));
    memset(&info,0,sizeof(info));
    memset(&lastInfo,0,sizeof(lastInfo));
    data.picStructure=pictureFrame;
    char *indexName=(char *)malloc(strlen(file)+6);
    sprintf(indexName,"%s.idx2",file);

    index=qfopen(indexName,"wt");
    if(!index)
    {
        printf("[PsIndex] Cannot create %s\n",indexName);
        free(indexName);        
        return false;
    }

    int append=PS_DEFAULT_FRAGMENT_SIZE;
    {
    int nbFollowUps=ADM_probeSequencedFile(file,&append);
    if(nbFollowUps<0)
    {
        printf("[PsIndex] Cannot open %s\n",file);
        qfclose(index);
        index=NULL;
        free(indexName);
        indexName=NULL;
        return 0;
    }
    if(!nbFollowUps || false==GUI_Question(QT_TRANSLATE_NOOP("psdemuxer","There are several files with sequential file names. Should they be all loaded ?")))
        append=0;
    }
    writeSystem(file,append);
    pkt=new psPacketLinearTracker(0xE0);

    audioTracks=psProbeAudio(file,append);
    if(audioTracks)
    {
        for(int i=0;i<audioTracks->size();i++)
        {
                printf("[PsProbe] Found audio Track %d, pid=%x\n",i,(*audioTracks)[i]->esID);
                WAVHeader *hdr=&((*audioTracks)[i]->header);
                printf("[PsProbe] codec    : 0x%x \n",hdr->encoding);
                printf("[PsProbe] frequency: %" PRIu32" Hz\n",hdr->frequency);
                printf("[PsProbe] channel  : %" PRIu32" \n",hdr->channels);
                printf("[PsProbe] byterate : %" PRIu32" Byte/s\n",hdr->byterate);

        }

    }

    uint32_t lastConsumed=0;
    bool picEntryPending=false;
    bool timingIsInvalid=false;
    markType update=markStart;

    if(!pkt->open(file,append))
        goto cleanup;
    data.pkt=pkt;
    fullSize=pkt->getSize();

    ui=createWorking(QT_TRANSLATE_NOOP("psdemuxer","Indexing"));

      while(1)
      {
        if(!ui->isAlive())
        {
            res=ADM_IGN;
            break;
        }
        uint8_t startCode=pkt->findStartCode();
        if(picEntryPending && startCode!=0xB5) // picture not followed by extension
        {
            /*printf("Writing pic %u type: %d size: 0x%x startAt: 0x%x offset: 0x%x\n",
                data.nbPics,data.frameType,lastConsumed,lastInfo.startAt,lastInfo.offset);*/
            picEntryPending=false;
            Mark(&data,&lastInfo,timingIsInvalid,lastConsumed,update);
            lastConsumed=0;
            data.state=idx_startAtImage;
            data.nbPics++;
        }

        if(!pkt->stillOk()) break;
        pkt->getInfo(&info);
        info.offset-=4;
        //printf("start code: 0x%x pos: 0x%" PRIx64" offset: 0x%x pts: %s\n",startCode,pkt->getPos(),info.offset,ADM_us2plain(timeConvert(info.pts)));

          switch(startCode)
                  {
                  case 0xB3: // sequence start
                          //printf("Writing sequence start.\n");
                          lastConsumed+=pkt->getConsumed();
                          Mark(&data,&info,false,lastConsumed,markStart);
                          lastConsumed=0;
                          data.state=idx_startAtGopOrSeq;
                          if(seq_found)
                          {
                                  pkt->forward(8);  // Ignore
                                  continue;
                          }
                          //
                          seq_found=1;
                          val=pkt->readi32();
                          video.interlaced=0; // the correct value will be set later
                          video.w=val>>20;
                          video.w=((video.w+15)&~15);
                          video.h=((val>>8) & 0xfff);

                          video.ar = (val >> 4) & 0xf;

                          
                          video.fps= FPS[val & 0xf];
                          pkt->forward(4);
                          pkt->resetStats();
                          if(audioTracks)
                          {
                              uint32_t i,n=audioTracks->size();
                              for(i=0;i<n;i++)
                                  pkt->collectStats((*audioTracks)[i]->esID);
                          }
                          break;
                  case 0xB5: //  extension
                                { 
                                    uint8_t firstByte=pkt->readi8();
                                    uint8_t id=firstByte>>4;
                                    switch(id)
                                    {
                                        case 1: // Sequence extension
                                            break;
                                        case 2: // Sequence display extension
                                            if(firstByte&1)
                                            {
                                               for(int i=0;i<3;i++) pkt->readi8();
                                            }
                                            {
                                                uint32_t extSize=pkt->readi32();
                                                uint32_t eh=(extSize<<15)>>18;
                                                uint32_t ew=(extSize)>>18;
                                                //printf("**** %x ->%d x %d\n",extSize,ew,eh);
                                            }
                                            break;
                                        case 8: // picture coding extension (mpeg2)
                                        {
                                            // skip motion vector
                                            pkt->forward(1); // 4*4 bits
                                            uint8_t picture_structure=pkt->readi8();
                                            picture_structure&=3;
                                            uint8_t two=pkt->readi8();
                                            uint8_t three=pkt->readi8();
                                            bool tff=!!(two&0x80);
                                            bool progressive_frame=!!(three&0x80);
                                            if(!progressive_frame && picture_structure==3)
                                                picture_structure+=tff? 1 : 2;
                                            //printf("Pic struct: %d %s\n",picture_structure,progressive_frame? "progressive" : tff? "top field first" : "bottom field first");
                                            switch(picture_structure)
                                            {
                                            case 3: video.frameCount++;
                                                    data.picStructure=pictureFrame;
                                                    break;
                                            case 1: data.picStructure=pictureFieldTop;
                                                    video.fieldCount++;
                                                    break;
                                            case 2: data.picStructure=pictureFieldBottom;
                                                    video.fieldCount++;
                                                    break;
                                            case 4: video.frameCount++;
                                                    data.picStructure=pictureTopFirst;
                                                    break;
                                            case 5: video.frameCount++;
                                                    data.picStructure=pictureBottomFirst;
                                                    break;
                                            default: ADM_warning("picture structure %d met, this is illegal\n",picture_structure);
                                            }
                                            if(picEntryPending)
                                            {
                                                /*printf("Writing pic %u type: %d size: 0x%x startAt: 0x%x offset: 0x%x\n",
                                                    data.nbPics,data.frameType,lastConsumed,lastInfo.startAt,lastInfo.offset);*/
                                                picEntryPending=false;
                                                Mark(&data,&lastInfo,timingIsInvalid,lastConsumed,update);
                                                lastConsumed=0;
                                                data.state=idx_startAtImage;
                                                data.nbPics++;
                                            }
                                            if(!video.interlaced && picture_structure && picture_structure!=3)
                                                video.interlaced=1;
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

#define PREAMBLE qfprintf(index,"[Data]");
                          if(!seq_found) continue;
                          if(headerDumped==false)
                          {
                                PREAMBLE
                                headerDumped=true;
                          }
                          //printf("GOP, pics: %u\n",data.nbPics);
                          if(data.state==idx_startAtGopOrSeq) continue;
                          //printf("GOP, writing.\n");
                          lastConsumed+=pkt->getConsumed();
                          Mark(&data,&info,false,lastConsumed,markStart);
                          lastConsumed=0;
                          data.state=idx_startAtGopOrSeq;
                          break;
                  case 0x00 : // picture
                        {
                          int type;
                          update=markNow;
                          if(!seq_found)
                          {
                              printf("[psIndexer] No sequence start yet, skipping...\n");
                              continue;
                          }
                          if(headerDumped==false)
                          {
                                PREAMBLE
                                headerDumped=true;
                          }
                          // Get the size of the previous pic prior to reading the type of the current
                          // so that we don't need to account for 2 bytes offset in Mark().
                          // Add size of skipped pics with invalid type to the last valid one.
                          if(data.state==idx_startAtImage)
                              lastConsumed+=pkt->getConsumed();
                          val=pkt->readi16();
                          temporal_ref=val>>6;
                          type=7 & (val>>3);
                          if( type<1 ||  type>3)
                          {
                                  printf("[Indexer]Met illegal pic at %" PRIx64" + %" PRIx32"\n",
                                                  info.startAt,info.offset);
                                  continue;
                          }
                          //
#if 0
                            printf("Found pic of type %d at 0x%" PRIx64" dts=%s pts=%s\n",type,info.startAt,ADM_us2plain(timeConvert(info.dts)),ADM_us2plain(timeConvert(info.pts)));
#endif
                          //
                          if(lastValidVideoDts!=ADM_NO_PTS && info.dts!=ADM_NO_PTS)
                          {
                            if(lastValidVideoDts>(info.dts))
                                if(false==handleScrReset(info.dts)) goto theEnd;
                          }
                          if(info.dts!=ADM_NO_PTS)
                            {
                                    lastValidVideoDts=info.dts;
                            }
                          if(data.state==idx_startAtGopOrSeq) 
                              update=markEnd;
                          data.frameType=type;
                          if((info.pts!=ADM_NO_PTS || info.dts!=ADM_NO_PTS) &&
                              lastInfo.pts==info.pts && lastInfo.dts==info.dts &&
                              lastInfo.startAt==info.startAt) // both pics belong to the same packet
                          {
                              ADM_warning("Invalidating timing for frame %u\n",data.nbPics);
                              timingIsInvalid=true;
                          }else
                          {
                              timingIsInvalid=false;
                          }
                          lastInfo=info;
                          //printf("preparing pic entry, lastInfo.pts: %s\n",ADM_us2plain(timeConvert(lastInfo.pts)));
                          picEntryPending=true;
                        }
                          break;
                  default:
                    break;
                  }
      }
      if(res==ADM_IGN) goto cleanup;
theEnd:    
        printf("\n");
        // Dump progressive/frame gop
        if(picEntryPending)
            Mark(&data,&lastInfo,timingIsInvalid,lastConsumed,markStart);
        else
            Mark(&data,&info,timingIsInvalid,pkt->getConsumed(),markStart);

        qfprintf(index,"\n# Found %" PRIu32" images \n",data.nbPics); // Size
        qfprintf(index,"# Found %" PRIu32" frame pictures\n",video.frameCount); // Size
        qfprintf(index,"# Found %" PRIu32" field pictures\n",video.fieldCount); // Size

       
        // Now write the header
        writeVideo(&video);
        writeAudio();
        writeScrReset();
        qfprintf(index,"\n[End]\n");
        res=ADM_OK;
cleanup:
        qfclose(index);
        index=NULL;
        if(audioTracks) DestroyListOfPsAudioTracks( audioTracks);
        audioTracks=NULL;
        delete pkt;
        pkt=NULL;
	free(indexName);
        return res;
}
/**
    \fn   Mark
    \brief update the file
*/
bool  PsIndexer::Mark(indexerData *data,dmxPacketInfo *info,bool invalidateTiming,uint32_t size,markType update)
{
    uint64_t pts,dts;
    if(invalidateTiming)
    {
        pts=dts=ADM_NO_PTS;
    }else
    {
        pts=info->pts;
        dts=info->dts;
    }
    if(update==markStart || update==markNow)
    {
        if(data->nbPics)
        {
            // Write previous image data (size) : TODO
            qfprintf(index,":%06" PRIx32" ",size); // Size
        }
    }
    if(update==markEnd || update==markNow)
    {
        if(data->frameType==1)
        {
            // In field encoded streams both fields of the keyframe can be intra,
            // update data from info then.
            if(data->state==idx_startAtImage)
            {
                data->startAt=info->startAt;
                data->offset=info->offset;
            }
            // If audio, also dump audio
            if(audioTracks)
            {
                qfprintf(index,"\nAudio bf:%08" PRIx64" ",data->startAt);
                for(int i=0;i<audioTracks->size();i++)
                {
                    uint8_t e=(*audioTracks)[i]->esID;
                    packetStats *s=pkt->getStat(e);
                    
                    qfprintf(index,"Pes:%x:%08" PRIx64":%" PRIi32":%" PRId64" ",e,s->startAt,s->startSize,s->startDts);
                }
                
            }
            // start a new line
            qfprintf(index,"\nVideo at:%08" PRIx64":%04" PRIx32" Pts:%08" PRId64":%08" PRId64" ",data->startAt,data->offset,pts,dts);
            data->gopStartDts=dts;
        }
        int64_t deltaDts,deltaPts;
#if 0
        printf("Dts: %" PRId64",PTS :%" PRId64"\n",dts,pts);
#endif
        if(dts==ADM_NO_PTS || data->gopStartDts==ADM_NO_PTS) deltaDts=-1;
                else deltaDts=(int64_t)dts-(int64_t)data->gopStartDts;
        if(pts==ADM_NO_PTS || data->gopStartDts==ADM_NO_PTS) deltaPts=-1;
                else deltaPts=(int64_t)pts-(int64_t)data->gopStartDts;
        qfprintf(index,"%c%c:%" PRId64":%" PRId64,Type[data->frameType],Structure[data->picStructure%sizeof(Structure)],
                    deltaPts,deltaDts);
    }
    if(update==markEnd || update==markNow)
    {
        data->pts=pts;
        data->dts=dts;
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
    qfprintf(index,"Fps=%d\n",video->fps);
    qfprintf(index,"Interlaced=%d\n",video->interlaced);
    qfprintf(index,"AR=%d\n",video->ar);
    return true;
}

/**
    \fn writeScrReset
    \brief 
*/
bool PsIndexer::writeScrReset(void)
{
 // Dump resets
    int n=listOfScrReset.size();
    if(!n)
    {
        ADM_info("No SCR reset detected\n");
        return true;
    }
    qfprintf(index,"[ScrResets]\n");
    qfprintf(index,"NbResets=%d\n",n);
    for(int i=0;i<n;i++)
    {
        char head[30];
        sprintf(head,"Reset%1d",i);
        qfprintf(index,"#%s\n",ADM_us2plain(timeConvert(listOfScrReset[i].timeOffset)));
        qfprintf(index,"%s.position=%" PRId64"\n",head,listOfScrReset[i].position);
        qfprintf(index,"%s.timeOffset=%" PRId64"\n",head,listOfScrReset[i].timeOffset);
    }
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
/**
    \fn handleScrReset
*/
bool PsIndexer::handleScrReset(uint64_t dts)
{
#define PRETTY(x) ADM_us2plain(timeConvert(x))
        ADM_warning("DTS are going back, maybe several video appended ?\n");
        uint64_t newOffset=timeOffset+pkt->getLastVobuEndTime();
        uint64_t newPosition=pkt->getNextVobuPosition();
        ADM_info("Trying to correct with VOBU offset :%s\n", PRETTY(newOffset));

        uint64_t newDts=dts+newOffset;
        if(newDts>lastValidVideoDts+timeOffset)
        {
              ADM_info("SCR reset, using vobu to correct. New time offset %s, position 0x%" PRIx64"\n",
                        PRETTY(newOffset),newPosition);
              ADM_warning("last Valid Dts %s\n",PRETTY(lastValidVideoDts));
              timeOffset=newOffset;
              ADM_info("TimeOffset is now %s\n",PRETTY(timeOffset));

              scrGap newGap;
              newGap.position=newPosition;
              newGap.timeOffset=newOffset;
              listOfScrReset.append(newGap);
              return true;
        }else
        {
            ADM_warning("last Valid Dts %s\n",PRETTY(lastValidVideoDts));
            ADM_warning("current    Dts %s\n",PRETTY(dts));
            return false;
        }
}
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//

//EOF
