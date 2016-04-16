/**
    \file ADM_audioAccessFileAACADTS
    \brief Source is a AAC audio file, wrapped in ADTS container

*/
#include "ADM_default.h"
#include "ADM_audioStream.h"
#include "ADM_audioAccessFileAACADTS.h"

/**
 * 
 * @return 
 */
bool ADM_audioAccessFileAACADTS::init(void)
{
    aac=new  ADM_adts2aac();    
    // extract data frequency etc.. to initialize clock
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
    // Get extraData
    uint8_t *p=NULL;
    aac->getExtraData(&extraDataLen,&p);
    if(extraDataLen)
    {
        extraData=new uint8_t[extraDataLen];
        memcpy(extraData,p,extraDataLen);
    }
    // fillup header
    headerInfo.encoding=WAV_AAC;
    headerInfo.frequency=aac->getFrequency();
    headerInfo.channels=aac->getChannels();
    headerInfo.bitspersample=16;
    headerInfo.blockalign=0;
    aac->reset();
    
    clock= new audioClock(headerInfo.frequency);
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
    uint8_t tmp[1024];
    int n=fread(tmp,1,1024,_fd);
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
    bool keepGoing=true;
    ADM_adts2aac::ADTS_STATE state;
    while(keepGoing)
    {
        keepGoing=false;
        state=aac->getAACFrame(0,NULL);
        switch(state)
        {
            case ADM_adts2aac::ADTS_MORE_DATA_NEEDED: keepGoing=refill();break;
            case ADM_adts2aac::ADTS_ERROR: inited=false;ADM_warning("AAC/ADTS parser gone to error\n");break;
            case ADM_adts2aac::ADTS_OK: break;
            default: ADM_assert(0); break;
        }
    }
    if(state!=ADM_adts2aac::ADTS_OK)
    {
        ADM_warning("AAC/ADTS : Cannot get packet\n");
        return false;
    }
    // Now do it
       //ADTS_STATE convert2(int incomingLen,const uint8_t *intData,int *outLen,uint8_t *out);
    int outSize;
    state=aac->getAACFrame(&outSize,buffer);
    ADM_assert(ADM_adts2aac::ADTS_OK==state);
    *size=outSize;
    ADM_assert(outSize<maxSize);
    *dts=clock->getTimeUs();
    clock->advanceBySample(1024);
    return true;
          
}
/**
 * 
 * @param timeUs
 * @return 
 */
bool      ADM_audioAccessFileAACADTS::goToTime(uint64_t timeUs)
{
    if(!inited)
        return false;
    aac->reset();
    clock->setTimeUs(0);
    fseek(_fd,0,SEEK_SET);     // no seek ATM
    return true;
}

// EOF

