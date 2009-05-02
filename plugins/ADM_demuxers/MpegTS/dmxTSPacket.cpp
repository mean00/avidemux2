/**
    \file dmxtsPacket
    \brief Packet demuxer for mpeg TS
    copyright            : (C) 2005-2009 by mean
    email                : fixounet@free.fr
        
    Warning : For PSI packet (PAT & PMT) we assume they fit into one TS packet
                Else we fail miserably.


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"

#include "dmxTSPacket.h"
#include "dmx_mpegstartcode.h"

#define TS_PES_MAX_LIMIT (1024*1024*2)

#include "ADM_getbits.h"
#include "ADM_tsCrc.cpp"

extern void mixDump(uint8_t *ptr, uint32_t len);
/**
    \fn tsPacket
    \brief ctor
*/
tsPacket::tsPacket(void) 
{
    extraCrap=0; // =4 for ts2
}
/**
    \fn tsPacket
    \brief dtor
*/
tsPacket::~tsPacket()
{
    close();
}
/**
    \fn open
    \brief dtor
*/
bool tsPacket::open(const char *filenames,FP_TYPE append)
{
    _file=new fileParser();
    if(!_file->open(filenames,&append))
    {
        printf("[DmxPS] cannot open %s\n",filenames);
        delete _file;
        _file=NULL;
        return false;
    }
    _size=_file->getSize();
    return true;
}
/**
    \fn close
    \brief dtor
*/
bool tsPacket::close(void)
{
    if(_file)
    {
        delete _file;
        _file=NULL;
    }
    return true;
}
/**
    \fn getPos
*/
uint64_t    tsPacket::getPos(void)
{
    uint64_t pos;
    _file->getpos(&pos);
    return pos;
}
/**
    \fn setPos
*/
bool    tsPacket::setPos(uint64_t pos)
{
    if(!_file->setpos(pos))
    {
        printf("[tsPacket] Cannot seek to %"LLX"\n", pos);
        return false;
    }
}
/**
    \fn getSinglePacket
    \brief 
*/
bool tsPacket::getSinglePacket(uint8_t *buffer)
{
    uint8_t scratch[16];
#define MAX_SKIPPED_PACKET 6000
#define MAX_SEARCH 2048
    int count=0;
again:
    while(1)
    {
        uint8_t r=_file->read8i();
        if(r==TS_MARKER || _file->end()) break;
        count++;
        if(count>MAX_SEARCH) 
        {
            printf("[Mpeg TS] Sync definitevly lost\n");
            return false;
        }
#ifdef TS_DEBUG1
        uint64_t pos=_file->getpos(&pos);
        printf("[%02d] count=%d at 0x%x\n",r,count,pos);
#endif
    }
    if(_file->end()==true) 
    {
        printf("[Mpeg Ts] End of file reached\n");
        return false;
    }
    _file->read32(TS_PACKET_LEN-1,buffer); // 184-1
    if(extraCrap)  _file->read32(extraCrap,scratch);
    uint8_t r=_file->peek8i();
    if(r!=TS_MARKER)
    {
#ifdef TS_DEBUG1
        printf("[tsPacket] Sync lost (0x%x)\n",r);
#endif
        goto again;
    }
    return true;
}

