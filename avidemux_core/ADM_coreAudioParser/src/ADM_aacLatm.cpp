/***************************************************************************
  \file ADM_aacLatm.cpp
  \brief  Extract aac packet from LOAS/LATM stream
     Derived from vlc code, seel ADM_aacLatm.h for vlc (c)
 * 
 * http://www.nhzjj.com/asp/admin/editor/newsfile/2010318163752818.pdf
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
extern "C" {
#include "libavcodec/avcodec.h"
}
#include "ADM_aacLatm.h"

#define COOKIE   ((AVBitStreamFilterContext *)cookie)
#define CONTEXT  ((AVCodecContext *)codec)

#if 0
#define xdebug ADM_info
#else
#define xdebug(...) {}
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
bool ADM_latm2aac::getExtraData(uint32_t *len,uint8_t **data)
{
    *data=extraData;
    *len=extraLen;
    return true;
}
/**
    \fn getFrequency
    \brief get stream frequency. Convert must have been called ok once.
*/
int ADM_latm2aac::getFrequency(void)
{
        return fq;
}
/**
    \fn getChannels 
    \brief returns # of channels. Convert must have been called ok once.
*/
int ADM_latm2aac::getChannels(void)
{       
        return channels;
}
/**

*/
static int  LatmGetValue( getBits &bits )
{
    int i_bytes = bits.get( 2 );
    int v = 0;
    int i;
    for( i = 0; i < i_bytes; i++ )
        v = (v << 8) + bits.get(8);
    return v;
}
static int read31plus(getBits &bits)
{
    int type=bits.get(5);
    if(type==31)
        type=32+bits.get(6);
    return type;
}
/**
    \fn AudioSpecificConfig
*/
bool ADM_latm2aac::AudioSpecificConfig(getBits &bits,int &bitsConsumed)
{
    int consumed=bits.getConsumedBits();
    getBits myBits(bits); // get a copy, needed to extract extra data


    int audioObjectType=read31plus(bits);
    int samplingFrequencyIndex=bits.get(4);

    if(samplingFrequencyIndex==0xf)
    {
            fq=(bits.get(8)<<16)+bits.get(16);
    }else
    {
        fq=aacSampleRate[samplingFrequencyIndex];
    }
    int channelConfiguration=bits.get(4);
    channels=aacChannels[channelConfiguration];
    xdebug("Fq=%d, channel=%d\n",fq,channelConfiguration);
    xdebug("ObjectType=%d\n",audioObjectType);
    
    if(audioObjectType==5) // SBR
    {
               int extendedSamplingFrequencyIndex=bits.get(4);
               xdebug("SBR audio freq=%d\n",aacSampleRate[extendedSamplingFrequencyIndex]);
               
               audioObjectType=read31plus(bits); // 5 bits
               xdebug("New object type=%d\n",audioObjectType);        
    }
    //17
    switch(audioObjectType)
    {
        case 2: // GASpecificConfig
                {
                    int frameLengthFlags=bits.get(1);	// frameLength
                    bool dependsOnCoreCoder=bits.get(1);
                    xdebug("FrameLengthFlags=%d\n",frameLengthFlags);
                    xdebug("dependsOnCoreCoder=%d\n",dependsOnCoreCoder);
                    if(dependsOnCoreCoder) bits.skip(14); // coreCoderDelay
                    bool extensionFlag=bits.get(1); //23
                    if(!channelConfiguration)
                    {
                        ADM_error("No channel configuraiton\n");
                        return false;
                    }
                    if(extensionFlag)
                    {
                        ADM_warning("Extension flag\n");
                        //bits.get(1);	// extensionFlag3
                        return false;
                    }
                }
                break;
        default:
                ADM_error("AudioObjecttype =%d not handled\n",audioObjectType);
                return false;
    }
    consumed=bits.getConsumedBits()-consumed;
    bitsConsumed=consumed;
    xdebug("%d bits consumed\n",consumed);
    extraLen=(consumed+7)/8;
#if 0 // pad left
    // fill to get 8 bits..
    int top=consumed&7;
    if(!top) top=8;
    extraData[0]=myBits.get(top);
    for(int i=1;i<extraLen;i++)
    {
            extraData[i]=myBits.get(8);
    }
#else // pad right
    for(int i=0;i<extraLen;i++)
    {
            int rd=consumed;
            if(rd>8) rd=8;
            extraData[i]=(myBits.get(rd))<<(8-rd);
            consumed-=rd;
    }

#endif
    
    xdebug("Got %d extraData \n",extraLen);
    for(int i=0;i<extraLen;i++)
        xdebug(" %02x",extraData[i]);
    xdebug(" \nFrequency %d, channels %d\n",fq,channels);
    conf.gotConfig=true;
    return true;

}
/**
    \fn readPayloadInfoLength
*/
int ADM_latm2aac::readPayloadInfoLength(getBits &bits)
{
    if(conf.allStreamsSameTimeFraming)
    {
        // handle layer..., only one accepted ATM
        if(conf.frameLengthType[0]==0)
        {
            int l=0;
            while(1)
            {
                int tmp=bits.get(8);
                l+=tmp;
                if(tmp!=255) break;
            }
            xdebug("Payload Len=%d\n",l);
            return l;
        }
    }else
    {
        ADM_error("cannot handle allStreamSameTimeFraming==0\n");
        
    }
    return 0;
}
/**
    \fn readPayload
*/
bool ADM_latm2aac::readPayload(getBits &bits, uint64_t dts,int size)
{
    if(conf.allStreamsSameTimeFraming)
    {
            xdebug("Payload %d \n",size);
            if(size>LATM_MAX_BUFFER_SIZE)
            {
                    ADM_warning("Packet too big %d vs %d\n",size,LATM_MAX_BUFFER_SIZE);
                    return false;
            }
        
            // try to get a buffer...
            if(listOfFreeBuffers.isEmpty())
            {
                    ADM_error("No free buffer!\n");
                    return false;
            }
            latmBuffer *b=listOfFreeBuffers.popBack();
            b->dts=dts;
            for(int i=0;i<size;i++)
            {                
                    b->buffer[i]=bits.get(8);
            }
            b->bufferLen=size;
            if(!conf.gotConfig)
                listOfFreeBuffers.pushBack(b);
            else
                listOfUsedBuffers.pushBack(b);
            return true;
    }else
    {

        ADM_error("cannot handle allStreamSameTimeFraming==0\n");
        return false;
    }
}
/**
    \fn readStreamMuxConfig
*/
bool ADM_latm2aac::readStreamMuxConfig(getBits &bits)
{
 // streamMuxConfig
        conf.audioMuxVersion = bits.get(1);
        if(conf.audioMuxVersion==1)
            conf.audioMuxVersionA = bits.get(1);
        if( conf.audioMuxVersionA != 0 ) /* support only A=0 */
        {
            ADM_warning("LATM : versionA!=0, not supported\n");
            return false;
        }
    
        if( conf.audioMuxVersion == 1 )
        {
            LatmGetValue(bits); /* taraBufferFullness */
        }

        conf.allStreamsSameTimeFraming=bits.get(1);
        xdebug("AllSTreamSameTimeFraming =%d\n",conf.allStreamsSameTimeFraming);
        int numSubFrames=bits.get(6)+1;
        int numbSubProgram=1+bits.get(4);

        xdebug("NumSubFrame=%d, numSubProgram=%d\n",numSubFrames,numbSubProgram);

        if(numSubFrames!=1 || numbSubProgram!=1  )
        {
            ADM_warning("LATM: only supports subframe=1, subprogram=1\n");
            return false;
        }

        conf.nbLayers=1+bits.get(3);
        xdebug("NumLayer=%d\n",conf.nbLayers);
    
        for(int i=0;i<conf.nbLayers;i++)
        {
            bool useSameConfig=false;
            
            if(i)
                useSameConfig=bits.get(1);
            if(!useSameConfig)
            {
				int consumed=0;

                if(!conf.audioMuxVersion)
                {  // audio specific config
                    if(false==AudioSpecificConfig(bits,consumed))
                    {
                        ADM_warning("Error reading audioSpecificConfig\n");
                        return false;
                    }

                }else
                {
                    int ascLen=LatmGetValue(bits);

                    //ascLen-=audioSpecicConfig
                    //fillBits(ascLen)
                    if(false==AudioSpecificConfig(bits,consumed))
                    {
                        ADM_warning("Error reading audioSpecificConfig\n");
                        return false;
                    }
                    if(consumed>ascLen)
                    {
                        ADM_warning("Too much bits consumed in AudioSpecificConfig (%d/%d)\n",consumed,ascLen);
                        return false;                        
                    }
                    ascLen-=consumed;
                    while(ascLen)
                    {
                        int r=ascLen;
                        if(r>16) r=16;
                        bits.skip(r);
                        ascLen-=r;
                    }
                }
            }
            // frameLengthType
            conf.frameLengthType[i]=bits.get(3);
            xdebug("FrameLengthType=%d\n",conf.frameLengthType[i]);
            if(conf.frameLengthType[i]!=0)
            {
                ADM_error("frameLengthType!=0 not supported (%d)\n",conf.frameLengthType[i]);
                return false;
            }

            bits.get(8);	// latmBufferFulless

            int otherData=bits.get(1);
            
            if(otherData)
            {
				int otherDataLen=0;

                if(conf.audioMuxVersion==1)
                    otherDataLen=LatmGetValue(bits);
                else
                {
                    while(1)
                    {
                        int esc=bits.get(1);
                        int data=bits.get(8);
                        otherDataLen=(otherDataLen<<8)+data;
                        if(!esc) break;
                    }
                }
                    
            }
            int crc=bits.get(1);
            if(crc) bits.get(8);
        }
        return true;
}
/**
    \fn readStreamMux
    \brief from vlc
*/
bool  ADM_latm2aac::readAudioMux( uint64_t dts,getBits &bits )
{

    if( !bits.get(1) ) // use SameStreamMux
    {
       if(false==readStreamMuxConfig(bits)) 
       {
           return false;
       }
    } // streamMuxConfig
//    if(!numSubFrames) return false;
    if(conf.audioMuxVersionA==0)
    {
       // only 1 subFrames ATM... for(int i=0;i<numSubFrames;i++)
        {
            int len=readPayloadInfoLength(bits);
            if(!len) return false;
            
            bool r=readPayload(bits,dts,len);
            bits.align();
            return r;
        }
        // otherdata
    }
    return true;
}



