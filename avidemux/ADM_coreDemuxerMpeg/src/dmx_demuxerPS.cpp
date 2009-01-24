/***************************************************************************
                          PS demuxer
                             -------------------

    copyright            : (C) 2005 by mean
    email                : fixounet@free.fr
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

#include "dmx_demuxerPS.h"
uint8_t         dmx_demuxerPS::changePid(uint32_t newpid,uint32_t newpes)
{
          myPid=newpes & 0xff;
          if(myPid<9 || (myPid>=0xA0&&myPid<=0xA9) || (myPid>=0x20 && myPid<0x27) ||(myPid>=0x40 && myPid<=0x49) ) myPid|=0xff00;
        _pesBufferStart=0;  // Big value so that we read
        _pesBufferLen=0;
        _pesBufferIndex=0;
}
dmx_demuxerPS::dmx_demuxerPS(uint32_t nb,MPEG_TRACK *tracks,uint32_t multi)
{
        consumed=0;
        parser=new fileParser();
        stampAbs=0;
        _pesBuffer=new uint8_t [MAX_PES_BUFFER];

        memset(seen,0,sizeof(seen));

        _pesBufferStart=0;  // Big value so that we read
        _pesBufferLen=0;
        _pesBufferIndex=0;
        ADM_assert(nb>0);
        tracked=NULL;

        nbTracked=nb;
        for(int i=0;i<256;i++) trackPTS[i]=ADM_NO_PTS;
        myPid=tracks[0].pes;

        if(nb!=256)     // Only pick one track as main, and a few as informative
        {

                memset(mask,0,256);
                tracked=new uint8_t[nbTracked];
                for(int i=1;i<nb;i++)
                {
                        mask[tracks[i].pes&0xff]=1;
                        tracked[i]=tracks[i].pes&0xff;
                }

        }else
        {
                memset(mask,1,256); // take all tracks
        }

        if(myPid<9 || (myPid>0xA0&&myPid<0xA9) || (myPid>=0x20 && myPid<0x27)) myPid|=0xff00;



       	_probeSize=0;
       	memset(seen,0,255*sizeof(uint64_t));
        printf("Creating mpeg PS demuxer  main Pid: %X \n",myPid);
        _multi=multi;
}
dmx_demuxerPS::~dmx_demuxerPS()
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
uint8_t       dmx_demuxerPS::getStats(uint64_t *oseen)
{
        if(nbTracked!=256)
        {
                oseen[0]=0;
                for(int i=1;i<nbTracked;i++)
                {
                        oseen[i]=seen[tracked[i]];
                }
        }
        else
        {
                 for(int i=0;i<nbTracked;i++)
                {
                        oseen[i]=seen[i];
                }
        }
        return 1;
}
uint8_t         dmx_demuxerPS::getAllPTS(uint64_t *stat)
{
        if(nbTracked!=256)
        {
                stat[0]=0;
                for(int i=1;i<nbTracked;i++)
                {
                        stat[i]=trackPTS[tracked[i]];
                }
        }
        else
        {
                 for(int i=0;i<nbTracked;i++)
                {
                        stat[i]=trackPTS[i];
                }
        }
        return 1;
}

uint8_t dmx_demuxerPS::setProbeSize(uint32_t sz)
{
		_probeSize=sz;
		return 1;
}
uint8_t dmx_demuxerPS::open(const char *name)
{
FP_TYPE fp=FP_DONT_APPEND;
        if(_multi) fp=FP_APPEND;
        if(! parser->open(name,&fp)) return 0;
        _size=parser->getSize();

        return 1;
}
uint8_t dmx_demuxerPS::forward(uint32_t f)
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
uint8_t  dmx_demuxerPS::stamp(void)
{
        consumed=0;
}
uint64_t dmx_demuxerPS::elapsed(void)
{
        return consumed;
}
uint8_t  dmx_demuxerPS::getPos( uint64_t *abs,uint64_t *rel)
{
        *rel=_pesBufferIndex;
        *abs=_pesBufferStart;
        return 1;
}
uint8_t dmx_demuxerPS::setPos( uint64_t abs,uint64_t  rel)
{
				// Need to move ?
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
               if(!parser->setpos(abs))
                {
                        printf("DMX_PS: setPos failed\n");
                         return 0;
                }
                _pesBufferStart=abs;
                if(!refill())
                {
                        printf("DMX_PS: refill failed\n");
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



uint32_t         dmx_demuxerPS::read(uint8_t *w,uint32_t len)
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
                        printf("Refill failed at %"LLD"  \n",_pesBufferStart);
                        _lastErr=1;
                         return 0;
                }
                return mx+read(w,len);
}
uint8_t         dmx_demuxerPS::sync( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts,uint64_t *dts)
{
uint32_t val,hnt;
         *r=0;

                val=0;
                hnt=0;

                // preload
                hnt=read32i();
                if(_lastErr)
                {
                        _lastErr=0;
                        printf("\n io error , aborting sync 1\n");
                        return 0;
                }
                val=hnt&0xff;
                while(((hnt&0xffffff00)!=0x100))
                {

                        hnt<<=8;
                        val=read8i();
                        hnt+=val;
                        if(_lastErr)
                        {
                             _lastErr=0;
                            printf("\n io error , aborting sync 2\n");
                            return 0;
                         }

                }

                *stream=val;
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
                                 left=_oldPesLen-left;
#if 0
                                 printf("Next packet : %I64X Len :%lu, using previous packet %I64X len:%u as pos=%lu\n",
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

//
//      Refill the pesBuffer
//              Only with the pid that is of interest for us
//              Update PTS/DTS
//              Keep track of other pes len
//
 uint8_t         dmx_demuxerPS::getPacketInfo(uint8_t **data, uint32_t *len, uint32_t *usableLen,uint64_t *pts)
 {
            *data=_pesBuffer;
            *len=_pesBufferLen;
            *usableLen=_pesBufferLen-_pesBufferIndex;
            *pts=_pesPTS;
            return 1;
 }
uint8_t dmx_demuxerPS::refill(void)
{
uint32_t globstream,len;
uint8_t  stream,substream;
uint64_t abs,pts,dts;
        // Resync on our stream
_again:
        if(!parser->sync(&stream))
        {
                uint64_t pos;
                parser->getpos(&pos);
                printf("DmxPS: cannot sync  at %"LLU"/%"LLU"\n",pos,_size);
                _lastErr=1;
                return 0;
        }
        parser->getpos(&abs);
        if(_probeSize)
        {
        	uint64_t pos;
                parser->getpos(&pos);
                if(pos>_probeSize)
                {
                		printf("Probe exceeded\n");
                		return 0;
                }
        }
// Handle out of band stuff
        if(stream==PACK_START_CODE)
        {
        		parser->forward(8);
        		goto _again;
        }
        if(stream==PADDING_CODE || stream==SYSTEM_START_CODE)
        {
                        len=parser->read16i();
                        //printf("\tForwarding %lu bytes\n",len);
        		parser->forward(len);
        		goto _again;
        }
        // Only keep relevant parts
        // i.e. a/v : C0 C9 E0 E9
        // subs 20-29
        // private data 1/2
#define INSIDE(min,max) (stream>=min && stream<max)
        if(!(  INSIDE(0xC0,0xC9) || INSIDE(0xE0,0xE9) || INSIDE(0x20,0x29) || stream==PRIVATE_STREAM_1 || stream==PRIVATE_STREAM_2
        			)) goto _again;
        // Ok we got a candidate
        parser->getpos(&abs);
        abs-=4;
        if(!getPacketInfo(stream,&substream,&len,&pts,&dts))
        {
                goto _again;
        }
        if(!len) goto _again;
        if(len>MAX_PES_BUFFER) goto _again;

        if(stream==PRIVATE_STREAM_1) globstream=0xFF00+substream;
                else                 globstream=stream;
      //  seen[globstream & 0xFF]+=len;
        if(myPid==globstream)
        {
                _oldPesStart=_pesBufferStart;
                _oldPesLen=_pesBufferLen;
                _oldPTS=_pesPTS;
                _oldDTS=_pesDTS;

                _pesDTS=dts;
                _pesPTS=pts;
                _pesBufferStart=abs;
                _pesBufferLen=len;
                _pesBufferIndex=0;

                if(!parser->read32(len,_pesBuffer))
                {
                        printf("Read failed dmx_demuxerPS::refill %d\n",len);
                        return 0;
                }
                return 1;
        }
        if(mask[globstream &0xff])
        {
                seen[globstream& 0xff]+=len;
                if(trackPTS[globstream&0xff]==ADM_NO_PTS && pts!=ADM_NO_PTS)
                {
                        trackPTS[globstream & 0xff]=pts;
                }

        }
        // Here keep track of other tracks
         parser->forward(len);
        goto _again;
        return 0;
}
/***************************************************
    Alternate refill, we read the whole packet
****************************************************/
uint8_t dmx_demuxerPS::refillFull(uint8_t *outstream)
{
uint32_t globstream,len;
uint8_t  stream,substream;
uint64_t abs,pts,dts;
        // Resync on our stream
_again2:
        *outstream=0;
        if(!parser->sync(&stream))
        {
                uint64_t pos;
                parser->getpos(&pos);
                printf("DmxPS: cannot sync  at %"LLU"/%"LLU"\n",pos,_size);
                _lastErr=1;
                return 0;
        }
        parser->getpos(&abs);
        if(_probeSize)
        {
        	uint64_t pos;
                parser->getpos(&pos);
                if(pos>_probeSize)
                {
                		printf("Probe exceeded\n");
                		return 0;
                }
        }
// Handle out of band stuff
        if(stream==PACK_START_CODE)
        {
        		parser->forward(8);
        		goto _again2;
        }
        if( stream==PADDING_CODE ||stream==SYSTEM_START_CODE)
        {
                        len=parser->read16i();
                        //printf("\tForwarding %lu bytes\n",len);
        		parser->forward(len);
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
        parser->getpos(&abs);
        abs-=4;
        if(!getPacketInfo(stream,&substream,&len,&pts,&dts))
        {
                goto _again2;
        }
        if(len>MAX_PES_BUFFER) goto _again2;

        if(stream==PRIVATE_STREAM_1) globstream=0xFF00+substream;
                else                 globstream=stream;
      //  seen[globstream & 0xFF]+=len;

            uint32_t headerLen;
            uint64_t curPos,totalLen;

                 parser->getpos(&curPos);
                 headerLen=curPos-abs;
                _oldPesStart=_pesBufferStart;
                _oldPesLen=_pesBufferLen;
                _oldPTS=_pesPTS;
                _oldDTS=_pesDTS;

                totalLen=headerLen+len;

                _pesDTS=dts;
                _pesPTS=pts;
                _pesBufferStart=abs;
                _pesBufferLen=totalLen;
                _pesBufferIndex=headerLen;
                parser->setpos(abs);
                *outstream=globstream;
                if(!parser->read32(totalLen,_pesBuffer)) return 0;
                return 1;

}
/*
        Retrieve info about the packet we just met
        It is assumed that parser is just after the packet startcode

*/

uint8_t dmx_demuxerPS::getPacketInfo(uint8_t stream,uint8_t *substream,uint32_t *olen,uint64_t *opts,uint64_t *odts)
{

//uint32_t un ,deux;
uint64_t size=0;
uint8_t c,d;
uint8_t align=0;

                *substream=0xff;
                *opts=ADM_NO_PTS;
                *odts=ADM_NO_PTS;


                size=parser->read16i();
                if((stream==PADDING_CODE) ||
                	 (stream==PRIVATE_STREAM_2)
                        ||(stream==SYSTEM_START_CODE) //?
                        ) // special case, no header
                        {
                                *olen=size;
                                return 1;
                        }

                        //      remove padding if any

                while((c=parser->read8i()) == 0xff)
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
                        c=parser->read8i();     // PTS/DTS
                        //printf("%x ptsdts\n",c
                        ptsdts=c>>6;
                        // header len
                        len=parser->read8i();
                        size-=3;

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
                        }

                if(stream==PRIVATE_STREAM_1)
                {
                        if(size>5)
                        {
                        // read sub id
                               *substream=parser->read8i();
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
                                                parser->forward(3);
                                                size-=3;
                                                break;
                                // Subs
                                case 0x20:case 0x21:case 0x22:case 0x23:
                                case 0x24:case 0x25:case 0x26:case 0x27:
                                                break;

                                default:
                                                printf("Unkown substream %x\n",*substream);
                                                *substream=0xff;
                                }
                                // skip audio header (if not sub)
                                if(*substream>0x26 || *substream<0x20)
                                {
                                        parser->forward(3);
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
                        printf("*** packet type 1 inside type 2 ?????*****\n");
                        return 0; // mmmm
                }
          // now look at  STD buffer size if present
          // 01xxxxxxxxx
          if ((c>>6) == 1)
          {       // 01
                        size-=2;
                        parser->read8i();                       // skip one byte
                        c=parser->read8i();   // then another
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
                                        pts1=parser->read16i()>>1;
                                        pts2=parser->read16i()>>1;
                                        *opts=pts2+(pts1<<15)+(pts0<<30);
                                        break;
                  }
                  case 3:
                  {               // 0011 xxxx
                        uint64_t pts1,pts2,pts0;
                                        size -= 9;

                                        pts0=(c>>1) &7;
                                        pts1=parser->read16i()>>1;
                                        pts2=parser->read16i()>>1;
                                        *opts=pts2+(pts1<<15)+(pts0<<30);
                                        parser->forward(5);
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
