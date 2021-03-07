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
#include "ADM_quota.h"
#include "ADM_tsAudioProbe.h"
#include "DIA_processing.h"
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
static const char Structure[6]={'X','T','B','F','C','S'}; // Invalid, Top, Bottom, Frame, Frame+TFF, Frame+BFF
static const char Type[5]={'X','I','P','B','D'};
#define VC1_MAX_SEQ_SIZE 64
#if 1
#define ASK_APPEND_SEQUENCED
#endif
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
    pictureFrame=3,
    pictureFieldTop=1,
    pictureFieldBottom=2,
    pictureTopFirst=4,
    pictureBottomFirst=5
}pictureStructure;

typedef struct
{
    uint64_t pts,dts; //startAt;
    //uint32_t offset;
    uint32_t frameType;
    pictureStructure picStructure;
    uint32_t nbPics;
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
    unitTypeSps=3,
    unitTypePicInfo=4
};

/**
    \class TsIndexer
*/
class TsIndexerBase
{
protected:
        uint64_t        beginConsuming;
        uint64_t        fullSize;
        vector <H264Unit> listOfUnits;
        H264Unit        thisUnit;
        bool            decodingImage;
        int             processedThisRound;
        
protected:
        FILE                    *index;
        tsPacketLinearTracker   *pkt;
        listOfTsAudioTracks     *audioTracks;
        DIA_processingBase      *gui;        
        bool                    updateUI(void);
        // H264
        bool                    addUnit(indexerData &data,int unitType,const H264Unit &unit,uint32_t overRead);
        bool                    dumpUnits(indexerData &data,uint64_t nextConsumed,const dmxPacketInfo *nextPacket);
public:
                TsIndexerBase(listOfTsAudioTracks *tr);
        virtual ~TsIndexerBase();
virtual uint8_t run(const char *file,ADM_TS_TRACK *videoTrac)=0;
        bool    writeVideo(TSVideo *video,ADM_TS_TRACK_TYPE trkType);
        bool    writeAudio(void);
        bool    writeSystem(const char *filename,int append=0);
        bool    updateLastUnitStructure(int structure);
        bool    updatePicStructure(TSVideo &video,const uint32_t t)
                {
                    switch(t)
                    {
                        case 3: video.frameCount++;
                                thisUnit.imageStructure=pictureFrame;
                                break;
                        case 1: thisUnit.imageStructure=pictureFieldTop;
                                video.fieldCount++;
                                break;
                        case 2: thisUnit.imageStructure=pictureFieldBottom;
                                video.fieldCount++;
                                break;
                        case 4: video.frameCount++;
                                thisUnit.imageStructure=pictureTopFirst;
                                break;
                        case 5: video.frameCount++;
                                thisUnit.imageStructure=pictureBottomFirst;
                                break;
                        default: ADM_warning("frame type 0 met, this is illegal\n");
                    }
                    return true;
                }
};

/**
    \class TsIndexerH264
*/
class TsIndexerH264 : public TsIndexerBase
{
protected:
        bool            findH264SPS(tsPacketLinearTracker *pkt,TSVideo &video);
        
protected:
        ADM_SPSInfo             spsInfo;
        uint8_t                 decodeSEI(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength,pictureStructure *nextpicstruct);
        #define                 ADM_NAL_BUFFER_SIZE (2*1024) // only used to decode SEI, should plenty enough
        uint8_t                 payloadBuffer[ADM_NAL_BUFFER_SIZE];
        uint8_t                 spsCache[ADM_NAL_BUFFER_SIZE];
        uint32_t                spsLen;
public:
                TsIndexerH264(listOfTsAudioTracks *tr) : TsIndexerBase(tr)
                {
                    memset(&spsInfo,0,sizeof(spsInfo));
                    spsLen=0;
                }
                ~TsIndexerH264()
                {
                  
                } 
        uint8_t run(const char *file,ADM_TS_TRACK *videoTrac);
};

