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



#define ADM_NO_CONFIG_H
extern "C"
{
#include "libavutil/common.h"
#include "libavutil/bswap.h"
#include "ADM_lavcodec/bitstream.h"

}
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
    return 0;
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

    if(id!=pid) goto nextPack;
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
extern void mixDump(uint8_t *ptr, uint32_t len);
bool        tsPacket::getNextPES(TS_PESpacket *pes)
{
#define zprintf printf
    TSpacketInfo pkt;
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
        printf("[Ts Demuxer] No PES startcode\n");
        goto nextPack3;
    }
    //mixDump(pkt.payload,pkt.payloadSize);
    uint32_t pesPacketLen=(pkt.payload[4]<<8)+(pkt.payload[5]);
    zprintf("[TS Demuxer] Pes Packet Len=%d\n",pesPacketLen);
    
    pes->payloadSize=0;
    pes->addData(pkt.payloadSize,pkt.payload);
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
    }
    // Now decode PES header (extra memcpy)
    printf("[Ts Demuxer] Full size :%d\n",pes->payloadSize);
    if(false==decodePesHeader(pes))
        goto nextPack3;

    //
    printf("[Ts Demuxer] Found PES of len %d\n",pes->payloadSize);
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
    uint32_t packLen=(pes->payload[4])<<8+(pes->payload[5]);
    int      align=0,c;


    pes->dts=ADM_NO_PTS;
    pes->pts=ADM_NO_PTS;

    
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

        switch(ptsdts)
        {
                case 2: // PTS=1 DTS=0
                       
                        {
                                uint64_t pts1,pts2,pts0;
                                //      printf("\n PTS10\n");
                                        pts0=*start++;  
                                        pts1=*start++; 
                                        pts2=*start++;                 
                                        pes->pts=(pts1>>1)<<15;
                                        pes->pts+=pts2>>1;
                                        pes->pts+=(((pts0&6)>>1)<<30);
                        }
                        break;
                case 3: // PTS=1 DTS=1
                                #define PTS11_ADV 10 // nut monkey
//                                if(len>=PTS11_ADV)
                                {
                                        uint32_t skip=PTS11_ADV;
                                        uint64_t pts1,pts2,dts,pts0;
                                                //      printf("\n PTS10\n");
                                                pts0=*start++;  
                                                pts1=*start++;; 
                                                pts2=(start[0]<<8)+start[1]; 
                                                start+=2;
                                                                        
                                                pes->pts=(pts1>>1)<<15;
                                                pes->pts+=pts2>>1;
                                                pes->pts+=(((pts0&6)>>1)<<30);
                                                pts0=*start++;  
                                                pts1=(start[0]<<8)+start[1]; 
                                                pts2=(start[2]<<8)+start[3];       
                                                start+=4;
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
        int maxLen=(int)(end-start);
        pes->offset=start-pes->payload;
        if(packLen)
        {
            if(packLen<maxLen) maxLen=packLen;
            else
                if(packLen>maxLen)
                {
                    fail("Pes too long");
                }   
        }
        pes->payloadSize=maxLen;
        return true;
}
/**
    \fn getPacket
*/      
bool        tsPacket::getPacket(uint32_t maxSize, uint8_t *pid, uint32_t *packetSize,uint64_t *opts,uint64_t *odts,uint8_t *buffer,uint64_t *startAt)
{
uint32_t globstream,len;
uint8_t  stream,substream;
uint64_t pts,dts;
        // Resync on our stream
_again2:
        *pid=0;
        if(!_file->sync(&stream)) 
        {
                uint64_t pos;
                _file->getpos(&pos);
                printf("[DmxPS] cannot sync  at "LLU"/"LLU"\n",pos,_size);
                return false;
        }
// Position of this packet just before startcode
        _file->getpos(startAt);
        *startAt-=4;
// Handle out of band stuff        
        if(stream==PACK_START_CODE) 
        {
        		_file->forward(8);
        		goto _again2;
        }
        if( stream==PADDING_CODE ||stream==SYSTEM_START_CODE) 
        {
                        len=_file->read16i();
                        //printf("\tForwarding %lu bytes\n",len);
        		_file->forward(len);
        		goto _again2;
        }
        // Only keep relevant parts
        // i.e. a/v : C0 C9 E0 E9
        // subs 20-29
        // private data 1/2
#define INSIDE(min,max) (stream>=min && stream<max)
        if(!(  INSIDE(0xC0,0xC9) || INSIDE(0xE0,0xE9) || INSIDE(0x20,0x29) || stream==PRIVATE_STREAM_1 || stream==PRIVATE_STREAM_2
        			)) goto _again2;
        // Ok we got a candidate
        if(!getPacketInfo(stream,&substream,&len,&pts,&dts))   
        {
                goto _again2;
        }
        
        //printf("Main Stream :%x substream :%x\n",stream,substream);
        if(stream==PRIVATE_STREAM_1) globstream=0xFF00+substream;
                else                 globstream=stream;

        *pid=globstream;
        *opts=pts;
        *odts=dts;
        *packetSize=len;
        if(len>     maxSize)
        {
                printf("[DmxPS] Packet too big %d vs %d\n",len,maxSize);
        }
        if(!_file->read32(len,buffer)) return false;
        return true;
       
}
/**

    \fn getPacketInfo
    \brief       Retrieve info about the packet we just met.It is assumed that parser is just after the packet startcode

*/

uint8_t tsPacket::getPacketInfo(uint8_t stream,uint8_t *substream,uint32_t *olen,uint64_t *opts,uint64_t *odts)
{

//uint32_t un ,deux;
uint64_t size=0;
uint8_t c,d;
uint8_t align=0;
                        
                *substream=0xff;
                *opts=ADM_NO_PTS;
                *odts=ADM_NO_PTS;
                
                                        
                size=_file->read16i();
                if((stream==PADDING_CODE) || 
                	 (stream==PRIVATE_STREAM_2)
                        ||(stream==SYSTEM_START_CODE) //?
                        ) // special case, no header
                        {
                                *olen=size;      
                                return 1;
                        }
                                
                        //      remove padding if any                                           
        
                while((c=_file->read8i()) == 0xff) 
                {
                        size--;
                }
//----------------------------------------------------------------------------
//-------------------------------MPEG-2 PES packet style----------------------
//----------------------------------------------------------------------------
                if(((c&0xC0)==0x80))
                {
                        uint32_t ptsdts,len;
                        //printf("\n mpeg2 type \n");
                        //_muxTypeMpeg2=1;
                        // c= copyright and stuff       
                        //printf(" %x align\n",c);      
                        if(c & 4) align=1;      
                        c=_file->read8i();     // PTS/DTS
                        //printf("%x ptsdts\n",c
                        ptsdts=c>>6;
                        // header len
                        len=_file->read8i();
                        size-=3;  

                        switch(ptsdts)
                        {
                                case 2: // PTS=1 DTS=0
                                        if(len>=5)
                                        {
                                                uint64_t pts1,pts2,pts0;
                                                //      printf("\n PTS10\n");
                                                        pts0=_file->read8i();  
                                                        pts1=_file->read16i(); 
                                                        pts2=_file->read16i();                 
                                                        len-=5;
                                                        size-=5;
                                                        *opts=(pts1>>1)<<15;
                                                        *opts+=pts2>>1;
                                                        *opts+=(((pts0&6)>>1)<<30);
                                        }
                                        break;
                                case 3: // PTS=1 DTS=1
                                                #define PTS11_ADV 10 // nut monkey
                                                if(len>=PTS11_ADV)
                                                {
                                                        uint32_t skip=PTS11_ADV;
                                                        uint64_t pts1,pts2,dts,pts0;
                                                                //      printf("\n PTS10\n");
                                                                pts0=_file->read8i();  
                                                                pts1=_file->read16i(); 
                                                                pts2=_file->read16i(); 
                                                                                        
                                                                *opts=(pts1>>1)<<15;
                                                                *opts+=pts2>>1;
                                                                *opts+=(((pts0&6)>>1)<<30);
                                                                pts0=_file->read8i();  
                                                                pts1=_file->read16i(); 
                                                                pts2=_file->read16i();                 
                                                                dts=(pts1>>1)<<15;
                                                                dts+=pts2>>1;
                                                                dts+=(((pts0&6)>>1)<<30);
                                                                len-=skip;
                                                                size-=skip;
                                                                *odts=dts;
                                                                        //printf("DTS: %lx\n",dts);                
                                                   }
                                                   break;               
                                case 1:
                                                return 0;//ADM_assert(0); // forbidden !
                                                break;
                                case 0: 
                                                // printf("\n PTS00\n");
                                                break; // no pts nor dts
                                                                                
                                                            
                        }  
// Extension bit        
// >stealthdave<                                

                        // Skip remaining headers if any
                        if(len) 
                        {
                                _file->forward(len);
                                size=size-len;
                        }
                                
                if(stream==PRIVATE_STREAM_1)
                {
                        if(size>5)
                        {
                        // read sub id
                               *substream=_file->read8i();
  //                    printf("\n Subid : %x",*subid);
                                switch(*substream)
                                {
                                // DTS
                                        case 0x88:case 0x89:case 0x8A:case 0x8B:
                                        
                                                *substream=*substream-0x48;
                                                break;

                                //AC3
                                        case 0x80:case 0x81:case 0x82:case 0x83:
                                        case 0x84:case 0x85:case 0x86:case 0x87:
                                                *substream=*substream-0x80;
                                                break;
                                // PCM
                                        case 0xA0:case 0xA1:case 0xa2:case 0xa3:
                                        case 0xA4:case 0xA5:case 0xa6:case 0xa7:
                                                // we have an additionnal header
                                                // of 3 bytes
                                                _file->forward(3);
                                                size-=3;
                                                break;
                                // Subs
                                case 0x20:case 0x21:case 0x22:case 0x23:
                                case 0x24:case 0x25:case 0x26:case 0x27:
                                                break;
                             
                                default:
                                                doNoComplainAnyMore++;
                                                if(doNoComplainAnyMore<10)
                                                    printf("[DmxPS]Unkown substream %x\n",*substream);
                                                *substream=0xff;
                                }
                                // skip audio header (if not sub)
                                if(*substream>0x26 || *substream<0x20)
                                {
                                        _file->forward(3);
                                        size-=3;
                                }
                                size--;
                        }
                }
               //    printf(" pid %x size : %x len %x\n",sid,size,len);
                *olen=size;
                return 1;
        }
//----------------------------------------------------------------------------------------------                
//-------------------------------MPEG-1 PES packet style----------------------                                  
//----------------------------------------------------------------------------------------------                                        
           if(0) //_muxTypeMpeg2)
                {
                        printf("[DmxPS]*** packet type 1 inside type 2 ?????*****\n");
                        return 0; // mmmm                       
                }
          // now look at  STD buffer size if present
          // 01xxxxxxxxx
          if ((c>>6) == 1) 
          {       // 01
                        size-=2;
                        _file->read8i();                       // skip one byte
                        c=_file->read8i();   // then another
           }                       
           // PTS/DTS
           switch(c>>4)
           {
                case 2:
                {
                        // 0010 xxxx PTS only
                        uint64_t pts1,pts2,pts0;
                                        size -= 4;
                                        pts0=(c>>1) &7;
                                        pts1=_file->read16i()>>1;
                                        pts2=_file->read16i()>>1;
                                        *opts=pts2+(pts1<<15)+(pts0<<30);
                                        break;
                  }
                  case 3:
                  {               // 0011 xxxx
                        uint64_t pts1,pts2,pts0;
                                        size -= 9;
                                                                        
                                        pts0=(c>>1) &7;
                                        pts1=_file->read16i()>>1;
                                        pts2=_file->read16i()>>1;
                                        *opts=pts2+(pts1<<15)+(pts0<<30);
                                        _file->forward(5);
                   }                                                               
                   break;
                   
                case 1:
                        // 0001 xxx             
                        // PTSDTS=01 not allowed                        
                                return 0;
                                break; 
                }
                                                                

                if(!align)      
                        size--;         
        *olen=size;
        return 1;
}
//************************************************************************************

#define ADM_PACKET_LINEAR 10*1024
/**
    \fn tsPacket
*/
tsPacketLinear::tsPacketLinear(uint8_t pid) : tsPacket()
{
    oldStartAt=startAt=0xfffffff;
    oldBufferLen=bufferLen=0;
    bufferIndex=0;
    myPid=pid;
    eof=false;
}
/**
    \fn ~tsPacket
*/
tsPacketLinear::~tsPacketLinear() 
{
}
/**
    \fn refill
*/
bool tsPacketLinear::refill(void) 
{
// In case a startcode spawns across 2 packets
// we have to keep track of the old one
        oldBufferDts=bufferDts;
        oldBufferPts=bufferPts;
        oldStartAt=startAt;
        oldBufferLen=bufferLen;
        if( false== getPacketOfType(myPid,ADM_PACKET_LINEAR, &bufferLen,&bufferPts,&bufferDts,buffer,&startAt)) 
        {
            printf("[tsPacketLinear] Refill failed for pid :%x\n",myPid);
            bufferIndex=bufferLen=0;
            return false;
        }
        //printf("Refill : At :%"LLX" size :%"LD"\n",startAt,bufferLen);
        bufferIndex=0;
        return true;
}
/**
    \fn readi8
*/
uint8_t tsPacketLinear::readi8(void)
{
    consumed++;
    if(bufferIndex<bufferLen)
    {
        return buffer[bufferIndex++];
    }
    if(false==refill()) 
    {
        eof=1;
        return 0;
    }
    ADM_assert(bufferLen);
    bufferIndex=1;
    return buffer[0];
    
}
/**
    \fn readi16
*/
uint16_t tsPacketLinear::readi16(void)
{
    if(bufferIndex+1<bufferLen)
    {
        uint16_t v=(buffer[bufferIndex]<<8)+buffer[bufferIndex+1];;
        bufferIndex+=2;
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
    if(bufferIndex+3<bufferLen)
    {
        uint8_t *p=buffer+bufferIndex;
        uint32_t v=(p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
        bufferIndex+=4;
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
 uint32_t delta=bufferLen-bufferIndex;
    if(v>100*1000)
    {
        ADM_assert(0);
    }
    if(v<=delta)
    {
        bufferIndex+=v;
        consumed+=v;
        return true;
    }
    // v>delta
    v-=delta;
    if(!refill()) return false;
    goto next;
}

/**
    \fn bool    read(uint32_t len, uint8_t *buffer);
    \brief
*/
bool    tsPacketLinear::read(uint32_t len, uint8_t *out)
{
    // Enough already ?
    while(len)
    {
        uint32_t avail=bufferLen-bufferIndex;
        uint32_t chunk=avail;
        if(chunk>len) chunk=len;
#if 0
        printf("len:%ld avail:%ld chunk %ld index:%d size:%d\n",
                len,avail,chunk,bufferIndex,bufferLen);
#endif
        memcpy(out,buffer+bufferIndex,chunk);
        bufferIndex+=chunk;
        len-=chunk;
        out+=chunk;
        consumed+=chunk;
        if(bufferIndex==bufferLen)
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
    if(bufferIndex<4)
    {
        info->startAt=this->oldStartAt;
        info->offset=oldBufferLen+bufferIndex;
        info->pts=oldBufferPts;
        info->dts=oldBufferDts;

    }else
    {
        info->startAt=this->startAt;
        info->offset=bufferIndex;
        info->pts=bufferPts;
        info->dts=bufferDts;
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
    ADM_assert(offset<bufferLen);
    bufferIndex=offset;
    
    return true;
}
/**
    \fn getConsumed
    \brief returns the # of bytes consumed since the last call
*/
uint32_t tsPacketLinear::getConsumed(void)
{
    uint32_t c=consumed;
    consumed=0;
    return c;
}
/**
    \fn changePid
    \brief change the pid of the stream we read (used when probing all tracks)
*/
bool    tsPacketLinear::changePid(uint32_t pid) 
{
    myPid=(pid&0xff);
    bufferLen=bufferIndex=0;
    return true;
}
/* ********************************************************* */
/**
    \fn tsPacketLinearTracker
*/
 tsPacketLinearTracker::tsPacketLinearTracker(uint8_t pid)  : tsPacketLinear(pid)
{
   resetStats();
}
/**
    \fn ~tsPacketLinearTracker
*/
tsPacketLinearTracker::~tsPacketLinearTracker()
{

    
}
/**
        \fn getStat
*/
packetStats    *tsPacketLinearTracker::getStat(int index)
{   
    if(index<0 || index>=256) ADM_assert(0);
    return stats+index;
}
/**
    \fn getPacketgetPacketOfType
    \brief Keep track of all the packets we have seen so far.
    Usefull to detect the streams present and to look up the PTS/DTS of audio streams for the audio part of the index
*/
bool           tsPacketLinearTracker::getPacketOfType(uint8_t pid,uint32_t maxSize, uint32_t *packetSize,uint64_t *pts,uint64_t *dts,uint8_t *buffer,uint64_t *startAt)
{
 bool xit=false;
    uint8_t tmppid;
    while(1)
    {
        if(true!=getPacket(maxSize,&tmppid,packetSize,pts,dts,buffer,startAt))
                return false;
        else
        {
                // Update 
                ADM_assert(tmppid<0x100);
                packetStats *p=stats+tmppid;
                uint64_t ts=*pts;
                if(ts==ADM_NO_PTS) ts=*dts;
                if(ts!=ADM_NO_PTS)
                {
                    p->startCount=p->count;
                    p->startAt=*startAt;
                    p->startSize=p->size;
                    p->startDts=ts;
                }
                p->count++;
                p->size+=*packetSize;
                if(tmppid==pid) return true;
        }
    }
    return false;
}
/**
    \fn resetStats
*/
bool           tsPacketLinearTracker::resetStats(void)
{
    memset(stats,0,sizeof(stats));
    for(int i=0;i<256;i++)
    {
        packetStats *p=stats+i;
        p->startDts=ADM_NO_PTS;
    }
}
//EOF
