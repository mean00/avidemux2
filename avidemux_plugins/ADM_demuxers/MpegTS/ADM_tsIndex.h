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
#ifndef ADM_TS_INDEX_H
#define ADM_TS_INDEX_H

#include "ADM_cpp.h"
using std::string;
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
#include "ADM_clock.h"
#include "ADM_indexFile.h"
#include "ADM_getbits.h"
#include "ADM_tsGetBits.h"
#include "ADM_coreUtils.h"

#if (1) || !defined(ADM_DEBUG)
#define aprintf(...) {}
#else
#define aprintf printf
#endif
static const char Structure[4]={'X','T','B','F'}; // X Top Bottom Frame
static const char Type[5]={'X','I','P','B','D'};
#define VC1_MAX_SEQ_SIZE 64
class TSVideo
{
public:
    TSVideo(void) {w=h=fps=interlaced=ar=pid=frameCount=fieldCount=0;extraDataLength=0;}
    uint32_t w;
    uint32_t h;
    uint32_t fps;
    uint32_t interlaced;
    uint32_t ar;
    uint32_t pid;
    uint32_t frameCount;
    uint32_t fieldCount;
    uint32_t extraDataLength;
    uint8_t  extraData[VC1_MAX_SEQ_SIZE];
};

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
    uint64_t pts,dts; //startAt;
    //uint32_t offset;
    uint32_t frameType;
    pictureStructure picStructure;
    uint32_t nbPics;
    indexerState state;
    tsPacketLinear *pkt;
    int32_t        nextOffset;
    uint64_t beginPts,beginDts;
    uint64_t prevPts,prevDts;
}indexerData;

/**
    \class VC1Context
*/
class VC1Context
{
public:
        bool advanced;
        bool interlaced;
        bool interpolate;
        VC1Context() {advanced=false;interlaced=false;interpolate=false;}

};
/**
    \class H264Unit
*/
class H264Unit
{
public:
    int             unitType;
    dmxPacketInfo   packetInfo;
    uint64_t        consumedSoFar;
    uint32_t        overRead;
    int             imageType;
    pictureStructure imageStructure;
    uint32_t        recoveryCount;
                    H264Unit() {memset(this,0,sizeof(*this));recoveryCount=0xff;imageStructure=pictureFrame;}
};
enum
{
    unitTypeSei=1,
    unitTypePic=2,
    unitTypeSps=3
};

/**
    \class TsIndexer
*/
class TsIndexer
{
protected:
        uint32_t        currentFrameType;
        uint32_t        beginConsuming;
        indexerState    currentIndexState;
        uint64_t        fullSize;
        Clock           ticktock;
        VC1Context      vc1Context;
protected:
        FILE                    *index;
        tsPacketLinearTracker   *pkt;
        listOfTsAudioTracks     *audioTracks;
        DIA_workingBase         *ui;
        ADM_SPSInfo             spsInfo;
        void                    updateUI(void);
        bool                    decodeSEI(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength,pictureStructure *nextpicstruct);
        bool                    decodeVC1Seq(tsGetBits &bits,TSVideo &video);
        bool                    decodeVC1Pic(tsGetBits &bits,uint32_t &frameType,uint32_t &frameStructure);
        // H264
        bool                    addUnit(indexerData &data,int unitType,const H264Unit &unit,uint32_t overRead);
        bool                    dumpUnits(indexerData &data,uint64_t nextConsumed);
public:
                TsIndexer(listOfTsAudioTracks *tr);
                ~TsIndexer();
        bool    runMpeg2(const char *file,ADM_TS_TRACK *videoTrac);
        bool    runH264(const char *file,ADM_TS_TRACK *videoTrac);
        bool    runVC1(const char *file,ADM_TS_TRACK *videoTrac);
        bool    writeVideo(TSVideo *video,ADM_TS_TRACK_TYPE trkType);
        bool    writeAudio(void);
        bool    writeSystem(const char *filename,bool append);
        bool    Mark(indexerData *data,dmxPacketInfo *s,uint32_t overRead);
        bool    updatePicStructure(TSVideo &video,indexerData &idata, const uint32_t t)
                        {
                                            switch(t)
                                            {
                                                case 3: video.frameCount++;
                                                        idata.picStructure=pictureFrame;
                                                        break;
                                                case 1:  idata.picStructure=pictureTopField;
                                                         video.fieldCount++;
                                                         break;
                                                case 2:  idata.picStructure=pictureBottomField;
                                                         video.fieldCount++;
                                                         break;
                                                default: ADM_warning("frame type 0 met, this is illegal\n");
                                            }
                                            return true;
                        }
};
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//
#endif
//EOF