/**
    \fn getNextPsiPacket
    \brief Take a raw packet of type pid & remove the header
*/
bool tsPacket::getNextPacket_NoHeader(uint32_t pid,TSpacketInfo *pkt,bool psi)
{
    uint8_t scratch[188+4];
    int count=0;
nextPack:

    if(false==getSinglePacket(scratch)) return false;
    uint32_t id=scratch[1]+((scratch[0]&0x1F)<<8);
//#define TS_DEBUG2
#ifdef TS_DEBUG2
        printf("[Demuxer] Looking for 0x%x found 0x%x\n",pid,id);
#endif
    count++;
    if(count>MAX_SKIPPED_PACKET) return false;

    if(id!=pid) 
    {
        updateStats(scratch);
        goto nextPack;
    }
    pkt->pid=pid;
#ifdef TS_DEBUG2
    printf("[**************> Found matching pid 0x%x\n",pid);
#endif
    
    int payloadUnitStart=scratch[0]&0x40;
    int fieldControl=(scratch[2]>>4)&3;
    int continuity=(scratch[2]&0xf);
    int len=TS_PACKET_LEN-4; // useful datas

    pkt->continuityCounter=continuity;
    pkt->payloadStart=payloadUnitStart;
    // Adaptation field
    // 11 Adapt+payload
    // 10 Adapt only
    // 01 Payload only
    // 00 forbidden
    if(!(fieldControl & 1)) 
    {
        // No payload, continue
#ifdef TS_DEBUG2
        printf("[Demuxer] No payload\n");
#endif        
        goto nextPack;
    }
    uint8_t *start,*end;
    start=scratch+3;
    end=scratch+TS_PACKET_LEN-1;

    if((fieldControl & 2)|| psi) // Adaptation layer
    {
        int payloadSize=*start++;
        start+=payloadSize;
    }
    int size=(int)(end-start);
    if(size<=0)  
    {
#ifdef TS_DEBUG2
        printf("[Demuxer] size=%d\n",size);
#endif        

        goto nextPack;
    }
    memcpy(pkt->payload,start,size);
    pkt->payloadSize=size;
    uint64_t pos;
    _file->getpos(&pos);
    pkt->startAt=pos-extraCrap-TS_PACKET_LEN;
    return true;
}
/**
        \fn updateStats
        \brief Hook for tracker
*/
bool        tsPacket::updateStats(uint8_t *data)
{
    return true;
}
/**
    \fn getNextPsiPacket
    \brief Take a raw packet of type pid & remove the header (PSI)
*/
#ifdef VERBOSE_PSI
#define DUMMY(x,n) {dummy=get_bits(&s,n);printf("[TS]: "#x" =0x%x %d\n",dummy,dummy);}
#else
#define DUMMY(x,n) {dummy=get_bits(&s,n);}
#endif
bool tsPacket::getNextPSI(uint32_t pid,TS_PSIpacketInfo *psi)
{
    TSpacketInfo pkt;
nextPack2:

    if(false==getNextPacket_NoHeader(pid,&pkt,true)) return false;    

    GetBitContext s;
    uint32_t tableId;
    uint32_t sectionLength=0;
    uint32_t transportStreamId=0;
    uint32_t dummy;
    

    init_get_bits( &s,pkt.payload, (pkt.payloadSize)*8); // dont need checksum

    DUMMY(tableId,8);
    skip_bits(&s,1);             // Section syntax indicator
    if(get_bits(&s,1))             // Marker
    {
          printf("[MpegTs] getNextPSI Missing 0 marker\n");
          goto nextPack2;
    }
    skip_bits(&s,2);
    sectionLength=get_bits(&s,12);    // Section Length
#if 1
    if(sectionLength+3>pkt.payloadSize) 
    {
        printf("[MpegTs] sectionLength=%d, len=%d\n",sectionLength,pkt.payloadSize);
        goto nextPack2;
    }
#endif
    transportStreamId=get_bits(&s,16);// transportStreamId
#ifdef VERBOSE_PSI
    printf("[MpegTs] Section length    =%d\n",sectionLength);
    printf("[MpegTs] transportStreamId =%d\n",transportStreamId);
#endif
    skip_bits(&s,2);                  // ignored
    DUMMY(VersionNumber,5);         // Version number
    DUMMY(CurrentNext,1);           // Current Next indicator
    psi->count=get_bits(&s,8);           // Section number
    psi->countMax=get_bits(&s,8);        // Section last number
#ifdef VERBOSE_PSI
    printf("[MpegTs] Count=%d CountMax=%d\n",psi->count,psi->countMax);
#endif
    if(psi->count!=psi->countMax) return false; // we dont handle split psi at the moment

    uint8_t *c=pkt.payload+sectionLength-4+3;

// Verify CRC
    uint32_t crc1=mpegTsCRC(pkt.payload,sectionLength-4+3);
    uint32_t crc2=(c[0]<<24)+(c[1]<<16)+(c[2]<<8)+c[3];
    if(crc1!=crc2)
    {
        printf("[MpegTs] getNextPSI bad checksum :%04x vs %04x\n",crc1,crc2);
        goto nextPack2;
    }


    int hdr=(8+16+16+8+8+8)/8;
    int payloadsize=sectionLength-4-5; // Remove checksum & header
    if(payloadsize<4) goto nextPack2;
    psi->payloadSize=payloadsize;
    memcpy(psi->payload,pkt.payload+hdr,psi->payloadSize);
    return true;
}
/**
        \fn getNextPES
        \brief Returns the next PES packet
        
*/

