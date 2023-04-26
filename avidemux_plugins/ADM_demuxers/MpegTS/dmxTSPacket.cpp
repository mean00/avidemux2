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

#include "ADM_getbits.h"
#include "ADM_tsCrc.cpp"

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
    \fn score
    \return the number of TS packet of size TS_PACKET_SIZE+extraData in a raw, nbTry tries

*/
static int score(fileParser *parser, int nbTry, int extraData)
{
int count=0;
int round=nbTry;
    if(parser->read8i()!=TS_MARKER)
        return 0;
    while(round--)
    {
        if(!parser->forward(TS_PACKET_LEN-1+extraData)) break;
        if(parser->read8i()!=TS_MARKER) break;
        count++;
    }
    return count;
}
/**
    \fn open
    \brief dtor
*/
bool tsPacket::open(const char *filenames, int append)
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

    // Detect TS1/TS2...
        printf("[TsPacket] Detecting TS/TS2...\n");

#define MAX_BOGUS_TS_MARKERS 4
    for(int i=0;i<MAX_BOGUS_TS_MARKERS;i++)
    {
        int tryMe=250;
        while(tryMe--)
        {
            if(_file->read8i()==TS_MARKER) break;
            if(_file->end()) 
            {
                tryMe=0;
            }
        }
        if(!tryMe) 
        {
            printf("[TsPacket] Cannot sync ???\n");
            return true;
        }
        uint64_t startPos=getPos()-1;
        printf("[tsPacket::open] Sync byte found at offset %" PRIu64"\n",startPos);
        int score1,score2;

        setPos(startPos);
#define NB_CONTIGUOUS_PACKETS 20
#define SYNC_THRESHOLD 2 /* require at least 2 consecutive packets */
        score1=score(_file,NB_CONTIGUOUS_PACKETS,0);
        setPos(startPos);
        score2=score(_file,NB_CONTIGUOUS_PACKETS,4);
        printf("[TsPacket] Score : 188:%d, 192:%d out of %d\n",score1,score2,NB_CONTIGUOUS_PACKETS);
        if(!score1 && !score2)
        {
            startPos++;
            ADM_info("Probably bogus sync byte detection, retrying at offset %" PRIu64"\n",startPos);
            setPos(startPos);
            continue;
        }
        if(score1 < SYNC_THRESHOLD && score2 < SYNC_THRESHOLD)
        {
            startPos++;
            ADM_info("Unconclusive results, retrying at offset %" PRIu64"\n",startPos);
            setPos(startPos);
            continue;
        }
        if(score2>score1)
        {
            printf("[TsPacket] Probably TS2 (192)...\n");
            extraCrap=4;
        }else
        {
            printf("[TsPacket] Probably TS1 (188)...\n");
        }
        printf("[tsPacket::open] Sync established at offset %" PRIu64"\n",startPos);
        break;
    }
    setPos(0);
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
        printf("[tsPacket] Cannot seek to %" PRIx64"\n", pos);
        return false;
    }
    return true;
}
/**
    \fn getSinglePacket
    \brief Read s TS packet, make sure it starts & ends by TS marker (0x47)
*/
bool tsPacket::getSinglePacket(uint8_t *buffer)
{
#define MAX_SKIPPED_PACKET 300*100 // 150*180*100=~ 2.2 MBytes
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
        printf("[tsPacket::getSinglePacket] End of file reached\n");
        return false;
    }
    _file->read32(TS_PACKET_LEN-1,buffer); // 184-1
    if(extraCrap)  _file->forward(extraCrap);
    uint8_t r=_file->peek8i();
    if(r!=TS_MARKER)
    {
        printf("[tsPacket::getSinglePacket] Sync lost at 0x%" PRIx64" (value: 0x%x)\n",getPos(),r);
        goto again;
    }
    return true;
}