/**
    \fn demuxLatm
    \brief extract extrdata + aac frame from LATM
*/
bool ADM_latm2aac::demuxLatm(uint64_t date,uint8_t *start,uint32_t size)
{
    getBits  bits(size,start);
    return readAudioMux(date,bits);
}
/**
    \fn pushData
    \brief Check for LOAS sync word, extract LATM frames
*/

bool ADM_latm2aac::pushData(int incomingLen,uint8_t *inData,uint64_t dts)
{
    // Lookup sync
    uint8_t *end=inData+incomingLen;
    uint8_t *start=inData;
    xdebug("Pushing data %d bytes\n",incomingLen);
    while(start<end)
    {
        int key=(start[0]<<8)+start[1];
        if((key & 0xffe0)!=0x56e0)  // 0x2b7 shifted by one bit
        {
            ADM_warning("Sync lost\n");
            return true;
        }
        uint32_t len=start[2]+((key & 0x1f)<<8);
        start+=3;
        if(start+len>end)
        {
            ADM_warning("Not enough data, need %d, got %d\n",len,(int)(end-start));
            return true;
        }
        xdebug("Found LATM : size %d\n",len);
        demuxLatm(dts,start,len);
        dts=ADM_NO_PTS;
        // LATM demux
        start+=len;
    }
    xdebug("-- end of this LATM frame --\n");
    return true;
}
/**
    \fn ctor
*/

