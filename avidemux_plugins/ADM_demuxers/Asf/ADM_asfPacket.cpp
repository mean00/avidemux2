/***************************************************************************
    copyright            : (C) 2006 by mean
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
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <math.h>

#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"
#include "ADM_assert.h"

#include "fourcc.h"


#include "ADM_asfPacket.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_ASF
#include "ADM_osSupport/ADM_debug.h"

 
asfPacket::asfPacket(FILE *f,uint32_t nb,uint32_t pSize,ADM_queue *q,uint32_t startDataOffset)
 {
   _fd=f;
   pakSize=pSize;
   ADM_assert(pakSize);
   packetStart=ftello(f);;
   aprintf("Packet created at %x\n",packetStart);
   ADM_assert(_fd);
   queue=q;
   ADM_assert(q);
   currentPacket=0;
   _nbPackets=nb;
   _startDataOffset=startDataOffset;
 }
 asfPacket::~asfPacket()
 {
	 purge();
 }
 uint8_t   asfPacket::readChunkPayload(uint8_t *data, uint32_t *dataLen)
 {
   uint32_t remaining;
   *dataLen=0;
   ADM_assert(0);
   purge();
   return 1;
  
 }
 
 uint8_t asfPacket::goToPacket(uint32_t packet)
 {
   uint32_t offset=_startDataOffset+packet*pakSize;
   fseeko(_fd,offset,SEEK_SET);
   currentPacket=packet;
   return 1;
 }
 /*
      Read ASF packet & segments 
 
    Flags are bitwise OR of:
   
 0x40 Explicit packet size specified word16  0X60 Means word32
 0x20 Explicit packet size specified byte
   
 0x10 16-bit padding size specified  0x18 means word32
 0x08 8-bit padding size specified
   
 0x04 sequence coded in word16
 0x02 sequence coded in byte
 0x01 More than one segment
 
 
 Docs from http://avifile.sourceforge.net/asf-1.0.htm
  completed by mplayer code
  
 
 */
