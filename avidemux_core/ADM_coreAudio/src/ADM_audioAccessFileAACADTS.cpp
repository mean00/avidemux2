/**
    \file ADM_audioAccessFileAACADTS
    \brief Source is a AAC audio file, wrapped in ADTS container

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_audioStream.h"
#include "ADM_audioAccessFileAACADTS.h"
#include "ADM_vidMisc.h"
#include "ADM_audioAccessFileAACADTS_indexer.cpp"

/**
 * 
 * @return 
 */
bool ADM_audioAccessFileAACADTS::init(void)
{
    aac=new  ADM_adts2aac();    
    // ----- extract data frequency etc.. to initialize clock
#define PROBE_SIZE 8000
    uint8_t buffer[PROBE_SIZE+1];
    int n=fread(buffer,1,PROBE_SIZE,_fd);
    if(n<=0) return false;
    fseek(_fd,0,SEEK_SET);
    ADM_info("Probing AAC/ADTS with %d bytes\n",n);
    aac->addData(n,buffer);
    if(ADM_adts2aac::ADTS_OK!=aac->getAACFrame(NULL,NULL))
    {
        ADM_warning("Cannot sync\n");
        return false;
    }
    // ----- Get extraData
    uint8_t *p=NULL;
    aac->getExtraData(&extraDataLen,&p);
    if(extraDataLen)
    {
        extraData=new uint8_t[extraDataLen];
        memcpy(extraData,p,extraDataLen);
    }
    // -----  fillup header
    headerInfo.encoding=WAV_AAC;
    headerInfo.frequency=aac->getFrequency();
    headerInfo.channels=aac->getChannels();
    headerInfo.bitspersample=16;
    headerInfo.blockalign=0;
    
    
    aac->reset();
    
    clock= new audioClock(headerInfo.frequency);
    
    // ----- build time map
    fseek(_fd,0,SEEK_SET);
    adtsIndexer dexer(_fd,headerInfo.frequency,headerInfo.channels);
    ADM_info("Indexing adts/aac file\n");
    dexer.index(seekPoints );  
    ADM_info("found %d seekPoints\n",seekPoints.size());
    fseek(_fd,0,SEEK_SET);
    payloadSize=dexer.getPayloadSize();
    // 
    // compute duration
    audioClock ck(headerInfo.frequency);
    ck.advanceBySample(1024*dexer.getNbPackets());
    durationUs=ck.getTimeUs();
    double byteRate=dexer.getPayloadSize();
    byteRate=byteRate/(1+durationUs);
    headerInfo.byterate=byteRate*1000000.; // b/us -> b/ss

    ADM_info("AAC total duration %s\n",ADM_us2plain(durationUs));
    ADM_info("# of packets found : %d\n",dexer.getNbPackets());
    ADM_info("Byterate : %d\n",headerInfo.byterate);
    return true;
}

/**
    \fn
    \brief
*/

ADM_audioAccessFileAACADTS::ADM_audioAccessFileAACADTS(const char *fileName,int offset)
{
        fileSize=ADM_fileSize(fileName)-offset;
        _fd=ADM_fopen(fileName,"rb");
        ADM_assert(_fd);
        clock=NULL;
        inited=init();;
        
        
}
/**
    \fn
    \brief
*/

ADM_audioAccessFileAACADTS::~ADM_audioAccessFileAACADTS()
{
        if(_fd) ADM_fclose(_fd);
        _fd=NULL;
        if(clock) delete clock;
        clock=NULL;
        if(aac) delete aac;
        aac=NULL;
}
/**
 * 
 * @param buffer
 * @param size
 * @param maxSize
 * @param dts
 * @return 
 */
bool ADM_audioAccessFileAACADTS::refill(void)
{
    // Read in temp buffer
    uint8_t tmp[4024];
    int n=fread(tmp,1,4024,_fd);
    if(n<1) return false;
    // feed
    aac->addData(n,tmp);
    return true;
}
/**
    \fn
    \brief
*/

bool    ADM_audioAccessFileAACADTS::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
    if(!inited) return false;
    // Search sync
    bool keepGoing=false;
    int  outSize;
    ADM_adts2aac::ADTS_STATE state;
    
    do{
        state=aac->getAACFrame(&outSize,buffer);
        switch(state)
        {
            case ADM_adts2aac::ADTS_MORE_DATA_NEEDED: 
                    keepGoing=refill();
                    break;
            case ADM_adts2aac::ADTS_ERROR: 
                    inited=false;
                    keepGoing=false;
                    ADM_warning("AAC/ADTS parser gone to error\n");
                    break;
            case ADM_adts2aac::ADTS_OK: 
                    keepGoing=false;
                    break;
            default: 
                    ADM_assert(0); 
                    break;
        }
    }while(keepGoing);
    
    if(state!=ADM_adts2aac::ADTS_OK)
    {
        ADM_warning("AAC/ADTS : Cannot get packet\n");
        return false;
    }
    // Now do it
       //ADTS_STATE convert2(int incomingLen,const uint8_t *intData,int *outLen,uint8_t *out);
    *size=outSize;
    ADM_assert(outSize<maxSize);
    *dts=clock->getTimeUs();
    printf("Time = %s\n",ADM_us2plain(*dts));
    clock->advanceBySample(1024);
    return true;         
}
/**
 * 
 * @param timeUs
 * \brief We can be off by one frame (~ 10 ms)
 * @return 
 */
bool      ADM_audioAccessFileAACADTS::goToTime(uint64_t timeUs)
{
    if(!inited)
        return false;

    // Search for the seek point just before timeUS
    int n=seekPoints.size();
    if(!n) return false;
    int s=n-1;
    for(int i=0;i<n-1;i++)
    {
        if(seekPoints[i+1].dts>timeUs)
        {
            s=i;
            break;
        }
    }
    aacAdtsSeek seek=seekPoints[s];
    ADM_info("AAC/ADTS seek to %s requested \n",ADM_us2plain(timeUs));
    ADM_info(" done at index %d,  %s requested \n",s,ADM_us2plain(seek.dts));
    clock->setTimeUs(seek.dts);
    fseek(_fd,seek.position,SEEK_SET);
    aac->reset();
    return true;
}

// EOF

