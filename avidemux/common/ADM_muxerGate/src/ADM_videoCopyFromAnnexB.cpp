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
extern bool ADM_findH264StartCode(uint8_t *start, uint8_t *end,uint8_t *outstartcode,uint32_t *offset);
extern void mixDump(uint8_t *ptr, uint32_t len);
#if 1
#define aprintf ADM_info
#define check isNalValid
#else
#define aprintf(...) {}
#define check(...) {}
#endif

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
    \fn findNalu
    \brief lookup for a specific NALU in the given buffer
*/
bool ADM_videoStreamCopyFromAnnexB::findNalu(int nalu,uint8_t *start,uint8_t *end,
            uint8_t **outPtr,uint32_t *outLen)
{
    uint8_t *realstart=start;
    while(start+5<end) // NALU starts with 4 bytes size + (1 bytes nalu id, size-1 other payload)
    {
        uint32_t len=readBE32(start);
        uint8_t  n=start[4];
        
        if(len+start+4>end)
        {
            ADM_warning("Nal size too big (%d)\n",len);
            return false;
        }
        check(n);
        aprintf("Looking for %x found %x, size=%d at offset 0x%x\n",nalu,n,len,start-realstart);
        if((n&0x1f)==nalu)
        {
            *outPtr=start+5;
            *outLen=len-1;
            return true;
        }
        start+=4+len;
    }
    return false;
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
    if(true==ADM_videoStreamCopy::getPacket(myBitstream))
    {
        uint8_t *tmp=new uint8_t[myBitstream->len*2];
        
        int size=convertFromAnnexB(myBitstream->data,myBitstream->len,tmp+10,myBitstream->len*2-10);
        // search sps
        uint8_t *spsStart,*ppsStart;
        uint32_t spsLen=0, ppsLen=0;

        if(false==findNalu(NAL_SPS,tmp+10,tmp+size-10,&spsStart,&spsLen))
        {
            ADM_error("Cannot find SPS");
            
        }
        if(false==findNalu(NAL_PPS,tmp+10,tmp+size-10,&ppsStart,&ppsLen))
        {
            ADM_error("Cannot find PPS");
            
        }
        if(spsLen && ppsLen)
        {
            ADM_info("Copy from annexB: Found sps=%d, pps=%d.\n",(int)spsLen,(int)ppsLen);
            // Build extraData
            myExtraLen=5+1+2+spsLen+1+2+ppsLen;
            myExtra=new uint8_t[myExtraLen];
            uint8_t *ptr=myExtra;
            *ptr++=1;           // AVC version
            *ptr++=0x64;        // Profile
            *ptr++=0x00;        // Profile
            *ptr++=0x33;        // Level
            *ptr++=0xff;        // Nal size minus 1

            *ptr++=0xe1;        // SPS
            *ptr++=spsLen>>8;
            *ptr++=spsLen&0xff;

            memcpy(ptr,spsStart,spsLen);
            ptr+=spsLen;

            *ptr++=0x1;         // PPS
            *ptr++=ppsLen>>8;
            *ptr++=ppsLen&0xff;

            memcpy(ptr,ppsStart,ppsLen);
            ptr+=ppsLen;

            ADM_info("generated %d bytes of extradata.\n",(int)myExtraLen);
            mixDump(myExtra, myExtraLen);
        }
        delete [] tmp;

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
    \fn unescapeH264
    \brief should be moved to coreUtils
*/
static uint32_t unescapeH264(uint8_t *in, uint32_t len,uint8_t *out)
{
  uint32_t outlen=0;
  uint8_t *start=out;
  uint8_t *tail=in+len;

    if(len<3) 
    {
        memcpy(out,in,len);
        return len;
    }
    while(in+3<tail)
    {
      if(!in[0]  && !in[1] && in[2]==3)
      {
        out[0]=0;
        out[1]=0;
        out+=2;
        in+=3;
        continue;
      }
      *out++=*in++;
    }
    // copy last bytes
    uint32_t left=tail-in;
    memcpy(out,in,left);
    out+=left;
    outlen=out-start;
    ADM_assert(outlen<=len);
    return outlen;
 ;

}
/**
    \fn convertFromAnnexB   
    \brief convert annexB startcode (00 00 00 0 xx) to NALU
*/
int ADM_videoStreamCopyFromAnnexB::convertFromAnnexB(uint8_t *inData,uint32_t inSize,
                                                      uint8_t *outData,uint32_t outMaxSize)
{
    uint8_t *tail=inData+inSize;
    uint8_t *head=inData;
    uint8_t *tgt=outData;
    uint8_t startCode;
    uint32_t offset;
    bool first=true;
    uint8_t oldStartCode;
    while(head<tail)
    {
        if(false==ADM_findH264StartCode(head,tail,&startCode,&offset))
            break;
        check(startCode);
        if(true==first)
        {
            oldStartCode=startCode;
            head+=offset; // First startcode
            first=false;
            continue;
        }
        uint32_t toEscape=offset-5; // ignore startcode
        uint32_t nalHeaderSize=4;
        uint32_t nalSize=unescapeH264(head,toEscape,tgt+nalHeaderSize+1);
        head+=offset;
        writeBE32(tgt,1+nalSize);
        aprintf("%x , %d -> %d at offset 0x%x\n",oldStartCode,toEscape,nalSize,tgt-outData);
        tgt[nalHeaderSize]=oldStartCode;
        tgt+=nalSize+1+nalHeaderSize;
        

        oldStartCode=startCode;
    }
    uint32_t toEscape=tail-head;
    uint32_t nalHeaderSize=4;
    uint32_t nalSize=unescapeH264(head,toEscape,tgt+nalHeaderSize+1);
    writeBE32(tgt,1+nalSize);
    aprintf("%x , %d -> %d at offset 0x%x\n",oldStartCode,toEscape,nalSize,tgt-outData);
    tgt[nalHeaderSize]=oldStartCode;
    tgt+=1+nalSize+nalHeaderSize;
    int outputSize=tgt-outData;
    ADM_assert(outputSize<outMaxSize);
    return outputSize;
}
/**
    \fn compactNalus
    \brief remove unwanted NALU, marge NALU of the same type
*/
bool ADM_videoStreamCopyFromAnnexB::compactNalus(ADMBitstream *out)
{
    uint8_t *start=out->data;
    uint8_t *end=start+out->len;
    uint8_t *realstart=start;
    uint8_t *write=start;
    uint8_t lastNalu=0xff;
    uint32_t originalSize=out->len;
    while(start+5<end) // NALU starts with 4 bytes size + (1 bytes nalu id, size-1 other payload)
    {
        uint32_t len=readBE32(start);
        uint8_t  n=start[4];
        if(len+start+4>end)
        {
            ADM_warning("Nal size too big (4+%d/%d)\n",len,end-start);
            return false;
        }
        check(n);
        aprintf("Compacting: Nal %d, size %d\n",n,len);
        switch(n&0x1f)
        {
            case NAL_FILLER: break;
            case NAL_AU_DELIMITER: break;
            default: // keep it
                memmove(write,start,len+4);
                write+=4+len;
                break;
        }
        start+=4+len;
    }
    out->len=write-realstart;
    aprintf("compact %d -> %d\n",originalSize,out->len);
    return true;
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
    compactNalus(out);
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