bool        tsPacket::getNextPES(TS_PESpacket *pes)
{
#if 1
#define zprintf(...) {}
#else
#define zprintf printf
#endif
    TSpacketInfo pkt;
    pes->fresh=false;
nextPack3:
    // Sync at source
    if(false==getNextPacket_NoHeader(pes->pid,&pkt,false)) return false;    
    // If it does not contain a payload strat continue
    if(!pkt.payloadStart)
    {
        printf("[Ts Demuxer] Pes for Pid =0x%d does not contain payload start\n",pes->pid);
        goto nextPack3;
    }
    //____________________
    //____________________
    // 1- Start Headers
    //____________________
    //____________________
    uint32_t code=(pkt.payload[0]<<24)+(pkt.payload[1]<<16)+(pkt.payload[2]<<8)+pkt.payload[3];
    zprintf("[TS Demuxer] Code=0x%x pid=0x%x\n",code,pes->pid);
    if((code&0xffffff00)!=0x100)
    {
        zprintf("[Ts Demuxer] No PES startcode at 0x%"LLX"\n",pkt.startAt);
        zprintf("0x:%02x %02x %02x %02x\n",pkt.payload[4],pkt.payload[5],pkt.payload[6],pkt.payload[7]);
        goto nextPack3;
    }
    //mixDump(pkt.payload,pkt.payloadSize);
    uint32_t pesPacketLen=(pkt.payload[4]<<8)+(pkt.payload[5]);
    zprintf("[TS Demuxer] Pes Packet Len=%d\n",pesPacketLen);
    
    pes->payloadSize=0;
    pes->addData(pkt.payloadSize,pkt.payload);
    pes->startAt=pkt.startAt;
    while(1)
    {
        uint64_t pos;
        _file->getpos(&pos);
        if(false==getNextPacket_NoHeader(pes->pid,&pkt,false)) return false;    
        if(!pkt.payloadStart)
         {
                pes->addData(pkt.payloadSize,pkt.payload);
         }else  
         {
                _file->setpos(pos);
                break;
         }
        if(pes->payloadLimit>TS_PES_MAX_LIMIT)
        {
            printf("[Ts Demuxer] Pes Packet too big\n");
            goto nextPack3;
        }
    }
    //____________________
    // Now decode PES header (extra memcpy)
    //____________________
    zprintf("[Ts Demuxer] Full size :%d\n",pes->payloadSize);
    if(false==decodePesHeader(pes))
        goto nextPack3;

    //
    zprintf("[Ts Demuxer] Found PES of len %d\n",pes->payloadSize);  
    pes->fresh=true;
    
    return true;
}
/**
    \fn decodePesHeader
*/
#define fail(x) {printf("[Ts Demuxer]*********"x"*******\n");return false;}
bool tsPacket::decodePesHeader(TS_PESpacket *pes)
{
    uint8_t  *start=pes->payload+6;
    uint8_t  *end=pes->payload+pes->payloadSize;
    uint8_t  stream=pes->payload[3];
    uint32_t packLen=(pes->payload[4]<<8)+(pes->payload[5]);
    int      align=0,c;


    pes->dts=ADM_NO_PTS;
    pes->pts=ADM_NO_PTS;

    if(pes->payloadSize<(4+2+1+2)) return false;
    
    while(*start==0xff && start<end) start++; // Padding
    if(start>=end) fail("too much padding");

    c=*start++;
    if((c&0xc0)!=0x80) fail("No Mpeg2 marker");
    
        uint32_t ptsdts,len;
        if(c & 4) align=1;      
        c=*start++;     // PTS/DTS
        //printf("%x ptsdts\n",c
        ptsdts=c>>6;
        // header len
        len=*start++;
        int available=(int)(end-start);
        switch(ptsdts)
        {
                case 2: // PTS=1 DTS=0
                       
                        {
                                if(available<5) return false;
                                uint64_t pts1,pts2,pts0;
                                //      printf("\n PTS10\n");
                                        pts0=start[0];  
                                        pts1=(start[1]<<8)+start[2]; 
                                        pts2=(start[3]<<8)+start[4]; 
                                        start+=5;
                                        pes->pts=(pts1>>1)<<15;
                                        pes->pts+=pts2>>1;
                                        pes->pts+=(((pts0&6)>>1)<<30);
                        }
                        break;
                case 3: // PTS=1 DTS=1
                                if(available<10) return false;
                                #define PTS11_ADV 10 // nut monkey
                                if(len>=PTS11_ADV)
                                {
                                        uint32_t skip=PTS11_ADV;
                                        uint64_t pts1,pts2,dts,pts0;
                                                //      printf("\n PTS10\n");
                                                pts0=start[0];  
                                                pts1=(start[1]<<8)+start[2]; 
                                                pts2=(start[3]<<8)+start[4]; 
                                                start+=5;
                                                                        
                                                pes->pts=(pts1>>1)<<15;
                                                pes->pts+=pts2>>1;
                                                pes->pts+=(((pts0&6)>>1)<<30);
                                                pts0=*start++;  
                                                pts1=(start[0]<<8)+start[1]; 
                                                pts2=(start[2]<<8)+start[3];       
                                                start+=5;
                                                pes->dts=(pts1>>1)<<15;
                                                pes->dts+=pts2>>1;
                                                pes->dts+=(((pts0&6)>>1)<<30);
                                   }
                                   break;               
                case 1:
                                fail("unvalid pts/dts");
                                break;
                case 0: 
                                // printf("\n PTS00\n");
                                break; // no pts nor dts
                                                                
        }  
       
        int sizeCheck=pes->payloadSize-6;
        int tail=0;
        pes->offset=start-pes->payload;
        if(packLen)
        {
            //printf("***Zimbla***\n");
            if(packLen<sizeCheck) 
            {
                tail=sizeCheck-packLen;
                pes->payloadSize-=tail;
            }
            else
                if(packLen>sizeCheck)
                {
                    printf("[TS Packet] PackLen=%d, avalailble=%d\n",packLen,sizeCheck);
                    fail("Pes too long");
                }   
        }
        return true;
}
/**
    \fn getPacket
*/      
bool        tsPacket::getPacket(uint32_t maxSize, uint8_t *pid, uint32_t *packetSize,uint64_t *opts,uint64_t *odts,uint8_t *buffer,uint64_t *startAt)
{

        return false;
       
}

