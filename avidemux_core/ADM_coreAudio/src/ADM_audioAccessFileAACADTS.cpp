/**
    \file ADM_audioAccessFileAACADTS
    \brief Source is a AAC audio file, wrapped in ADTS container

*/
#include "ADM_default.h"
#include "ADM_audioStream.h"
#include "ADM_audioAccessFileAACADTS.h"
#include "ADM_vidMisc.h"

#define AAC_SEEK_PERIOD (10000000LL) // 10 sec

/**
 * 
 * @param fd
 */
class adtsIndexer
{
public:
    
            adtsIndexer(FILE *fd,int fq,int chan)
            {
                f=fd;
                this->fq=fq;
                this->channels=chan;
                payload=0;
            }
            int getPayloadSize() {return payload;}
    virtual ~adtsIndexer()
            {
                
            }
    bool    index(std::vector<aacAdtsSeek> &seekPoints);    
    
protected:
    FILE *f;
    int  fq;
    int  channels;
    int  payload;
    
};
/**
 * 
 * @param seekPoints
 * @return 
 */
bool adtsIndexer::index(std::vector<aacAdtsSeek>& seekPoints)
{
   uint8_t  buffer[5*1024];
   int      fileOffset=0;;
   uint64_t lastPoint=0;
   int len;
   audioClock clk(fq);
   ADM_adts2aac aac;
   aacAdtsSeek start;
   start.dts=0;start.position=0;
    seekPoints.push_back(start);
   
   while(1)
   {
       ADM_adts2aac::ADTS_STATE s=aac.getAACFrame(&len,buffer);
       switch(s)
       {
           case ADM_adts2aac::ADTS_OK:
            {
                payload+=len;
                clk.advanceBySample(1024);
                continue;
                break;
            }
           case ADM_adts2aac::ADTS_ERROR:
           {
               return true;
               break;
           }
           case ADM_adts2aac::ADTS_MORE_DATA_NEEDED:
           {
               uint64_t currentPoint=clk.getTimeUs();
               if( (currentPoint-lastPoint)>AAC_SEEK_PERIOD) // one seek point every 10 s
               {
                   start.dts=currentPoint; // we have an error of 1 block ~ 1024 samples ~ 10 ms
                   start.position=fileOffset;
                   seekPoints.push_back(start);
                   lastPoint=currentPoint;   
               }
               int n=fread(buffer,1,5*1024,f);
               if(n<=0)
                   return true;
               fileOffset+=n;
               aac.addData(n,buffer);
               break;
           }
           default:
               ADM_assert(0);
               break;
       }
    
   }
   return true; 
}

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
    adtsIndexer dexer(_fd,headerInfo.frequency,headerInfo.channels);
    ADM_info("Indexing adts/aac file\n");
    dexer.index(seekPoints );  
    ADM_info("found %d seekPoints\n",seekPoints.size());
    fseek(_fd,0,SEEK_SET);
    payloadSize=dexer.getPayloadSize();
    // 
    double nbBlock=seekPoints.size()+1;
    nbBlock*=AAC_SEEK_PERIOD; // each seekpoint is 10 sec
    durationUs=nbBlock;
    ADM_info("AAC total duration %s\n",ADM_us2plain(durationUs));
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
    bool keepGoing;
    int  outSize;
    ADM_adts2aac::ADTS_STATE state;
    
    do{
        keepGoing=false;
        state=aac->getAACFrame(&outSize,buffer);
        switch(state)
        {
            case ADM_adts2aac::ADTS_MORE_DATA_NEEDED: keepGoing=refill();break;
            case ADM_adts2aac::ADTS_ERROR: inited=false;ADM_warning("AAC/ADTS parser gone to error\n");break;
            case ADM_adts2aac::ADTS_OK: break;
            default: ADM_assert(0); break;
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
    int s=n-1;
    for(int i=0;i<n-1;i++)
    {
        if(seekPoints[i+1].dts>timeUs)
        {
            s=i;
            break;
        }
    }
    ADM_info("AAC/ADTS seek to %s requested ",ADM_us2plain(timeUs));
    ADM_info(" done at index %d,  %s requested ",s,ADM_us2plain(seekPoints[s].dts));
    clock->setTimeUs(seekPoints[s].dts);
    fseek(_fd,seekPoints[s].position,SEEK_SET);     // no seek ATM
    aac->reset();
    return true;
}

// EOF

