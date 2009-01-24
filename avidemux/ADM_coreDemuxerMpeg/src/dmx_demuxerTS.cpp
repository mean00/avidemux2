/***************************************************************************
                          TS demuxer
                             -------------------

    copyright            : (C) 2005 by mean
    email                : fixounet@free.fr

        Here we try to read a whole PES packet at a time...
        So we concatenate all TS packet to get it
        Else it is a pain to handle potentiel padding bytes

        A special case, if the PES size is 0, it means it is an unbound
        PES packet, and so the demuxer must guess the size when encountering
        the next TS packet having the payload unit flag
        It is a way to overcome the 64k size limit of the PES packetization
        In that case the padding must be in the adaptation layer bytes.


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include <math.h>

#include "ADM_default.h"

//#define TS_VERBOSE 1

#include "dmx_mpegstartcode.h"
#include "dmx_demuxerTS.h"
uint8_t         dmx_demuxerTS::changePid(uint32_t newpid,uint32_t newpes)
{
          myPid=newpid;
          // Be sure our new Pid is taken care of
          allPid[newpid]=1;

        _pesBufferStart=0;  // Big value so that we read
        _pesBufferLen=0;
        _pesBufferIndex=0;
}
dmx_demuxerTS::dmx_demuxerTS(uint32_t nb,MPEG_TRACK *tracks,uint32_t psi,DMX_TYPE muxType)
{
        consumed=0;
        parser=new fileParser();
        stampAbs=0;
        _pesBuffer=new uint8_t [MAX_TS_BUFFER];

        memset(seen,0,sizeof(seen));
        memset(allPid,0,sizeof(allPid));
        _pesBufferStart=0;  // Big value so that we read
        _pesBufferLen=0;
        _pesBufferIndex=0;
        ADM_assert(nb>0);
        tracked=NULL;
        nbTracked=nb;
        myPid=tracks[0].pid; // For mpeg TS we use the PID field as the PES field is irrelevant
        printf("Ts: Using %x as pid for track 0\n",myPid);

        // Build reverse lookup
        if(nb==TS_ALL_PID)
                for(int i=0;i<nb;i++)
                    allPid[ i ]=1+i;
        else
                for(int i=0;i<nb;i++)
                {
                        allPid[ tracks[i].pid ]=1+i;
                }

        _probeSize=0;
        packMode=0;
        packLen=0;
        isPsi=psi;
        switch(muxType)
        {
          case DMX_MPG_TS: TS_PacketSize=TS_PACKET_SIZE;break;
          case DMX_MPG_TS2: TS_PacketSize=TS2_PACKET_SIZE;break;
          default: ADM_assert(0);
        }
        printf("Creating mpeg TS demuxer  main Pid: %X , pes id :%x, packet size=%u\n",myPid,tracks[0].pes,TS_PacketSize);
}
dmx_demuxerTS::~dmx_demuxerTS()
{
        if(parser) delete parser;
        parser=NULL;
        if(_pesBuffer) delete [] _pesBuffer;
        _pesBuffer=NULL;
        if(tracked) delete [] tracked;
        tracked=NULL;
}
/*
        Get stats about the PES ids tracked in tracked order
        if nbTracked=256 it means tracks all possible PES id
        The 0 rank is video
*/
uint8_t       dmx_demuxerTS::getStats(uint64_t *oseen)
{
        for(int i=0;i<nbTracked;i++)
        {
                oseen[i]=seen[i];
        }
        return 1;
}
uint8_t dmx_demuxerTS::setProbeSize(uint32_t sz)
{
		_probeSize=sz;
		return 1;
}
uint8_t dmx_demuxerTS::open(const char *name)
{
FP_TYPE fp=FP_DONT_APPEND;

        if(! parser->open(name,&fp)) return 0;
        _size=parser->getSize();
        return 1;
}
uint8_t dmx_demuxerTS::forward(uint32_t f)
{
uint32_t left;
        if(_pesBufferIndex+f<=_pesBufferLen)
        {
                _pesBufferIndex+=f;
                consumed+=f;
                return 1;
        }
        // else read another packet
        left=_pesBufferLen-_pesBufferIndex;
        f-=left;
        consumed+=left;
        if(!refill()) return 0;
        return forward(f);
}
uint8_t  dmx_demuxerTS::stamp(void)
{
        consumed=0;
}
uint64_t dmx_demuxerTS::elapsed(void)
{
        return consumed;
}
uint8_t  dmx_demuxerTS::getPos( uint64_t *abs,uint64_t *rel)
{
        *rel=_pesBufferIndex;
        *abs=_pesBufferStart;
        return 1;
}
uint8_t dmx_demuxerTS::setPos( uint64_t abs,uint64_t  rel)
{
        // Still in same packet ?
        if(abs==_pesBufferStart && _pesBufferLen)
        {
                if(_pesBufferLen<rel)
                {
                        printf("Asked setpos to go %"LLU" whereas %"LU" is max\n",
                                rel,_pesBufferLen);
                        ADM_assert(rel<_pesBufferLen);
                }

                _pesBufferIndex=rel;
                return 1;
        }
        // There is a risk we don't get the PES start for that
        //
        packMode=0;
        if(!parser->setpos(abs))
        {
                printf("DMX_TS: setPos failed\n");
                return 0;
        }
        _pesBufferStart=abs;
        if(!refill())
        {
                printf("DMX_TS: refill failed\n");
                return 0;
        }

        if(rel>_pesBufferLen)
        {
                printf("Set pos failed : asked rel:%"LLU" max: %"LU", absPos:%"LLU" absPosafterRefill:%"LLU"\n",
                                        rel,_pesBufferLen,abs,_pesBufferStart);
                ADM_assert(rel<_pesBufferLen);
        }

        _pesBufferIndex=rel;
        return 1;
}
/*
        Sync on mpeg sync word, returns the sync point in abs/r
*/


