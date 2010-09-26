/**
    \file ADM_videoCopyFromAnnexB
    \brief Convert from annexB h264 to iso on the fly
    (c) Mean 2010/GPLv2

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_cpp.h"
using std::string;
#include "ADM_default.h"
#include "ADM_videoCopy.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_coreUtils.h"
#include "ADM_h264_tag.h"
extern ADM_Composer *video_body; // Fixme!

extern bool ADM_findH264StartCode(uint8_t *start, uint8_t *end,uint8_t *outstartcode,uint32_t *offset);
extern void mixDump(uint8_t *ptr, uint32_t len);


//#define ANNEX_B_DEBUG

#if defined(ANNEX_B_DEBUG)
#define aprintf ADM_info
#define check isNalValid
#else
#define aprintf(...) {}
#define check(...) {}
#endif

#define MAX_NALU_PER_CHUNK 20

typedef struct
{
    uint8_t  *start;
    uint32_t size;   // size of payload excluding nalu type
    uint8_t  nalu;
}NALU_descriptor;

static uint32_t readBE32(uint8_t *p)
{
    uint32_t v=(p[0]<<8)+p[1];
    uint32_t u=(p[2]<<8)+p[3];
    return (v<<16)+u;
}


static void writeBE32(uint8_t *p, uint32_t size)
{
    p[0]=size>>24;
    p[1]=(size>>16)&0xff;
    p[2]=(size>>8)&0xff;
    p[3]=(size>>0)&0xff;
}
static const char *nalType[13]=
{
    "invalid","nonIdr","invalid","invalid","invalid",
    "idr","sei","sps","pps","au delimiter","invalid","invalid","filler"
};
static bool isNalValid(int nal)
{
    nal&=0x1f;
    switch(nal)
    {
        case 1:case 5:case 6: case 7: case 8: case 9:case 0xc:
                break;
        default:
            ADM_warning("Invalid NAL 0x%x\n",nal);
    }
    if(nal<=12) ADM_info("Nal : %s",nalType[nal]);
    return true;
}
/**
    \fn splitNalu
    \brief split a nalu annexb size into a list of nalu descriptor
*/
static int splitNalu(uint8_t *start, uint8_t *end, uint32_t maxNalu,NALU_descriptor *desc)
{
bool first=true;
uint8_t *head=start;
uint32_t offset;
uint8_t startCode,oldStartCode=0xff;
int index=0;
      while(true==ADM_findH264StartCode(head,end,&startCode,&offset))
      {
            if(true==first)
            {
                head+=offset;
                first=false;
                oldStartCode=startCode;
                continue;
            }
        ADM_assert(index<maxNalu);
        desc[index].start=head;
        desc[index].size=offset-5;
        desc[index].nalu=oldStartCode;
        index++;
        head+=offset;
        oldStartCode=startCode;
      }
    // leftover
    desc[index].start=head;
    desc[index].size=(uint32_t)(end-head);
    desc[index].nalu=oldStartCode;
    index++;
    return index;
}
/**
    \fn findNalu
    \brief lookup for a specific NALU in the given buffer
*/
static int findNalu(uint32_t nalu,uint32_t maxNalu,NALU_descriptor *desc)
{
    for(int i=0;i<maxNalu;i++)
    {
            if((desc[i].nalu&0x1f) == (nalu&0x1f))
                return i;
    }
    return -1;
}

