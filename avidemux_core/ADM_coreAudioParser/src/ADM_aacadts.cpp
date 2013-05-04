/***************************************************************************
  \file ADM_aacadts.cpp
  \brief wrapper around libavcodec bitstream filter
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>
#include "ADM_default.h"
#include "ADM_aacadts.h"

#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif

static const int aacChannels[16]=
{
0, //0: Defined in AOT Specifc Config
1, //1: 1 channel: front-center
2, //2: 2 channels: front-left, front-right
3, //3: 3 channels: front-center, front-left, front-right
4, //4: 4 channels: front-center, front-left, front-right, back-center
5, // 5: 5 channels: front-center, front-left, front-right, back-left, back-right
6, // 6: 6 channels: front-center, front-left, front-right, back-left, back-right, LFE-channel
8, // 7: 8 channels: front-center, front-left, front-right, side-left, side-right, back-left, back-right, LFE-channel
0,0,0,0,
0,0,0,0,
};

static 	uint32_t aacSampleRate[16]=
{
	96000, 88200, 64000, 48000,
	44100, 32000, 24000, 22050,
	16000, 12000, 11025,  8000,
	0,     0,     0,     0 
};
/**
    \fn getExtraData
    \brief extract extradata from adts/aac stream. You must have converted successfully at leat one frame.
*/
bool ADM_adts2aac::getExtraData(uint32_t *len,uint8_t **data)
{
    if(hasExtra)
    {
        *data=extra;
        *len=2;
        return true;
    }
    return false;
}
/**
    \fn getFrequency
    \brief get stream frequency. Convert must have been called ok once.
*/
int ADM_adts2aac::getFrequency(void)
{
    if(!hasExtra)
    {
            ADM_error("No extradata in aac! using default of 48 kHz");
            return 48000;
    }
    uint8_t *p=extra;
    int dex=((p[0]&7)<<1)+(p[1]>>7);
    return aacSampleRate[dex];
}
/**
    \fn getChannels 
    \brief returns # of channels. Convert must have been called ok once.
*/
int ADM_adts2aac::getChannels(void)
{
    if(!hasExtra)
    {
            ADM_error("No extradata in aac! using default of 2 channels");
            return 2;
    }
    uint8_t *p=extra;
    int dex=((p[1]>>3)&0xf);
    return aacChannels[dex];
}

/**
    \fn convert
    \brief strip adts header. Out can be null if you just want to get headers
*/

bool ADM_adts2aac::convert(int incomingLen,uint8_t *inData,int *outLen,uint8_t *out)
{
    bool r=false;
    // Step 1 : append to our buffer...
    *outLen=0;
    if(head==tail)
    {
        head=tail=0;
    }
    if(tail>ADTS_BUFFER_SIZE)
    {
        int size=head-tail;
        memmove(buffer.at(0),buffer.at(tail),size);
        head=size;
        tail=0;
    }
    if(head+incomingLen>ADTS_BUFFER_SIZE*2)
    {
        ADM_error("Head=%d tail=%d bufferSize=%d\n",head,tail,ADTS_BUFFER_SIZE*2);
        ADM_error("Adts buffer overflow\n");
        ADM_assert(0);
    }
    memcpy(buffer.at(head),inData,incomingLen);
    head+=incomingLen;
    // ok , done
    aprintf("********** LOOP *******\n");
again:
    aprintf("*** head=%d tail=%d size=%d***\n",head,tail,head-tail);
    if(tail+7>head) // we neeed at least 7 bytes...
    {
        aprintf("Adts: Not enough data \n");
        return r;
    }
    // now search for sync....
    bool found=false;
    uint8_t *p=NULL;
    int packetLen=0;
    int nbFrames=0;
    bool crc=false;
    int match;
    for( p=(buffer.at(tail));p<(buffer.at(head-2));p++)
    {
        if(p[0]==0xff && ((p[1]&0xf0)==0xf0))
        {
            match=p-buffer.at(0); // offset of syncword
            packetLen=((p[3]&0x3)<<11)+(p[4]<<3)+(p[5]>>5);
            nbFrames=1+(p[6]&3);
            if(!p[1]&1)
            {
                crc=true;
            }
            if(nbFrames!=1) continue;
            aprintf("Packet len=%d, nbframes=%d\n",packetLen,nbFrames);
            aprintf("Found sync at offset %d, buffer size=%d\n",(int)(p-buffer.at(0)),(int)(head-tail));
            aprintf("Dropping %d bytes\n",(int)(p-buffer.at(0)-tail));
            if(match==tail && match+packetLen==head)
            {
                aprintf("Perfect match\n");
                found=true;
                break;
            }
            if(match+packetLen+2>head && match+packetLen!=head)
            {
                aprintf("** not enough data **\n");
                return r;
            }
            // do we have sync at the end ?
            if(p[packetLen]!=0xff)
            {
                aprintf("no ff marker at the end\n");
                continue;
            }
            aprintf("End marker found\n");
            found=true;
            break;
        }
    }
    if(found==false)
    {
        aprintf("No sync\n");
        tail=head-1;
        return r;
    }
    if(!hasExtra)
    { // build codec specific info, thanks vlc  
        int i_profile=p[2]>>6;
        int i_sample_rate_idx=(p[2]>>2)&0x0f;
        int pi_channels=((p[2]<<2)+((p[3]>>6)))&0x7;
      extra[0] =   (i_profile + 1) << 3 | (i_sample_rate_idx >> 1);
      extra[1] =   ((i_sample_rate_idx & 0x01) << 7) | (pi_channels <<3);
      hasExtra=true;
    }
    // size ?
    uint8_t *o;
    int produced=0;
    // filter!
    //-------

        if(crc) 
            {
                o=p+9;
                produced=packetLen-9;
            }else
            {
                o=p+7;
                produced=packetLen-7;
            }
    //
    //---------
    if(!produced)
    {
        tail=match+1;
        printf("No data produced\n");
        goto again;
    }
    aprintf("Found adts packet of size %d, extradataLen=%d\n",produced,2);
    if(out)
    {
        memcpy(out,o,produced);
        out+=produced;
        *outLen+=produced;
    }
    r=true; // we have found something
    //printf("In : %d out %d\n",incomingLen,*outLen);
    tail=match+packetLen; // ~ skip 
    ADM_assert(tail<=head);
    goto again;
}
/**
    \fn ctor
*/

ADM_adts2aac::ADM_adts2aac(void)
{
    hasExtra=false;
    extra[0]=extra[1]=0;
    head=tail=0;
    buffer.setSize(ADTS_BUFFER_SIZE*2);
}
/**
    \fn dtor
*/

ADM_adts2aac::~ADM_adts2aac()
{
   
}

//EOF