uint32_t         dmx_demuxerTS::read(uint8_t *w,uint32_t len)
{
uint32_t mx;
                // enough in buffer ?
                if(_pesBufferIndex+len<=_pesBufferLen)
                {
                        memcpy(w,_pesBuffer+_pesBufferIndex,len);
                        _pesBufferIndex+=len;
                        consumed+=len;
                        return len;
                }
                // flush
                mx=_pesBufferLen-_pesBufferIndex;
                if(mx)
                {
                        memcpy(w,_pesBuffer+_pesBufferIndex,mx);
                        _pesBufferIndex+=mx;
                        consumed+=mx;
                        w+=mx;
                        len-=mx;
                }
                if(!refill())
                {
                        _lastErr=1;
                         return mx;
                }
                return mx+read(w,len);
}
uint8_t         dmx_demuxerTS::sync( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts,uint64_t *dts)
{
uint32_t val,hnt;
retry:
         *r=0;

                val=0;
                hnt=0;

                // preload
                hnt=(read8i()<<16) + (read8i()<<8) +read8i();
                if(_lastErr)
                {
                        _lastErr=0;
                        printf("\n io error , aborting sync\n");
                        return 0;
                }

                while((hnt!=0x00001))
                {

                        hnt<<=8;
                        val=read8i();
                        hnt+=val;
                        hnt&=0xffffff;

                        if(_lastErr)
                        {
                             _lastErr=0;
                            printf("\n io error , aborting sync\n");
                            return 0;
                         }

                }

                *stream=read8i();
                // Case 1 : assume we are still in the same packet
                if(_pesBufferIndex>=4)
                {
                        *abs=_pesBufferStart;
                        *r=_pesBufferIndex-4;
                        *pts=_pesPTS;
                        *dts=_pesDTS;
                }
                else
                {       // pick what is needed from oldPesStart etc...
                        // since the beginning in the previous packet
                        uint32_t left=4-_pesBufferIndex;
                                 if(left>_oldPesLen)
                                 { // previous Packet which len is very shoty
                                   // Ignore
                                   _pesBufferIndex=0;
                                   printf("Ignoring too short packet");
                                   goto retry;
                                 }
                                 left=_oldPesLen-left;
#if 0
                                 printf("Next packet : %I64X Len :%"LU", using previous packet %I64X len:%u as pos=%"LU"\n",
                                 		_pesBufferStart,_pesBufferLen,_oldPesStart,_oldPesLen,_pesBufferIndex);
#endif
                                 if(left>_oldPesLen)
                                {
                                        printf("Need %"LU" bytes from previous packet, which len is %"LU"\n",left,_oldPesLen);
                                        ADM_assert(0);
                                }
                                *abs=_oldPesStart;
                                *r=left;
                                *pts=_oldPTS;
                                *dts=_oldDTS;
                }
                return 1;
}
/**
      \fn readPes
      \brief Read a complete PES packet
*/
uint8_t dmx_demuxerTS::readPes(uint8_t *data, uint32_t *pesBlockLen, uint32_t *dts,uint32_t *pts)
{
  uint32_t total=0;
    if(!refill())
    {
      printf("[DMX] Refill failed\n");
      return 0;
    }
    *dts=_pesDTS;
    *pts=_pesPTS;
    total=_pesBufferLen;
    memcpy(data,_pesBuffer,_pesBufferLen);
    while(packLen)
    {
        if(!refill())
        {
          printf("[DMX] Refill failed\n");
          return 0;
        }

        memcpy(data+total,_pesBuffer,_pesBufferLen);
        total+=_pesBufferLen;

    }
    *pesBlockLen=total;
    printf("[DMX] Read %d bytes, packMode %u, pesLen %u\n",*pesBlockLen,packMode,packLen);
    return 1;
}
//
//      Refill the pesBuffer
//              Read packet of correct PID, locate a PES start and read the whole PES packet
//              It cannot be bigger than 64 k in bound mode, in that case packMode=1, packLen is the leftover to read
//
//
uint8_t dmx_demuxerTS::refill(void)
{
uint32_t consumed,len,pid,payload=0;
uint8_t  stream,substream;
uint64_t count,abs,pts,dts,first;
uint32_t left,cc,lenPes;

        _pesBufferIndex=0;


_againBranch:
        while(1)
        {
                if(!readPacket(&pid,&left, &payload,&abs,&cc))
                {
                        printf("dmxTs: Cannot read packet (1) at %"LLX"\n",abs);
                        return 0;
                }
                if(allPid[pid])
                        break;
                parser->forward(left); // Else skip packet
        }

        if(!payload) // Take as is...
        {
                if(pid==myPid)
                {
                        // No payloadStart, read it raw
                        _oldPesStart=_pesBufferStart;
                        _oldPesLen=_pesBufferLen;
                        _pesBufferStart=abs;

                        parser->read32(left,_pesBuffer);
                        // FIXME HACK
                        if(TS_PacketSize==192 && left >4)
                          {
                            left-=4; // Remove timestamp of m2ts packet
                          }
                       // FIXME HACK
                        _pesBufferLen=left;


                        _pesPTS=ADM_NO_PTS;
                        _pesDTS=ADM_NO_PTS;
                        // If we are in pack mode, cut padding bits
                        if(packMode)
                        {
                                if(packLen<left)
                                {
#if 1 //def 1  TS_VERBOSE
                                        printf("Dropping some bytes : %"LU" / %"LU"\n",_pesBufferLen,packLen);
#endif
                                         _pesBufferLen=packLen;
                                }
#if 0
                                printf("at %llx, packLen=%"LU" minus %"LU"\n",_pesBufferStart,packLen,_pesBufferLen);
#endif
                                packLen-=_pesBufferLen;

                                if(!packLen)
                                {
                                  packMode=0;
                                }
                        }
                        return 1;
                }
                // Udate info on that track
                updateTracker(pid,left);
                parser->forward(left);
                goto _againBranch;
        }
        // Payload present, read header
#ifdef TS_VERBOSE
        parser->getpos(&first);
        printf("BF: left:%"LU" delta :%"LLU"\n",left,first-abs);
#endif
        if(pid==myPid && isPsi)
        {
                if(!getInfoPSI(&consumed,&lenPes))
                        goto _againBranch;
                if(left<consumed)
                        goto _againBranch;
                left-=consumed;
                _pesBufferStart=abs;


                _pesPTS=ADM_NO_PTS;
                _pesDTS=ADM_NO_PTS;

                parser->read32(left,_pesBuffer);
                 // FIXME HACK
                if(TS_PacketSize==192 && left >4)
                  {
                    left-=4; // Remove timestamp of m2ts packet
                  }
                _pesBufferLen=left;
                 // FIXME HACK
                return 1;
        }


        if(!getInfoPES(&consumed,&dts,&pts,&stream,&substream,&lenPes))
        {
                        printf("dmxTs: get info failed at %"LLX"\n",abs);
                        goto _againBranch;
        }
#ifdef TS_VERBOSE
        printf("Stream :%x found at %"LLX" size :%"LU"\n",stream,abs,lenPes);
        parser->getpos(&count);
        printf("consumed :%"LU" left:%"LU" delta :%"LLU"\n",consumed,left,count-first);
        if(count-first!=consumed) printf("*** PES header length is wrong***\n");

#endif
        if(consumed>left)
        {
                printf("Wrong PES header at %"LLX" %"LU" / %"LU"\n",abs,consumed,left);
                goto _againBranch;
        }

        left-=consumed;
        if(myPid==pid)
        {
                if(lenPes)
                {
                        packMode=1;
                        packLen=lenPes;
                }
                else
                {
                        packMode=0;
                }

                _oldPesStart=_pesBufferStart;
                _oldPesLen=_pesBufferLen;

                _pesBufferStart=abs;


                _pesPTS=pts;
                _pesDTS=dts;

                parser->read32(left,_pesBuffer);
                  // FIXME HACK
                if(TS_PacketSize==192 && left >4)
                  {
                    left-=4; // Remove timestamp of m2ts packet
                  }
                _pesBufferLen=left;
                if(packMode)
                {
                        if(packLen<left) _pesBufferLen=packLen;
                        packLen-=_pesBufferLen;
                        if(!packLen) packMode=0;
                }
                return 1;
        }
        // update info
        updateTracker(pid,left);
        parser->forward(left);
        goto _againBranch;
}
//***********************************
// Read a Ts packet and extract
// interesting infos
//***********************************
uint8_t dmx_demuxerTS::readPacket(uint32_t *opid,uint32_t *oleft, uint32_t *isPayloadStart,uint64_t *ostart,uint32_t *occ)
{
uint32_t consumed,len,pid,payloadunit=0,discarded;
uint64_t count,abs;
uint32_t left;
uint8_t  byte1,byte2;
        parser->getpos(&count);
#ifdef TS_VERBOSE
        printf("Starting sync at %"LLX"\n",count);
#endif
_again:
        payloadunit=0;
        parser->getpos(&count);
        count=_size-count;
        discarded=0;
        while(parser->read8i()!=0x47 && count>TS_PacketSize)
                {
                        discarded++;
                        count--;
                }
        if(count<TS_PacketSize)
        {
                printf("DmxTS: cannot sync (EOF reached) \n");
                _lastErr=1;
                return 0;
        }
        // Check that there is a 0x47 later on...
        parser->getpos(&abs);
        if(discarded) // We did not have a continuous sync, check 2 more packet boundaries..
        {

                parser->forward(TS_PacketSize-1);
                byte1=parser->read8i();
                parser->forward(TS_PacketSize-1);
                byte2=parser->read8i();
                parser->setpos(abs);  // The setpos/getpos is mostly free due to the parser large buffer
                if(byte1!=0x47 || byte2!=0x47) goto _again;
        }
#ifdef TS_VERBOSE
        printf("Sync at %"LLX"\n",abs-1);
#endif
        // Memorize where it starts
        *ostart=abs-1;
        // Read Pid etc...
        pid=parser->read16i();
        if((pid>>8) & TS_UNIT_START) payloadunit=1;
        pid&=0x1fff; // remove flags
        if(discarded)
                printf("Ts: Discontinuity of %"LU" at %"LLX" pid:%"LX"\n",discarded,abs,pid);
        // Start of packet..
        left=TS_PacketSize-3;
        if(_probeSize)
        {
                if(abs>_probeSize)
                {
                		printf("dmxTs:Probe exceeded\n");
                		return 0;
                }
        }
        // Ok now get some informations....
        // only interested in my Pid & user data Pid
        if(pid!=myPid && pid<0x10)
        {
                parser->forward(left);
                goto _again;
        }
        // One of the stream we are looking for ?
        if(!allPid[pid])
        {
                parser->forward(left);
                goto _again; // No
        }
        // Remove header if any

        int cc,val,adaptation;
        // Flags : adaptation layer + continuity counter etc...
        val=parser->read8i();
        left--;
        cc=val & 0xf;
        adaptation=(val >>4)&0x3;
        if(!(adaptation & 1)) // no payload
        {
                parser->forward(left);
                goto _again;
        }
        if(adaptation & 2) // There is an adaptation field
        {
                val=parser->read8i();
                left--;
                if(val>=left)
                {
                 printf("Wrong adaptation layer size at %"LLX" size=%"LU", bytes left = %"LU" pid=%"LX"\n",abs,val,left,pid);
                 goto _again; // need to search..
                }
                parser->forward(val); // skip adaptation field
                left-=val;
        }
        // Ok now we got the raw data packet
        *oleft=left;
        *opid=pid;
        *occ=cc;

        if(payloadunit) // A PSI or PES packet starts here
        {
                *isPayloadStart=1;
        }else
                *isPayloadStart=0;
        return 1;
}
/*
        Retrieve info about the packet we just met
        It is assumed that parser is just after the packet startcode

*/
uint8_t       dmx_demuxerTS::updateTracker(uint32_t trackerPid,uint32_t nbData)
{
        seen[(allPid[trackerPid])-1]+=nbData;
        return 1;
}