class TsIndexerVC1: public TsIndexerBase
{
protected:
        VC1Context      vc1Context;
        
protected:
        bool            decodeVC1Seq(tsGetBits &bits,TSVideo &video);
        bool            decodeVC1Pic(tsGetBits &bits,uint32_t &frameType,uint32_t &frameStructure);
public:
                        ~TsIndexerVC1()
                        {
                          
                        }
        uint8_t         run(const char *file,ADM_TS_TRACK *videoTrac);
                        TsIndexerVC1(listOfTsAudioTracks *tr) : TsIndexerBase(tr)
                        {

                        }
};
//--
/**
*
    \class TsIndexerH264
*/
class TsIndexerMpeg2 : public TsIndexerBase
{
protected:

public:
                ~TsIndexerMpeg2()
                {
                  
                }
        uint8_t run(const char *file,ADM_TS_TRACK *videoTrac);
                TsIndexerMpeg2(listOfTsAudioTracks *tr) : TsIndexerBase(tr)
                {

                }
       
};
//--
/**
*
    \class TsIndexerH264
*/
class TsIndexerH265 : public TsIndexerBase
{
protected:
        ADM_SPSinfoH265 info;
        
        bool            findH265VPS(tsPacketLinearTracker *pkt,TSVideo &video);
        bool            decodeH265SPS(tsPacketLinearTracker *pkt);
        int             decodePictureTypeH265(int nalType,getBits &bits) ;     
        bool            decodeSEIH265(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength,   pictureStructure *picStruct);
        
protected:
public:
                ~TsIndexerH265()
                {
                  
                }
        uint8_t run(const char *file,ADM_TS_TRACK *videoTrac);
                TsIndexerH265(listOfTsAudioTracks *tr) : TsIndexerBase(tr)
                {

                }
        
};
//---
#if 0
/**
*
    \class TsIndexerH264
*/
class TsIndexer
{
protected:
        uint32_t        beginConsuming;
        uint64_t        fullSize;
        VC1Context      vc1Context;
        vector <H264Unit> listOfUnits;
        H264Unit        thisUnit;
        bool            decodingImage;
        int             processedThisRound;
        ADM_SPSinfoH265 info;
        
        bool            findH264SPS(tsPacketLinearTracker *pkt,TSVideo &video);
        bool            findH265VPS(tsPacketLinearTracker *pkt,TSVideo &video);
        bool            decodeH265SPS(tsPacketLinearTracker *pkt);
        int             decodePictureTypeH265(int nalType,getBits &bits) ;       
        
protected:
        FILE                    *index;
        tsPacketLinearTracker   *pkt;
        listOfTsAudioTracks     *audioTracks;
        DIA_processingBase      *gui;
        ADM_SPSInfo             spsInfo;
        bool                    updateUI(void);
        bool                    decodeSEI(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength,pictureStructure *nextpicstruct);
        bool                    decodeSEIH265(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength,pictureStructure *nextpicstruct);
        bool                    decodeVC1Seq(tsGetBits &bits,TSVideo &video);
        bool                    decodeVC1Pic(tsGetBits &bits,uint32_t &frameType,uint32_t &frameStructure);
        // H264
        bool                    addUnit(indexerData &data,int unitType,const H264Unit &unit,uint32_t overRead);
        bool                    dumpUnits(indexerData &data,uint64_t nextConsumed,const dmxPacketInfo *nextPacket);
        #define                 ADM_NAL_BUFFER_SIZE (2*1024) // only used to decode SEI, should plenty enough
        uint8_t                 payloadBuffer[ADM_NAL_BUFFER_SIZE];
public:
                TsIndexer(listOfTsAudioTracks *tr);
                ~TsIndexer();
        bool    runMpeg2(const char *file,ADM_TS_TRACK *videoTrac);
        bool    runH264(const char *file,ADM_TS_TRACK *videoTrac);
        bool    runH265(const char *file,ADM_TS_TRACK *videoTrac);
        bool    runVC1(const char *file,ADM_TS_TRACK *videoTrac);
        bool    writeVideo(TSVideo *video,ADM_TS_TRACK_TYPE trkType);
        bool    writeAudio(void);
        bool    writeSystem(const char *filename,bool append);
        bool    updatePicStructure(TSVideo &video,const uint32_t t)
                        {
                                            switch(t)
                                            {
                                                case 3: video.frameCount++;
                                                        thisUnit.imageStructure=pictureFrame;
                                                        break;
                                                case 1:  thisUnit.imageStructure=pictureFieldTop;
                                                         video.fieldCount++;
                                                         break;
                                                case 2:  thisUnit.imageStructure=pictureFieldBottom;
                                                         video.fieldCount++;
                                                         break;
                                                default: ADM_warning("frame type 0 met, this is illegal\n");
                                            }
                                            return true;
                        }
};
#endif
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//
#endif
//EOF