/**
    \fn ctor
*/
ADM_videoStreamCopyFromAnnexB::ADM_videoStreamCopyFromAnnexB(uint64_t startTime,uint64_t endTime):
      ADM_videoStreamCopy(startTime,endTime)  
{
    ADM_info("AnnexB to iso filter\n");
    myBitstream=new ADMBitstream(ADM_COPY_FROM_ANNEX_B_SIZE);
    myBitstream->data=buffer;

    myExtra=NULL;
    myExtraLen=0;
    // Read First frame, it contains PPS & SPS
    // Built avc-like atom from it.

    // We need the absolute 1st frame to gety SPS PPS
    // Hopefully it will be ok
    ADMCompressedImage img;
    img.data=buffer;
    img.dataLength=0;

    if(true==video_body->getDirectImageForDebug(0,&img))
    {
        myBitstream->len=img.dataLength;
        NALU_descriptor desc[MAX_NALU_PER_CHUNK];
        int nbNalu=splitNalu(myBitstream->data,myBitstream->data+myBitstream->len,
                                MAX_NALU_PER_CHUNK,desc);
        // search sps
        uint8_t *spsStart,*ppsStart;
        uint32_t spsLen=0, ppsLen=0;
        int indexSps,indexPps;

        indexSps=findNalu(NAL_SPS,nbNalu,desc);
        if(-1==indexSps)
        {
            ADM_error("Cannot find SPS");
        }
        indexPps=findNalu(NAL_PPS,nbNalu,desc);
        if(-1==indexPps)
        {
            ADM_error("Cannot find SPS");
        }
       
        if(indexSps!=-1 && indexPps!=-1)
        {
            spsLen=desc[indexSps].size;
            ppsLen=desc[indexPps].size;
            
            ADM_info("Copy from annexB: Found sps=%d, pps=%d.\n",(int)spsLen,(int)ppsLen);
            // Build extraData
            myExtraLen=5+1+2+spsLen+1+2+ppsLen;
            myExtra=new uint8_t[myExtraLen];
            uint8_t *ptr=myExtra;
            uint8_t *sps=desc[indexSps].start;
            *ptr++=1;           // AVC version
            *ptr++=sps[0];        // Profile
            *ptr++=sps[1];        // Profile
            *ptr++=sps[2];        // Level
            *ptr++=0xff;        // Nal size minus 1

            *ptr++=0xe1;        // SPS
            *ptr++=(1+spsLen)>>8;
            *ptr++=(1+spsLen)&0xff;
            *ptr++=desc[indexSps].nalu;
            memcpy(ptr,desc[indexSps].start,spsLen);
            ptr+=spsLen;

            *ptr++=0x1;         // PPS
            *ptr++=(1+ppsLen)>>8;
            *ptr++=(1+ppsLen)&0xff;
            *ptr++=desc[indexPps].nalu;
            memcpy(ptr,desc[indexPps].start,ppsLen);
            ptr+=ppsLen;

            ADM_info("generated %d bytes of extradata.\n",(int)myExtraLen);
            mixDump(myExtra, myExtraLen);
        }

    }else ADM_warning("Cannot read first frame to get PPS and SPS");
    rewind();

}
/**

    \fn dtor

*/

ADM_videoStreamCopyFromAnnexB::~ADM_videoStreamCopyFromAnnexB()
{
    ADM_info("Destroying AnnexB to iso filtet\n");
    delete myBitstream;
    myBitstream=NULL;
}
/**
    \fn convertFromAnnexB   
    \brief convert annexB startcode (00 00 00 0 xx) to NALU
*/
int ADM_videoStreamCopyFromAnnexB::convertFromAnnexB(uint8_t *inData,uint32_t inSize,
                                                      uint8_t *outData,uint32_t outMaxSize)
{
    uint8_t *tgt=outData;
    NALU_descriptor desc[MAX_NALU_PER_CHUNK];
    int nbNalu=splitNalu(myBitstream->data,myBitstream->data+myBitstream->len,
                        MAX_NALU_PER_CHUNK,desc);
    int nalHeaderSize=4;
    int outputSize=0;


    for(int i=0;i<nbNalu;i++)
    {
        NALU_descriptor *d=desc+i;
        aprintf("%d/%d : Nalu :0x%x size=%d\n",i,nbNalu,d->nalu,d->size);
        switch(d->nalu)
        {
            case NAL_FILLER: break;
            case NAL_AU_DELIMITER: break; 
            default:
                  writeBE32(tgt,1+d->size);
                  tgt[nalHeaderSize]=d->nalu;
                  memcpy(tgt+1+nalHeaderSize,d->start,d->size);
                  tgt+=d->size+1+nalHeaderSize;
                  break;
        }
        outputSize=tgt-outData;
        ADM_assert(outputSize<outMaxSize);
    }
    return outputSize;
}
static void parseNalu(uint8_t *head, uint8_t *tail)
{
    printf("**** Parsing NALU : %d****",(int)(tail-head));
    while(head<tail)
    {
        int32_t size=readBE32(head);
            printf("[%02x] size=%d\n",head[5],size);
        head+=size+4;
    }
}
/**
    \fn getPacket
*/

bool    ADM_videoStreamCopyFromAnnexB::getPacket(ADMBitstream *out)
{
    aprintf("-------%d--------\n",(int)currentFrame);
    if(false==ADM_videoStreamCopy::getPacket(myBitstream)) return false;
    
    int size=convertFromAnnexB(myBitstream->data,myBitstream->len,out->data,out->bufferSize);
    out->len=size;
    out->dts=myBitstream->dts;
    out->pts=myBitstream->pts;
    out->flags=myBitstream->flags;
    //compactNalus(out);
#ifdef ANNEX_B_DEBUG
    parseNalu(out->data,out->data+out->len);
#endif
    return true;
}
/**

*/
bool    ADM_videoStreamCopyFromAnnexB::getExtraData(uint32_t *extraLen, uint8_t **extraData) 
{
    *extraData=myExtra;
    *extraLen=myExtraLen;
    return 0;
}
// EOF