ADM_latm2aac::ADM_latm2aac(void)
{
                fq=0;
                channels=0;
                extraLen=0;
                memset(&conf,0,sizeof(conf));
                conf.gotConfig=false;
                for(int i=0;i<LATM_NB_BUFFERS;i++)
                    listOfFreeBuffers.pushBack(&(buffers[i]));
}
/**
    \fn dtor
*/

ADM_latm2aac::~ADM_latm2aac()
{
}

/**
   \fn empty
   \brief returns true if output packet queue is empty
*/
bool ADM_latm2aac::empty()
{
    if(listOfUsedBuffers.isEmpty()) return true;
    return false;
}

/**
    \fn flush
    \brief flush packet queue. Must be called when seeking
*/
bool ADM_latm2aac::flush()
{
   listOfFreeBuffers.clear();
   listOfUsedBuffers.clear();
   for(int i=0;i<LATM_NB_BUFFERS;i++)
                    listOfFreeBuffers.pushBack(&(buffers[i]));
   return true;
}
/**
    \fn getData
    \brief pop one packet from packet queue
*/
bool ADM_latm2aac::getData(uint64_t *time,uint32_t *len, uint8_t *data, uint32_t maxSize)
{
    if(empty()) return false;
//    xdebug("%d slogs in latm buffers\n",listOfUsedBuffers.size());
    latmBuffer *b=listOfUsedBuffers.pop();
    listOfFreeBuffers.pushBack(b);
    if(b->bufferLen>maxSize)
    {
        ADM_warning("Buffer too small\n");
        return false;

    }
    memcpy(data,b->buffer.at(0),b->bufferLen);
    *len=b->bufferLen;
    b->bufferLen=0;
    *time=b->dts;
    xdebug("   read %d bytes\n",*len);
    return true;
}
//EOF