#define ADM_PACKET_LINEAR 10*1024
/**
    \fn tsPacket
*/
tsPacketLinear::tsPacketLinear(uint32_t pid) : tsPacket()
{
    oldStartAt=0xfffffff;
    oldBufferLen=0;
    pesPacket=new TS_PESpacket(pid);
    eof=false;
}
/**
    \fn ~tsPacket
*/
tsPacketLinear::~tsPacketLinear() 
{
    if(pesPacket) delete pesPacket;
    pesPacket=NULL;
}
/**
    \fn refill
*/
bool tsPacketLinear::refill(void) 
{
// In case a startcode spawns across 2 packets
// we have to keep track of the old one
        oldBufferDts=pesPacket->dts;
        oldBufferPts=pesPacket->pts;
        oldStartAt=pesPacket->startAt;
        oldBufferLen=pesPacket->payloadSize;
        if(false==getNextPES(pesPacket))
        {
                printf("[tsPacketLinear] Refill failed for pid :%x\n",pesPacket->pid);
                return false;
        }
        return true;
}
/**
    \fn readi8
*/
uint8_t tsPacketLinear::readi8(void)
{
    consumed++;
    if(pesPacket->offset<pesPacket->payloadSize)
    {
        return pesPacket->payload[pesPacket->offset++];
    }
    if(false==refill()) 
    {
        eof=1;
        return 0;
    }
    return pesPacket->payload[pesPacket->offset++];
    
}
/**
    \fn readi16
*/
uint16_t tsPacketLinear::readi16(void)
{
    if(pesPacket->offset+1<pesPacket->payloadSize)
    {
        uint8_t *r=pesPacket->payload+pesPacket->offset;
        uint16_t v=(r[0]<<8)+r[1];;
        
        pesPacket->offset+=2;
        consumed+=2;
        return v;
    }
    return (readi8()<<8)+readi8();
}
/**
    \fn readi32
*/
uint32_t tsPacketLinear::readi32(void)
{
    if(pesPacket->offset+3<pesPacket->payloadSize)
    {
         uint8_t *p=pesPacket->payload+pesPacket->offset;
         uint32_t v=(p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
         pesPacket->offset+=4;
        consumed+=4;
        return v;
    }
    return (readi16()<<16)+readi16();
}
/**
    \fn forward
*/
bool tsPacketLinear::forward(uint32_t v)
{
next:
 uint32_t delta=pesPacket->payloadSize-pesPacket->offset;
    if(v>100*1000)
    {
        ADM_assert(0);
    }
    if(v<=delta)
    {
        pesPacket->offset+=v;
        consumed+=v;
        return true;
    }
    // v>delta
    v-=delta;
    consumed+=delta;
    if(!refill()) return false;
    goto next;
}

/**
    \fn bool    read(uint32_t len, uint8_t *buffer);
    \brief
*/
bool    tsPacketLinear::read(uint32_t len, uint8_t *out)
{
#if 0
    printf("[tsRead] Size 0x%x %d\n",len,len);
#endif
    // Enough already ?
    while(len)
    {
        uint32_t avail=pesPacket->payloadSize-pesPacket->offset;
        uint32_t chunk=avail;
        if(chunk>len) chunk=len;
#if 0
        printf("len:%ld avail:%ld chunk %ld index:%d size:%d\n",
                len,avail,chunk,bufferIndex,bufferLen);
#endif
        memcpy(out,pesPacket->payload+pesPacket->offset,chunk);
        pesPacket->offset+=chunk;
        len-=chunk;
        out+=chunk;
        consumed+=chunk;
        if(pesPacket->payloadSize==pesPacket->offset)
        {
            //printf("Refill\n");
            if(false==refill()) return false;
        }
    }
    return true;
}
/**
        \fn getInfo
        \brief Returns info about the current (or previous if it spawns) packet.
            It is expected that the caller will do -4 to the index to get the start of the 
            startCode
*/
bool    tsPacketLinear::getInfo(dmxPacketInfo *info)
{
#warning FIXME
    if(pesPacket->offset<4)
    {
        info->startAt=this->oldStartAt;
        info->offset=oldBufferLen;
        info->pts=oldBufferPts;
        info->dts=oldBufferDts;

    }else
    {
        info->startAt=pesPacket->startAt;
        info->offset=pesPacket->offset;
        info->pts=pesPacket->pts;
        info->dts=pesPacket->dts;
    }
    return true;

};
/**
    \fn seek
    \brief Async jump
*/
bool    tsPacketLinear::seek(uint64_t packetStart, uint32_t offset)
{
    if(!_file->setpos(packetStart))
    {
        printf("[tsPacket] Cannot seek to %"LLX"\n",packetStart);
        return 0;
    }
    if(!refill())
    {
        printf("[tsPacketLinear] Seek to %"LLX":%"LX" failed\n",packetStart,offset);
        return false;
    }
    ADM_assert(offset<pesPacket->payloadSize);
    pesPacket->offset=offset;
    return true;
}
/**
    \fn getConsumed
    \brief returns the # of bytes consumed since the last call
*/
uint32_t tsPacketLinear::getConsumed(void)
{
    uint32_t c=consumed;
    return c;
}
/**
    \fn setConsumed
    \brief set consumed bytes
*/
bool tsPacketLinear::setConsumed(uint32_t v)
{
    consumed=v;
    return true;
}

/**
    \fn changePid
    \brief change the pid of the stream we read (used when probing all tracks)
*/
bool    tsPacketLinear::changePid(uint32_t pid) 
{
    pesPacket->pid=pid;
    pesPacket->offset=pesPacket->payloadSize;
    return true;
}
/* ********************************************************* */
/**
    \fn tsPacketLinearTracker
*/
tsPacketLinearTracker::tsPacketLinearTracker(uint32_t pid,listOfTsAudioTracks *audio) : tsPacketLinear(pid)
{
    int nb=audio->size();
    otherPes=new TS_PESpacket(0);
    totalTracks=nb;
    if(!nb)    
    {
        this->stats=NULL;
        return;
    }

    // Convert ADM_TS_TRACKS to ststa
    stats=new packetTSStats[nb];    
    memset(stats,0,sizeof(packetTSStats)*nb);
    for(int i=0;i<nb;i++)
    {
        
        stats[i].pid=(*audio)[i].esId;
        stats[i].startDts=ADM_NO_PTS;
    }
}
/**
    \fn ~tsPacketLinearTracker
*/
tsPacketLinearTracker::~tsPacketLinearTracker()
{
    if(otherPes) delete otherPes;
    otherPes=NULL;
    if(stats) delete [] stats;
    stats=NULL;
}
/**
    \fn tsPacketLinearTracker
*/
bool    tsPacketLinearTracker::getStats(uint32_t *nb,packetTSStats **stats)
{
    *nb=totalTracks;
    *stats=this->stats;
    return true;
}
/**
        \fn updateStats
        \brief Decode a bit of PES header, just enough to get PTS or DTS
            if only PTS is there we assume PTS=DTS
*/
bool tsPacketLinearTracker::updateStats(uint8_t *scratch)
{
    uint32_t id=scratch[1]+((scratch[0]&0x1F)<<8);
    int found=-1;
    // Look if it is a pid we are interested in
    for(int i=0;i<totalTracks;i++)
        if(id==stats[i].pid) found=i;
    if(found==-1) return false;

    
    int payloadUnitStart=scratch[0]&0x40;
    int fieldControl=(scratch[2]>>4)&3;
    int continuity=(scratch[2]&0xf);


    if(!payloadUnitStart) return false; // no PES start in here...



    // Adaptation field
    // 11 Adapt+payload
    // 10 Adapt only
    // 01 Payload only
    // 00 forbidden
    if(!(fieldControl & 1)) 
    {
        // No payload, continue
#ifdef TS_DEBUG2
        printf("[Demuxer] No payload\n");
#endif        
        return true;
    }
    uint8_t *start,*end;
    start=scratch+3;
    end=scratch+TS_PACKET_LEN-1;

    if((fieldControl & 2)) // Adaptation layer
    {
        int payloadSize=*start++;
        start+=payloadSize;
    }
    int size=(int)(end-start);
    if(size<=0)  
    {
#ifdef TS_DEBUG2
        printf("[Demuxer] size=%d\n",size);
#endif        
        return true;
    }
    // Look into pes packet starting at "start"
    otherPes->payloadSize=size;
    uint64_t pos;
    _file->getpos(&pos);

    //*************************
    // ENTER PES
    //*************************

    otherPes->startAt=pos-extraCrap-TS_PACKET_LEN;
    // PES startcode ?
    if( start[0] || start[1] || start[2]!=1) return false; 
    int stream=start[3];
    // Skip startcode + length
    start+=6;
    // Update our stats
    stats[found].startAt=otherPes->startAt;
    stats[found].count++;
    //stats[found].startDts=ADM_NO_PTS;
    
    // Get PTS, DTS
#define LEFT (int)(end-start)
    int c,available;
    if(LEFT<(4+2+1+2)) return false;
    
    while(*start==0xff && start<end) start++; // Padding
    if(LEFT<5) fail("Not enough data in OES");

    c=*start++;
    if((c&0xc0)!=0x80) 
    {
        printf("[TS Demuxer] stream=0x%x pid=%d PES header :0x%x no mpeg2 PES marker\n",stream,id,c);
        return false;
    }
    
        uint32_t ptsdts,len;
        c=*start++;     // PTS/DTS
        //printf("%x ptsdts\n",c
        ptsdts=c>>6;
        // header len
        len=*start++;
        available=LEFT;
        if(len>available) fail("Not enough data for PES header");
    
        switch(ptsdts)
        {
                case 2: // PTS=1 DTS=0
                       
                        {
                                if(available<5) return false;
                                uint64_t pts1,pts2,pts0;
                                //      printf("\n PTS10\n");
                                        pts0=start[0];  
                                        pts1=(start[1]<<8)+start[2]; 
                                        pts2=(start[3]<<8)+start[4]; 
                                        start+=5;
                                        stats[found].startDts=(pts1>>1)<<15;
                                        stats[found].startDts+=pts2>>1;
                                        stats[found].startDts+=(((pts0&6)>>1)<<30);
                        }
                        break;
                case 3: // PTS=1 DTS=1
                                if(available<10) return false;
                                #define PTS11_ADV 10 // nut monkey
                                if(len>=PTS11_ADV)
                                {
                                        uint32_t skip=PTS11_ADV;
                                        uint64_t pts1,pts2,dts,pts0;
                                                //      printf("\n PTS10\n");
                                                pts0=start[0];  
                                                pts1=(start[1]<<8)+start[2]; 
                                                pts2=(start[3]<<8)+start[4]; 
                                                start+=5;
                                                                        
                                                // Assume PTS=DTS
                                                pts0=*start++;  
                                                pts1=(start[0]<<8)+start[1]; 
                                                pts2=(start[2]<<8)+start[3];       
                                                start+=5;
                                                stats[found].startDts=(pts1>>1)<<15;
                                                stats[found].startDts+=pts2>>1;
                                                stats[found].startDts+=(((pts0&6)>>1)<<30);
                                   }
                                   break;               
                case 1:
                                fail("unvalid pts/dts");
                                break;
                case 0: 
                                // printf("\n PTS00\n");
                                break; // no pts nor dts
                                                                
        }  
       

    return true;
}
// EOF