/**
    \fn getNextPacket_NoHeader
    \brief Get a raw packet with given pid & remove the header.
    @param first: Make sure a payload unit starts in this packet, skip data preceding it.
*/
bool tsPacket::getNextPacket_NoHeader(uint32_t pid,TSpacketInfo *pkt,bool first)
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

    if(fieldControl & 2) // Adaptation layer
    {
        int adaptationFieldLength = *start++;
        start += adaptationFieldLength;
        if(start >= end)
        {
#ifdef TS_DEBUG2
            printf("[getNextPacket_NoHeader] Adaptation field length %d out of bounds!\n",adaptationFieldLength);
#endif
            goto nextPack;
        }
    }
    if(first && pkt->payloadStart) // skip to the start of new payload
    {
        int payloadOffset = *start++;
        start += payloadOffset;
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

#define PSI_TABLE_HEADER_SIZE 3
#define PSI_TABLE_SYNTAX_SIZE 5
#define PSI_CHECKSUM_SIZE 4

/**
    \fn verifyPsiChecksum
    @param  data: Pointer to PSI table ID.
    @param  len:  Size of payload starting with PSI table ID including CRC32 at its end.
*/
static bool verifyPsiChecksum(uint8_t *data, uint32_t len)
{
    if(len <= PSI_TABLE_HEADER_SIZE + PSI_TABLE_SYNTAX_SIZE + PSI_CHECKSUM_SIZE)
        return false;
    len -= PSI_CHECKSUM_SIZE;
    uint8_t *c = data + len;
    uint32_t crc1 = mpegTsCRC(data,len);
    uint32_t crc2 = (c[0]<<24)+(c[1]<<16)+(c[2]<<8)+c[3];
    if(crc1 == crc2)
        return true;

    ADM_warning("Bad checksum : %04x vs %04x\n",crc1,crc2);
    return false;
}

/**
    \fn getNextPSI
    \brief Assemble PSI table data from raw packets with given pid, remove header and checksum.
*/
#ifdef VERBOSE_PSI
#define DUMMY(x,n) {dummy=bits.get(n);printf("[TS]: "#x" =0x%x %d\n",dummy,dummy);}
#else
#define DUMMY(x,n) {dummy=bits.get(n);}
#endif
bool tsPacket::getNextPSI(uint32_t pid,TS_PSIpacketInfo *psi)
{
    const int hdr = PSI_TABLE_HEADER_SIZE + PSI_TABLE_SYNTAX_SIZE;
    int multiPacketPsi=0;
    int nbRetries=0;
    uint64_t startOffset=0;
    uint32_t remaining,sectionLength=0;
    uint32_t transportStreamId=0;
    uint32_t dummy,tail = 0;
    TSpacketInfo pkt;
nextPack2:
    if(nbRetries && pkt.startAt-startOffset>(1<<25)) // max. 32 MiB
    {
        ADM_warning("Giving up after %d retries, consumed %" PRId64" bytes\n",nbRetries,pkt.startAt-startOffset);
        return false;
    }
    if(false == getNextPacket_NoHeader(pid,&pkt,!multiPacketPsi))
        return false;
    if(!nbRetries)
        startOffset=pkt.startAt;
    nbRetries++;

    if(!multiPacketPsi && !pkt.payloadStart)
        goto nextPack2;

    if(!multiPacketPsi)
    {
        if(pkt.payloadSize < hdr)
        {
            ADM_warning("PSI packet size %" PRIu32" too small, need at least %d bytes.\n",pkt.payloadSize,hdr);
            goto nextPack2;
        }
        // getBits needs AV_INPUT_BUFFER_PADDING_SIZE bytes past the end of payload
        // zeroed out to be safe against overreads.
        uint8_t paddedBuffer[TS_PACKET_LEN + 64];
        memcpy(paddedBuffer, pkt.payload, pkt.payloadSize);
        memset(paddedBuffer + pkt.payloadSize, 0, 64);
        getBits bits(pkt.payloadSize, paddedBuffer);

        DUMMY(tableId,8);
        int section_syntax_indicator=bits.get(1);

        if(!section_syntax_indicator)
        {
#ifdef VERBOSE_PSI
            ADM_warning("Syntax section indicator not set\n");
#endif
            goto nextPack2;
        }

        if(bits.get(1)) // Private bit
        {
            ADM_warning("Section syntax is set to private\n");
            goto nextPack2;
        }
        int reserved = bits.get(2); // 2 Reserved bits
        if(reserved!=3)
            printf("[getNextPSI] Invalid data: reserved bits = %d, should be 3\n",reserved);
        int unused = bits.get(2); // 2 unused bits
        if(unused)
            printf("[getNextPSI] Invalid data: unused bits = %d, should be 0\n",unused);
        sectionLength = bits.get(10); // Section Length
        if(sectionLength <= PSI_TABLE_SYNTAX_SIZE + PSI_CHECKSUM_SIZE ||
           sectionLength > 0x3FF - PSI_TABLE_HEADER_SIZE)
        {
            printf("[getNextPSI] Invalid section length: %d\n",sectionLength);
            goto nextPack2;
        }
#if 1
        if(sectionLength + PSI_TABLE_HEADER_SIZE > pkt.payloadSize)
        {
            if(!multiPacketPsi)
                ADM_warning("[MpegTs] Multi Packet PSI ? sectionLength=%d, len=%d\n",sectionLength,pkt.payloadSize);
            multiPacketPsi++;
        }
#endif
        transportStreamId=bits.get(16);// transportStreamId
#ifdef VERBOSE_PSI
        printf("[MpegTs] Section length    =%d\n",sectionLength);
        printf("[MpegTs] transportStreamId =%d\n",transportStreamId);
#endif
        bits.skip(2); // ignored
        DUMMY(VersionNumber,5); // Version number
        DUMMY(CurrentNext,1); // Current Next indicator
        psi->count=bits.get(8); // Section number
        psi->countMax=bits.get(8); // Section last number
#ifdef VERBOSE_PSI
        printf("[MpegTs] Count=%d CountMax=%d\n",psi->count,psi->countMax);
#endif
        if(psi->count!=psi->countMax) return false; // we dont handle split psi at the moment

        remaining = sectionLength + PSI_TABLE_HEADER_SIZE;
    }

    if(!multiPacketPsi)
    {
        int payloadsize = sectionLength + PSI_TABLE_HEADER_SIZE;
        if(false == verifyPsiChecksum(pkt.payload, payloadsize))
            goto nextPack2;
        payloadsize -= hdr + PSI_CHECKSUM_SIZE;
        psi->payloadSize=payloadsize;
        memcpy(psi->payload,pkt.payload+hdr,payloadsize);
        return true;
    }

    // TODO: check continuity

    // Copy packet payload
    while(true)
    {
        int chunk = (pkt.payloadSize > remaining)? remaining : pkt.payloadSize;
        ADM_assert(tail + chunk < TS_PSI_MAX_LEN);
        memcpy(psi->payload + tail, pkt.payload, chunk); // we keep PSI table header here
        tail += chunk;
        remaining -= chunk;
        if(remaining < 1) break;
        goto nextPack2;
    }
    // Verify PSI table checksum.
    if(false == verifyPsiChecksum(psi->payload, sectionLength + PSI_TABLE_HEADER_SIZE))
    {
        multiPacketPsi = 0;
        tail = 0;
        goto nextPack2;
    }
    psi->payloadSize = sectionLength - PSI_TABLE_SYNTAX_SIZE - PSI_CHECKSUM_SIZE;
    memmove(psi->payload, psi->payload + hdr, psi->payloadSize);

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
#define zprintf ADM_info
#endif
    int nbRetries=0,counter=0;
    uint64_t startOffset=0;
    TSpacketInfo pkt;
    pkt.startAt=0;
    pes->fresh=false;
nextPack3:
    if(pkt.startAt-startOffset>(1<<25)) // max. 32 MiB
    {
        ADM_warning("Giving up after %d retries, consumed %" PRId64" bytes\n",nbRetries,pkt.startAt-startOffset);
        return false;
    }
    // Sync at source
    if(false==getNextPacket_NoHeader(pes->pid,&pkt,false)) return false;
    if(!nbRetries)
        startOffset=pkt.startAt;
    nbRetries++;
    // If it does not contain a payload start continue
    bool mark=false;
    uint32_t code=(pkt.payload[0]<<24)+(pkt.payload[1]<<16)+(pkt.payload[2]<<8)+pkt.payload[3];
    if((code&0xffffffC0)==0x1C0) mark=true;
    zprintf("Mark=%x\n",code);
    if(!pkt.payloadStart && !mark)
    {
#if 0
        printf("[Ts Demuxer] Pes for Pid = 0x%x (%d) does not contain payload start\n",pes->pid,pes->pid);
#endif
        counter++;
        goto nextPack3;
    }
    if(counter>1)
        zprintf("Payload start for Pid = 0x%x (%d) found after %d retries\n",pes->pid,pes->pid,counter);
    counter=0;
    //____________________
    //____________________
    // 1- Start Headers
    //____________________
    //____________________
    zprintf("[TS Demuxer] Code=0x%x pid=0x%x\n",code,pes->pid);
    if((code&0xffffff00)!=0x100)
    {
#if 0
        printf("[Ts Demuxer] No PES startcode at 0x%" PRIx64"\n",pkt.startAt);
        printf("0x:%02x %02x %02x %02x\n",pkt.payload[4],pkt.payload[5],pkt.payload[6],pkt.payload[7]);
#endif
        goto nextPack3;
    }
    if(nbRetries>1)
        ADM_info("PES startcode found at 0x%" PRIx64" after %d retries\n",pkt.startAt,nbRetries);
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
    zprintf("[Ts Demuxer] Found PES of len %d, offset=%d\n",pes->payloadSize, pes->offset);  
    pes->fresh=true;
    
    return true;
}
/**
    \fn decodePesHeader
*/
#define fail(x) {ADM_warning("[Ts Demuxer]*********" x"*******\n");return false;}
bool tsPacket::decodePesHeader(TS_PESpacket *pes)
{
    uint8_t  *start=pes->payload+6;
    uint8_t  *end=pes->payload+pes->payloadSize;
    uint32_t packLen=(pes->payload[4]<<8)+(pes->payload[5]);
    int      c;


    pes->dts=ADM_NO_PTS;
    pes->pts=ADM_NO_PTS;

    if(pes->payloadSize<(4+2+1+2)) 
    {
            ADM_warning("[Ts] Pes size too small\n");
            return false;
    }
    while(*start==0xff && start<end) start++; // Padding
    if(start>=end) fail("too much padding");

    c=*start++;
    if((c&0xc0)!=0x80) fail("No Mpeg2 marker");
    
        uint32_t ptsdts,len;
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
                                if(available<5) fail("Not enough bytes for PTS");
                                uint64_t pts1,pts2,pts0;
                                //      printf("\n PTS10\n");
                                        pts0=start[0];  
                                        pts1=(start[1]<<8)+start[2]; 
                                        pts2=(start[3]<<8)+start[4]; 
                                        pes->pts=(pts1>>1)<<15;
                                        pes->pts+=pts2>>1;
                                        pes->pts+=(((pts0&6)>>1)<<30);
                        }
                        break;
                case 3: // PTS=1 DTS=1
                                if(available<10) fail("Not enough bytes for PTS/DTS");
                                #define PTS11_ADV 10 // nut monkey
                                if(len>=PTS11_ADV)
                                {
                                        uint64_t pts1,pts2,pts0;
                                                //      printf("\n PTS10\n");
                                                pts0=start[0];  
                                                pts1=(start[1]<<8)+start[2]; 
                                                pts2=(start[3]<<8)+start[4]; 
                                                                        
                                                pes->pts=(pts1>>1)<<15;
                                                pes->pts+=pts2>>1;
                                                pes->pts+=(((pts0&6)>>1)<<30);
                                                pts0=start[5];  
                                                pts1=(start[6]<<8)+start[7]; 
                                                pts2=(start[8]<<8)+start[9];       
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
        start+=len; 
        int sizeCheck=pes->payloadSize-6;
        int tail=0;
        pes->offset=start-pes->payload;
        if(packLen)
        {
            //printf("***Zimbla***\n");
            if((int)packLen<sizeCheck)
            {
                tail=sizeCheck-packLen;
                pes->payloadSize-=tail; 
                ADM_warning("[TS Packet]extra crap at the end %d\n",tail);
            }
            else
                if((int)packLen>sizeCheck)
                {
                    ADM_warning("[TS Packet] PackLen=%d, avalailble=%d\n",packLen,sizeCheck);
                    fail("Pes too long");
                }   
        }
        zprintf("[decodePesHeader] payloadSize=%d, offset=%d \n",pes->payloadSize,pes->offset);
        if(pes->offset>pes->payloadSize)
        {
            ADM_warning("[decodePesHeader] Inconsistent size, dropping\n");
            return false;
        }
        return true;
}
/**
    \fn getPacket
*/      
bool        tsPacket::getPacket(uint32_t maxSize, uint8_t *pid, uint32_t *packetSize,uint64_t *opts,uint64_t *odts,uint8_t *buffer,uint64_t *startAt)
{
    return true;
}
/** 
    \fn getNextPid
*/  
bool        tsPacket::getNextPid(int *pid)
{
 uint8_t scratch[188+4];
    int count=0;
nextPackx:

    if(false==getSinglePacket(scratch)) return false;
    uint32_t id=scratch[1]+((scratch[0]&0x1F)<<8);
    count++;
    if(count>MAX_SKIPPED_PACKET) return false;
    *pid=id;

    int fieldControl=(scratch[2]>>4)&3;

    if(!(fieldControl & 1)) 
    {
        goto nextPackx;
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
        goto nextPackx;
    }
    return true;
}

/**
    \fn tsPacket
*/
tsPacketLinear::tsPacketLinear(uint32_t pid) : tsPacket()
{
    oldStartAt=0xfffffff;
    oldBufferLen=0;
    pesPacket=new TS_PESpacket(pid);
    consumed=0;
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
                printf("[tsPacketLinear] Refill failed for pid : 0x%x (%d)\n",pesPacket->pid,pesPacket->pid);
                eof=true;
                return false;
        }
        eof=false;
        return true;
}
#ifndef TS_PACKET_INLINE
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
        return 0;
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
#endif
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
bool    tsPacketLinear::getInfo(dmxPacketInfo *info,int startCodeLength)
{
//#warning FIXME
    if(pesPacket->offset<startCodeLength)
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
    \fn invalidatePtsDts
    \brief in some cases (HDRunner/HDCPro), a pes contains multiple video frames. This call consumes pts
            and dts so that the multiple frames do not have the same pts/dts.
*/
bool     tsPacketLinear::invalidatePtsDts(int startCodeLength)
{
    if(pesPacket->offset<startCodeLength)
    {
        oldBufferPts=ADM_NO_PTS;
        oldBufferDts=ADM_NO_PTS;

    }else
    {
        pesPacket->pts=ADM_NO_PTS;
        pesPacket->dts=ADM_NO_PTS;
    }
    return true;
}
/**
    \fn seek
    \brief Async jump
*/
bool    tsPacketLinear::seek(uint64_t packetStart, uint32_t offset)
{
    if(!_file->setpos(packetStart))
    {
        printf("[tsPacket] Cannot seek to %" PRIx64"\n",packetStart);
        return 0;
    }
    if(!refill())
    {
        printf("[tsPacketLinear] Seek to %" PRIx64":%" PRIx32" failed\n",packetStart,offset);
        return false;
    }
    ADM_assert(offset<pesPacket->payloadSize);
    pesPacket->offset=offset;
    return true;
}
/**
    \fn setConsumed
    \brief set consumed bytes
*/
bool tsPacketLinear::setConsumed(uint64_t v)
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
    \fn resetStats
*/
bool tsPacketLinearTracker::resetStats(void)
{
    if(!stats) return false;
    for(uint32_t i=0; i<totalTracks; i++)
    {
        packetTSStats *s=stats+i;
        s->count=0;
        s->size=0;
        s->startAt=0;
        s->startCount=0;
        s->startSize=0;
        s->startDts=ADM_NO_PTS;
    }
    return true;
}
/**
    \fn findStartCode
    \brief Must check stillOk after calling this
*/
int tsPacketLinearTracker::findStartCode(void)
{
#define likely(x) x
#define unlikely(x) x
        unsigned int last=0xfffff;
        unsigned int cur=0xffff;
        int startCode=0;
        while(this->stillOk())
        {
            last=cur;
            cur=this->readi16();
            if(likely(last&0xff)) continue;
            if(unlikely(!last)) // 00 00 , need 01 xx
            {
                if((cur>>8)==1) 
                {
                        startCode=cur&0xff;
                        break;
                }
            }
            if(unlikely(!(last&0xff))) // xx 00 need 00 01
            {
                if(cur==1)
                {
                        startCode=this->readi8();
                        break;
                }
            }
        }
        return startCode;
}
/**
 * \fn fourByteStartCode
 * @param four
 * @return 
 */
int tsPacketLinearTracker::findStartCode2(bool &fourByteStartCode)
{
#define likely(x) x
#define unlikely(x) x
        unsigned int prev=0xfffff;
        unsigned int last=0xfffff;
        unsigned int cur=0xffff;
        int startCode=0;
        fourByteStartCode=false;
        while(this->stillOk())
        {
            prev=last;
            last=cur;
            cur=this->readi16();
            if(likely(last&0xff)) continue;
            if(unlikely(!last)) // 00 00 , need 01 xx
            {
                if((cur>>8)==1) 
                {
                        startCode=cur&0xff;
                        if(!(prev&0xff))
                            fourByteStartCode=true;
                        break;
                }
            }
            if(unlikely(!(last&0xff))) // xx 00 need 00 01
            {
                if(cur==1)
                {
                        startCode=this->readi8();
                        if(!(last>>8))
                            fourByteStartCode=true;
                        break;
                }
            }
        }
        return startCode;
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
                                        uint64_t pts1,pts2,pts0;
                                                //      printf("\n PTS10\n");
                                                pts0=start[0];  
                                                pts1=(start[1]<<8)+start[2]; 
                                                pts2=(start[3]<<8)+start[4]; 
                                                start+=5;
                                                                        
                                                // Assume PTS=DTS
                                                pts0=start[0];  
                                                pts1=(start[1]<<8)+start[2]; 
                                                pts2=(start[3]<<8)+start[4];       
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
/**
    \fn collectStats
    \brief Read from current position until we get the stats, then seek back
*/
bool tsPacketLinearTracker::collectStats(void)
{
    if(!resetStats()) // no audio tracks, nothing to do
        return false;

    bool success=false;
    uint32_t i,found=0,count=0;
    const uint32_t max=1<<24; // 16 MiB, should be enough
    const uint64_t remember=consumed;
    dmxPacketInfo info;
    getInfo(&info);

    const uint32_t len=sizeof(packetTSStats)*totalTracks;
    packetTSStats *first=(packetTSStats *)malloc(len);
    if(!first)
        return false;

    memset(first,0,len);
    for(i=0; i<totalTracks; i++)
    {
        first[i].startDts=ADM_NO_PTS;
    }

    while(count<max && stillOk())
    {
        count++;
        readi8();
        for(i=0; i<totalTracks; i++)
        {
            if(first[i].startAt) continue; // already set
            if(stats[i].startAt)
            {
#define CPY(x) first[i].x = stats[i].x;
                CPY(pid)
                CPY(count)
                CPY(size)
                CPY(startAt)
                CPY(startCount)
                CPY(startSize)
                CPY(startDts)
                found++;
#undef CPY
            }
        }
        if(found==totalTracks)
        {
            success=true;
            break;
        }
    }
    // Now sync back
    for(i=0; i<totalTracks; i++)
    {
        if(!first[i].startAt) continue;
#define CPY(x) stats[i].x = first[i].x;
        CPY(pid)
        CPY(count)
        CPY(size)
        CPY(startAt)
        CPY(startCount)
        CPY(startSize)
        CPY(startDts)
#undef CPY
    }
    free(first);
    first=NULL;

    ADM_info("Stats for %u tracks out of %u populated, bytes used: %u\n",found,totalTracks,count);
    consumed=remember;
    seek(info.startAt,info.offset);
    return success;
}
// EOF
