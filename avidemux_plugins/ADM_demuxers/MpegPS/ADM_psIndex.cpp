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
#include "DIA_coreToolkit.h"
bool ADM_probeSequencedFile(const char *fileName);

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
    uint64_t        gopStartDts;
}indexerData;

typedef enum
{
    markStart,
    markEnd,
    markNow
}markType;
/**
    \struct scrGap
    \brief Map gap/reset in the scr flow to put everything back to linear / monotonic
*/
typedef struct
{
    uint64_t position;
    uint64_t timeOffset;
}scrGap;

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
bool PsIndexer::run(const char *file)
{
uint32_t temporal_ref,val;
uint64_t fullSize;
uint8_t buffer[50*1024];
bool seq_found=false;

PSVideo video;
indexerData  data;    
dmxPacketInfo info;
bool bAppend=false;
    
    memset(&video,0,sizeof(video));
    memset(&data,0,sizeof(data));
    data.picStructure=pictureFrame;
    char *indexName=(char *)alloca(strlen(file)+5);
    sprintf(indexName,"%s.idx2",file);

    FP_TYPE append=FP_DONT_APPEND;
    if(true==ADM_probeSequencedFile(file))
    {
        if(true==GUI_Question("There are several files with sequential file names. Should they be all loaded ?"))
               bAppend=true;
    }
    if(true==bAppend)
        append=FP_APPEND;

    index=qfopen(indexName,"wt");
    if(!index)
    {
        printf("[PsIndex] Cannot create %s\n",indexName);
        return false;
    }
    writeSystem(file,bAppend);
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
                                    uint8_t firstByte=pkt->readi8();
                                    uint8_t id=firstByte>>4;
                                    uint8_t two;
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
                                                printf("**** %x ->%d x %d\n",extSize,ew,eh);
                                            }
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
                          if(headerDumped==false)
                          {
                               
                                qfprintf(index,"[Data]");
                                headerDumped=true;
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
                          //
#if 0
                            printf("Found pic of type %d at 0x%"LLX" dts=%s pts=%s\n",type,info.startAt,ADM_us2plain(timeConvert(info.dts)),ADM_us2plain(timeConvert(info.pts)));
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
        writeScrReset();
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
            data->gopStartDts=info->dts;
            data->nextOffset=-2;
        }
        int64_t deltaDts,deltaPts;
#if 0
        printf("Dts%"LLD",PTS:%"LLD"\n",info->dts, info->pts);
#endif
        if(info->dts==-1 || data->gopStartDts==-1) deltaDts=-1;
                else deltaDts=(int64_t)info->dts-(int64_t)data->gopStartDts;
        if(info->pts==-1 || data->gopStartDts==-1) deltaPts=-1;
                else deltaPts=(int64_t)info->pts-(int64_t)data->gopStartDts;
        qfprintf(index,"%c%c:%"LLD":%"LLD,Type[data->frameType],Structure[data->picStructure&3],
                    deltaPts,deltaDts);
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
        qfprintf(index,"%s.position=%"LLD"\n",head,listOfScrReset[i].position);
        qfprintf(index,"%s.timeOffset=%"LLD"\n",head,listOfScrReset[i].timeOffset);
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
              ADM_info("SCR reset, using vobu to correct. New time offset %s, position 0x%"LLX"\n",
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
