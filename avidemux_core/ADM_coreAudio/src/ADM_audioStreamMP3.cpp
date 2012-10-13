/**
    \file ADM_audioStream
    \brief Base class

(C) Mean 2008
GPL-v2
*/
#include "ADM_default.h"
#include "ADM_audioStreamMP3.h"
#include "ADM_mp3info.h"
#include "DIA_working.h"
#include "ADM_clock.h"

#if 1 
#define aprintf(...) {}
#else
#define aprintf printf
#endif
/**
    \fn ADM_audioStreamMP3
    \brief constructor
*/
ADM_audioStreamMP3::ADM_audioStreamMP3(WAVHeader *header,ADM_audioAccess *access,bool createMap) : ADM_audioStreamBuffered(header,access)
{
    // If hinted..., compute the duration ourselves
    if(access->isCBR()==true && access->canSeekOffset()==true)
    {
        // We can compute the duration from the length
        float size=access->getLength();
              size/=header->byterate; // Result is in second
              size*=1000;
              size*=1000; // s->us
              durationInUs=(uint64_t)size;
              return;
    }
    // and built vbr map if needed
    // The 2 conditions below means there is no gap i.e. avi style stream
    // else not needed
    if( access->canSeekTime()==false)
    {
        ADM_assert(access->canSeekOffset()==true);
        if(true==createMap)
        {
            buildTimeMap();
            return;
        }
    }
    // Time based
    durationInUs=access->getDurationInUs();
    

}

/**
    \fn ADM_audioStream
    \brief destructor
*/
ADM_audioStreamMP3::~ADM_audioStreamMP3()
{
    // Delete our map if needed...
   for(int i=0;i<seekPoints.size();i++)
   {
        delete seekPoints[i];
        seekPoints[i]=NULL;
    }
}
/**
    \fn goToTime
    \brief goToTime
*/
bool         ADM_audioStreamMP3::goToTime(uint64_t nbUs)
{
    if(access->canSeekTime()==true)
    {
        if( access->goToTime(nbUs)==true)
        {
           setDts(nbUs);
           limit=start=0;
           refill();
           return 1;
        }
        return 1;
    }
    // If CBR we can use the default way
    if(access->isCBR()==true)
        return ADM_audioStream::goToTime(nbUs);
    // if VBR use our time map
    if(!seekPoints.size())
    {
        ADM_error("VBR MP2/MP3 stream with no time map, cannot seek");
        return false;
    }
    // Search the switching point..
    for(int i=0;i<seekPoints.size()-1;i++)
    {
        //printf("[%d]Target %u * %u * %u *\n",i,nbUs,seekPoints[i]->timeStamp,seekPoints[i+1]->timeStamp);
        if(seekPoints[i]->timeStamp<=nbUs && seekPoints[i+1]->timeStamp>=nbUs)
        {
            start=limit=0;
            access->setPos(seekPoints[i]->offset);
            setDts(seekPoints[i]->timeStamp);
            return true;
        }
    }
    return false;
}
/**
        \fn getPacket
*/
uint8_t ADM_audioStreamMP3::getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
{
#define ADM_LOOK_AHEAD 4 // Need 4 bytes...
uint8_t data[ADM_LOOK_AHEAD];
MpegAudioInfo info;
uint32_t offset;
    while(1)
    {
        // Do we have enough ? Refill if needed ?
        if(needBytes(ADM_LOOK_AHEAD)==false) return 0;
        // Peek
        peek(ADM_LOOK_AHEAD,data);
        if(getMpegFrameInfo(data,ADM_LOOK_AHEAD, &info,NULL,&offset))
        {
            ADM_assert(info.size<=sizeMax);
            if(needBytes(info.size)==true)
            {
                *size=info.size;
                read(*size,buffer);
                *nbSample=info.samples;
                //if(info.samples!=1152) ADM_assert(0);
                *dts=lastDts;
            //    printf("MP3 DTS =%"PRId64" ->",*dts);
                advanceDtsBySample(*nbSample);
                //printf("%"PRId64" , size=%d\n",*dts,*size);
                return 1;
            }
            
        }
        //discard one byte
        printf("[MP3 Stream] Syncing...\n");
        read8();
    }
}
 /**
      \fn buildTimeMap
     \brief compute a map between time<->Offset. It is only used for stream in Offset mode with no gap (avi like)
            In that case, the incoming dts is irrelevant.
            We may have up to one packet error
  
  */
#define TIME_BETWEEN_UPDATE 1500
#define SAVE_EVERY_N_BLOCKS 3    // One seek point every ~ 60 ms
bool ADM_audioStreamMP3::buildTimeMap(void)
{
uint32_t size;
uint64_t newDts,pos;
DIA_workingBase *work=createWorking("Building time map");
    
    ADM_assert(access->canSeekOffset()==true);
    access->setPos(0);
    printf("[audioStreamMP3] Starting time map\n");
    rewind();
    Clock *clk=new Clock();
    clk->reset();
    uint32_t nextUpdate=clk->getElapsedMS()+TIME_BETWEEN_UPDATE;
    int markCounter=SAVE_EVERY_N_BLOCKS;
    while(1)
    {
        // Push where we are...
        markCounter++;
        if(markCounter>SAVE_EVERY_N_BLOCKS)
        {
            MP3_seekPoint *seek=new MP3_seekPoint;
            seek->offset=access->getPos();
            seek->timeStamp=lastDts;
            // Mark this point
            seekPoints.append(seek);
            markCounter=0;
        }
        // Shrink ?
        if(limit>ADM_AUDIOSTREAM_BUFFER_SIZE && start> 10*1024)
        {
            memmove(buffer, buffer+start,limit-start);
            limit-=start;
            start=0;
        }

        if(false==access->getPacket(buffer+limit, &size, 2*ADM_AUDIOSTREAM_BUFFER_SIZE-limit,&newDts))
        {
            aprintf("Get packet failed\n");
            break;
        }
        aprintf("Got MP3 packet : size =%d\n",(int)size);
        limit+=size;
        // Start at...
        pos=access->getPos();
        uint32_t now=clk->getElapsedMS();
        if(now>nextUpdate)
        {
            work->update(pos,access->getLength());
            nextUpdate=now+TIME_BETWEEN_UPDATE;
        }
        
        // consume all packets in the buffer we just got
        MpegAudioInfo info;
        uint32_t offset;

        while(1)
        {
            if(limit-start<ADM_LOOK_AHEAD) break;
            if(!getMpegFrameInfo(buffer+start,ADM_LOOK_AHEAD, &info,NULL,&offset))
            {
                start++;
                continue;
            }
            // Enough bytes ?
            if(limit-start>=info.size)
            {
                start+=info.size;
                advanceDtsBySample(info.samples);
                continue;
            }
            break;
        }

    }
    rewind();
    delete work;
    delete clk;
    access->setPos(0);
    printf("[audioStreamMP3] Ending time map\n");
    return true;
}
  
  // EOF

