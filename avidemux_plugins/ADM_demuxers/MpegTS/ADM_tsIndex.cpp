/***************************************************************************
                        Mpeg2 in PS indexer                                            
                             
    VC1: /!\ Escaping not done (yet)

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
#include "ADM_tsIndex.h"

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
            if(TS_guessContent(file,&nbTracks,&tracks)==false) 
            {
                printf("[Ts Indexer] Brute force scan failed\n");
                return false;
            }
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
        trk.mux=ADM_TS_MUX_NONE;
        trk.language=tracks[i].language;
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
            case ADM_TS_VC1: 
                            r=dx->runVC1(file,&(tracks[0]));
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
    memset(&spsInfo,0,sizeof(spsInfo));
    index=NULL;
    pkt=NULL;
    audioTracks=NULL;
    ui=createWorking ("Indexing");
    audioTracks=trk;
    ticktock.reset();
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
    \fn updateUI
*/
void TsIndexer::updateUI(void)
{
        if(ticktock.getElapsedMS()<1000) return;
        ticktock.reset();
        uint64_t p;
        p=pkt->getPos();
        float pos=p;
        pos=pos/(float)fullSize;
        pos*=100;
        ui->update( (uint32_t)pos);
}
/**
    \fn writeVideo
    \brief Write Video section of index file
*/
bool TsIndexer::writeVideo(TSVideo *video,ADM_TS_TRACK_TYPE trkType)
{
    qfprintf(index,"[Video]\n");
    qfprintf(index,"Width=%d\n",video->w);
    qfprintf(index,"Height=%d\n",video->h);
    qfprintf(index,"Fps=%d\n",video->fps);
    qfprintf(index,"Interlaced=%d\n",video->interlaced);
    qfprintf(index,"AR=%d\n",video->ar);
    qfprintf(index,"Pid=%d\n",video->pid);
    if(video->extraDataLength)
    {
        qfprintf(index,"ExtraData=%d ",video->extraDataLength);
        for(int i=0;i<video->extraDataLength;i++)
            qfprintf(index," %02x",video->extraData[i]);
        qfprintf(index,"\n");
    }
 switch(trkType)
    {
        case ADM_TS_MPEG2: qfprintf(index,"VideoCodec=Mpeg2\n");break;;
        case ADM_TS_H264:  qfprintf(index,"VideoCodec=H264\n");break;
        case ADM_TS_VC1:   qfprintf(index,"VideoCodec=VC1\n");break;
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
    qfprintf(index,"Version=%d\n",ADM_INDEX_FILE_VERSION);
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
        qfprintf(index,"%s.muxing=%d\n",head,t->mux);
        qfprintf(index,"%s.language=%s\n",head,t->language.c_str());
        if(t->extraDataLen)
        {
            qfprintf(index,"%s.extraData=%d",head,t->extraDataLen);
            uint8_t *p=t->extraData;
            for(int i=0;i<t->extraDataLen;i++)
                qfprintf(index," %02x",p[i]);
            qfprintf(index,"\n");
        }
    }
    return true;
}


/**
    \fn dumpUnits
*/
bool TsIndexer::dumpUnits(indexerData &data,uint64_t nextConsumed,const dmxPacketInfo *nextPacket)
{
        // if it contain a SPS or a intra/idr, we start a new line
        bool mustFlush=false;
        int n=listOfUnits.size();
        int picIndex=0;
        H264Unit *unit=&(listOfUnits[0]);
        pictureStructure pictStruct=pictureFrame;
        
        // if I, IDR or SPS we start a new line
        for(int i=0;i<n;i++)
        {
            switch(listOfUnits[i].unitType)
            {
                case unitTypeSps: mustFlush=true;;break;
                case unitTypePic: 
                            picIndex=i;
                            if(listOfUnits[i].imageType==1 || listOfUnits[i].imageType==4)
                            mustFlush=true;
                            break;
                case unitTypeSei:
                            pictStruct=listOfUnits[i].imageStructure;
                            break;
                default:
                        ADM_assert(0);
                        break;
            }
        }
        dmxPacketInfo *pic=&(listOfUnits[picIndex].packetInfo);
        dmxPacketInfo *p=&(unit->packetInfo);
        H264Unit      *picUnit=&(listOfUnits[picIndex]);
        if(mustFlush) 
        {
            if(audioTracks)
            {
                qfprintf(index,"\nAudio bf:%08"PRIx64" ",nextPacket->startAt);
                packetTSStats *s;
                uint32_t na;
                pkt->getStats(&na,&s);      
                ADM_assert(na==audioTracks->size());
                for(int i=0;i<na;i++)
                {   
                    packetTSStats *current=s+i;
                    qfprintf(index,"Pes:%x:%08"PRIx64":%"PRIi32":%"PRId64" ",
                                current->pid,current->startAt,current->startSize,current->startDts);
                }                
            }
            data.beginPts=pic->pts;
            data.beginDts=pic->dts;
            // start a new line
            qfprintf(index,"\nVideo at:%08"PRIx64":%04"PRIx32" Pts:%08"PRId64":%08"PRId64" ",
                        p->startAt,p->offset-unit->overRead,pic->pts,pic->dts);
        }
       
        
            int64_t deltaPts,deltaDts;

            if(data.beginPts==-1 || pic->pts==-1) deltaPts=-1;
                else deltaPts=pic->pts-data.beginPts;

            if(data.beginDts==-1 || pic->dts==-1) deltaDts=-1;
                else deltaDts=pic->dts-data.beginDts;            


        qfprintf(index," %c%c",Type[picUnit->imageType],Structure[pictStruct&3]);
        qfprintf(index,":%06"PRIx32,nextConsumed-beginConsuming);
        qfprintf(index,":%"PRId64":%"PRId64,deltaPts,deltaDts);
    
        beginConsuming=nextConsumed;
        listOfUnits.clear();
        return true;
}
/**
    \fn addUnit
*/
bool TsIndexer::addUnit(indexerData &data,int unitType2,const H264Unit &unit,uint32_t overRead)
{
        H264Unit myUnit=unit;
        myUnit.unitType=unitType2;
        myUnit.overRead=overRead;
#if 0
        printf("Adding new unit of type %x unitType2 PTS=%"PRId64" DTS=%"PRId64"\n",unitType2,
                    unit.packetInfo.pts,
                    unit.packetInfo.dts
                    );
#endif
        int n=listOfUnits.size();
        if(n)
            if(listOfUnits[n-1].unitType==unitTypePic)
            {
                dumpUnits(data,myUnit.consumedSoFar-overRead,&(unit.packetInfo));
                updateUI();
            }
        listOfUnits.push_back(myUnit);
        return true;
}
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//

//EOF