uint8_t   asfPacket::nextPacket(uint8_t streamWanted)
{
   uint32_t atime,aduration,nbSeg,segType=0x80;
   uint32_t sequenceLen,len,streamId;
   int32_t   packetLen=0;
   uint32_t  paddingLen;
   uint8_t   flags;
    
   packetStart=ftello(_fd);
#ifdef ADM_DEBUG
   uint32_t round=packetStart-_startDataOffset;
   if(round % pakSize)
   {
     printf("[ASF PACKET] we are starting a new packet at 0x%x\n",packetStart); 
     printf("[ASF PACKET]but data starts at  0x%x\n",_startDataOffset);
     printf("[ASF PACKET]and offset is not a multiple of length = %d\n",pakSize);
     ADM_assert(0);
     
   }
#endif
   _offset=0;
   if(read8()!=0x82) 
   {
     printf("[ASF PACKET]At pos %x \n",ftello(_fd));
     printf("[ASF PACKET]not a 82 packet\n");
     printf("[ASF PACKET]not a 82 packet\n");
     printf("[ASF PACKET]not a 82 packet\n");
     return 0;
   }
   
   aprintf("============== New packet ===============\n");
   read16();          // Always 0 ????
   flags=read8();
   segmentId=read8();
   packetLen=0;
   paddingLen=0;
   

   // Read packetLen
   packetLen=readVCL(flags>>5);
   // Sequence len
   sequenceLen=readVCL(flags>>1);
   // Read padding size (padding):
   paddingLen=readVCL(flags>>3);
   
   aprintf("paddingLen :         %d\n",paddingLen);
   
// Explicit (absolute) packet size	    
   if(((flags>>5)&3))
   {
     printf("## Explicit packet size %d\n",packetLen);
     if(packetLen>pakSize) printf("**************Len > packet size!! (%d /%d)\n",packetLen,pakSize);
   } 
   if(!packetLen)
   {
     // Padding (relative) size
     packetLen=pakSize-_offset;
     packetLen=packetLen-paddingLen;
   }

   
  
   atime=read32(); // Send time
   aduration=read16(); // Duration
   
   if(flags &1) // Multiseg
   {
     uint8_t r=read8();
     nbSeg=r&0x3f;
     segType=r>>6;
   }
   else
   {
     nbSeg=1; 
   }
#ifdef ASF_VERBOSE   
   printf("-----------------------\n");
   printf("Flags     :           0X%x",flags);
   
   if(flags & 0x40) printf(" Packet Len Specified  ");
   if(flags & 0x10) printf(" Padding 16bits ");
   if(flags & 0x8) printf(" Padding 8bits ");
   if(flags & 0x1) printf(" Multiseg ");
   printf("\n");
   printf("SegmentId :           %d\n",segmentId);
   printf("sequenceLen :         %d\n",sequenceLen);
   
   
   printf("packetLen :           %d\n",packetLen);
   printf("Send      :           %d\n",atime);
   printf("Duration  :           %d\n",aduration);
   printf("# of seg  :           %d %x\n",nbSeg,segType);
#endif
   // Now read Segments....
   //
   uint32_t sequence, offset,replica,r;
   int32_t remaining;
   uint32_t payloadLen;
   uint32_t keyframe;
   for(int seg=0;seg<nbSeg;seg++)
   {
     r=read8(); // Read stream Id
     if(r&0x80) keyframe=AVI_KEY_FRAME;
     else       keyframe=0;
     streamId=r&0x7f;
     aprintf(">>>>>Stream Id : %x<<<<<\n",streamId);
     if(r&0x80) 
     {
       aprintf("KeyFrame\n");
     }
     sequence=readVCL(segmentId>>4);
     offset=readVCL(segmentId>>2);
     replica=readVCL(segmentId);
     aprintf("replica                %d\n",replica);
     // Skip replica data_len
     skip(replica);
     
     payloadLen=0;
     if(flags &1)  // multi seg
     {
       payloadLen=readVCL(segType);
       if(payloadLen)
        aprintf("##len                    %d\n",payloadLen);
       
     }
     remaining=pakSize-_offset;
     remaining=remaining-paddingLen;
     aprintf("Remaining %d asked %d\n",remaining,payloadLen);
     if(remaining<=0) 
     {
       printf("** Err: No data left (%d)\n",remaining); 
     }
     if(!payloadLen)
     {
       payloadLen=remaining;
     }
     if(remaining<payloadLen)
     {
       printf("** WARNING too big %d %d\n", remaining,packetLen);
       payloadLen=remaining;
     }
#ifdef ASF_VERBOSE     
     printf("This segment %d bytes, %d /%d\n",packetLen,seg,nbSeg);
     printf("Offset                 %d\n",offset);
     printf("sequence               %d\n",sequence);
     printf("Grouping               %d\n",replica==1);
     printf("payloadLen             %d\n",payloadLen);
#endif
     // Frag
     if(replica==1) // Grouping
     {
       // Each tiny packet starts with 
       // 1 byte = packet Len
       // Data and we read them until "payloadLen" is comsumed
       while(payloadLen>0)
       {
         uint8_t l=read8();
         payloadLen--;
         if(l>payloadLen)
         {
           
           printf("oops exceeding %d/%d\n",l,payloadLen);
           if(streamId==streamWanted || streamWanted==0xff)
           {
             pushPacket(keyframe,currentPacket,offset,sequence,payloadLen,streamId);
             
           }else
           {
            skip(payloadLen);
           }
           break;
         }
         skip(l);
         payloadLen-=l;
       }
       
     }else
     { // else we read "payloadLen" bytes and put them at offset "offset"
       if(streamId==streamWanted|| streamWanted==0xff)
       {
         pushPacket(keyframe,currentPacket,offset,sequence,payloadLen,streamId);    
       }else
        skip(payloadLen);
       aprintf("Reading %d bytes\n",payloadLen);
     }
     
   }
   // Do some sanity check
   if(_offset+paddingLen!=pakSize)
   {
     printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %d %d\n",_offset,paddingLen);
   }
   currentPacket++;
   return 1;
  
 }
 /*
    Push a packet down the queue
    The packet could be a complete one or a fragement
    To know that, either look at the offset field which will be != for fragements
    Or look if the sequence number is increasing
 
 */

 uint8_t asfPacket::pushPacket(uint32_t keyframe,uint32_t packetnb,uint32_t offset,uint32_t sequence,uint32_t payloadLen,uint32_t stream)
 {
   asfBit *bit=new asfBit;
   aprintf("Pushing packet stream=%d len=%d seq=%d\n",stream,payloadLen,sequence);
   bit->sequence=sequence;
   bit->offset=offset;
   bit->len=payloadLen;
   bit->data=new uint8_t[payloadLen];
   bit->stream=stream;
   bit->packet=packetnb;
   bit->flags=keyframe;

   if(!read(bit->data,bit->len))
   {
		delete[] bit->data;
		delete bit;
		return 0; 
   }

   queue->push((void *)bit);
   return 1;
 }
 
 uint32_t asfPacket::readVCL(uint32_t bitwise)
 {
   uint32_t r;
   switch(bitwise&3)
   {
     case 3: r=read32();break;  // dword
     case 2: r=read16();break;  // word
     case 1: r=read8();break;   // byte
     default: r=0;
   }
   return r;
 }

 uint8_t   asfPacket::skipPacket(void)
 {
   uint32_t go;
   go=packetStart+ pakSize;
   aprintf("Pos %x\n",ftello(_fd));
   fseeko(_fd,go,SEEK_SET);
   aprintf("Skipping to %x\n",go);
  
   return 1; 
 }
 uint8_t   asfPacket::read(uint8_t *where, uint32_t how)
 {
 
   if(1!=fread(where,how,1,_fd))
   {
     printf("[AsfPacket] Read error\n");
     return 0; 
   }
   _offset+=how;
   ADM_assert(_offset<=pakSize);

   return 1;

  
 }
 uint8_t   asfPacket::skip( uint32_t how)
 {
   fseeko(_fd,how,SEEK_CUR);
   _offset+=how;
   ADM_assert(_offset<=pakSize);

   return 1;
 }
 //****************************
 uint8_t   asfPacket::dump(void)
 {
  
   return 1;
  
 }
 //****************************
 uint8_t asfPacket::purge(void)
 {
    // Flush queue
   while(!queue->isEmpty())
   {
     asfBit *bit;
     ADM_assert(queue->pop((void**)&bit));
     delete[] bit->data;
     delete bit;
   }
   return 1; 
 }
 //****************************
 uint8_t   asfPacket::packTo(uint8_t *buffer,uint32_t *len)
 {
   *len=0;
   while(!queue->isEmpty())
   {
     asfBit *bit;
     ADM_assert(queue->pop((void**)&bit));
     memcpy(buffer,bit->data,bit->len);
     *len+=bit->len;
     delete[] bit->data;
     delete bit;
   }
   return 1;
 }
 
 
#ifndef ASF_INLINE
#include "ADM_asfIo.h"
#endif

 //EOF