uint8_t       dmx_demuxerTS::getInfoPES(uint32_t *oconsumed,uint64_t *odts,uint64_t *opts,
                                        uint8_t *ostream,uint8_t *substream,
                                        uint32_t *olen)
{

uint32_t headconsumd=0;
int size=0,nulsize=0;
uint8_t c,d;
uint8_t align=0;
                *oconsumed=0;

                // Check it looks like a PES header
                // It could be a PSI header ...
                if(parser->read8i()) return 0;
                if(parser->read8i()) return 0;
                if(parser->read8i()!=1) return 0;
                *ostream=parser->read8i(); // Stream
                headconsumd=4;
//

                *substream=0xff;
                *opts=ADM_NO_PTS;
                *odts=ADM_NO_PTS;


                size=parser->read16i();
                headconsumd+=2;
                if(!size) nulsize=1;
                if((*ostream==PADDING_CODE) ||
                	 (*ostream==PRIVATE_STREAM_2)
                        ||(*ostream==SYSTEM_START_CODE) //?
                        ) // special case, no header
                        {
                                if(nulsize) size=0;
                                *olen=size;
                                *oconsumed=headconsumd;
                                return 1;
                        }

                        //      remove padding if any

                while((c=parser->read8i()) == 0xff)
                {
                        headconsumd++;
                        size--;
                }
//----------------------------------------------------------------------------
//-------------------------------MPEG-2 PES packet style----------------------
//----------------------------------------------------------------------------
                if(((c&0xC0)!=0x80))
                {
                        printf("DmxTs: Not mpeg2 PES!\n");
                        return 0;
                }

                        uint32_t ptsdts,len;
                        //printf("\n mpeg2 type \n");
                        //_muxTypeMpeg2=1;
                        // c= copyright and stuff
                        //printf(" %x align\n",c);
                        if(c & 4) align=1;
                        c=parser->read8i();     // PTS/DTS
                        //printf("%x ptsdts\n",c
                        ptsdts=c>>6;
                        // header len
                        len=parser->read8i();
                        size-=3;
                        headconsumd+=3;

                        switch(ptsdts)
                        {
                                case 2: // PTS=1 DTS=0
                                        if(len>=5)
                                        {
                                                uint64_t pts1,pts2,pts0;
                                                //      printf("\n PTS10\n");
                                                        pts0=parser->read8i();
                                                        pts1=parser->read16i();
                                                        pts2=parser->read16i();
                                                        len-=5;
                                                        size-=5;
                                                        headconsumd+=5;
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
                                                                pts0=parser->read8i();
                                                                pts1=parser->read16i();
                                                                pts2=parser->read16i();

                                                                *opts=(pts1>>1)<<15;
                                                                *opts+=pts2>>1;
                                                                *opts+=(((pts0&6)>>1)<<30);
                                                                pts0=parser->read8i();
                                                                pts1=parser->read16i();
                                                                pts2=parser->read16i();
                                                                dts=(pts1>>1)<<15;
                                                                dts+=pts2>>1;
                                                                dts+=(((pts0&6)>>1)<<30);
                                                                len-=skip;
                                                                size-=skip;
                                                                *odts=dts;
                                                                headconsumd+=10;
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
                                parser->forward(len);
                                size=size-len;
                                headconsumd+=len;
                        }


               //    printf(" pid %x size : %x len %x\n",sid,size,len);
                if(nulsize) size=0;
                *olen=size;
                *oconsumed=headconsumd;
                return 1;
}
uint8_t dmx_demuxerTS::getInfoPSI(uint32_t *oconsumed,uint32_t *olen)
{
        *oconsumed=*olen=0;
        return 1;

}
/**
      \fn    syncH264
      \brief Search H264 startcode in the stream
*/
uint8_t         dmx_demuxerTS::syncH264( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts,uint64_t *dts)
{
uint32_t val,hnt;
retry:
         *r=0;

                val=0;
                hnt=0;

                // preload
                hnt=(read8i()<<24) + (read8i()<<16) +(read8i()<<8)+(read8i());
                if(_lastErr)
                {
                        _lastErr=0;
                        printf("\n io error , aborting sync\n");
                        return 0;
                }

                while((hnt!=1))
                {

                        hnt<<=8;
                        val=read8i();
                        hnt+=val;

                        if(_lastErr)
                        {
                             _lastErr=0;
                            printf("\n io error , aborting sync\n");
                            return 0;
                         }

                }

                *stream=read8i();
                // Case 1 : assume we are still in the same packet
                if(_pesBufferIndex>=5)
                {
                        *abs=_pesBufferStart;
                        *r=_pesBufferIndex-5;
                        *pts=_pesPTS;
                        *dts=_pesDTS;
                }
                else
                {       // pick what is needed from oldPesStart etc...
                        // since the beginning in the previous packet
                        uint32_t left=5-_pesBufferIndex;
                                 if(left>_oldPesLen)
                                 { // previous Packet which len is very shoty
                                   // Ignore
                                   _pesBufferIndex=0;
                                   printf("Ignoring too short packet");
                                   goto retry;
                                 }
                                 left=_oldPesLen-left;
#if 0
                                 printf("Next packet : %I64X Len :%"LU", using previous packet %I64X len:%u as pos=%"LU"\n",
                                 		_pesBufferStart,_pesBufferLen,_oldPesStart,_oldPesLen,_pesBufferIndex);
#endif
                                 if(left>_oldPesLen)
                                {
                                        printf("Need %"LU" bytes from previous packet, which len is %"LU"\n",left,_oldPesLen);
                                        ADM_assert(0);
                                }
                                *abs=_oldPesStart;
                                *r=left;
                                *pts=_oldPTS;
                                *dts=_oldDTS;
                }
                return 1;
}
