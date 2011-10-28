/** *************************************************************************
    \file ADM_asfPacket.cpp
    copyright            : (C) 2006/2009 by mean
    email                : fixounet@free.fr

see http://avifile.sourceforge.net/asf-1.0.htm

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
#include <math.h>
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_vidMisc.h"

#include "ADM_asfPacket.h"

#if 0
#define aprintf printf
#define ASF_VERBOSE
#else
#define aprintf(...) {}
#endif

/**
    \fn asfPacket ctor
*/ 
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
/**
    \fn asfPacket dtor
*/
 asfPacket::~asfPacket()
 {
	 purge();
 }
 /**
    \fn goToPacket
*/
 uint8_t asfPacket::goToPacket(uint32_t packet)
 {
   uint32_t offset=_startDataOffset+packet*pakSize;
   fseeko(_fd,offset,SEEK_SET);
   currentPacket=packet;
   purge();
   return 1;
 }
 /**
    \fn     nextPacket
    \brief  Read ASF packet & segments 
 
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
   uint64_t sendTime;
   uint32_t aduration,nbSeg,payloadLengthType=0x80;
   uint32_t sequenceLen,len,streamId;
   int32_t   packetLen=0;
   uint32_t  paddingLen;
   int   lengthTypeFlags,propertyFlags,multiplePayloadPresent;
   int sequenceType,sequence,offsetLenType,replicaLenType,streamNumberLenType,mediaObjectNumberLenType;
   

    
   packetStart=ftello(_fd);
   _offset=0;
   int r82=read8();
   if(r82!=0x82) 
   {
     printf("[ASF PACKET]At pos 0x%"LLX" \n",(uint64_t)ftello(_fd));
     printf("[ASF PACKET]not a 82 packet but 0x%x\n",r82);
     return 0;
   }
   
   aprintf("============== New packet ===============\n");
   read16();          // Always 0 ????

    // / end of error correction
    // Payload parsing information
   packetLen=0;
   paddingLen=0;
   sequenceType=0;
   sequenceLen=0;
   sequence=0;
   offsetLenType=0;
   replicaLenType=0;
   streamNumberLenType=0;
   mediaObjectNumberLenType=0;
   lengthTypeFlags=read8();
   propertyFlags=read8();
   multiplePayloadPresent=lengthTypeFlags&1;
   // Read packetLen
   packetLen=readVCL(lengthTypeFlags>>5,pakSize);
   // Sequence len
   sequenceLen=readVCL(lengthTypeFlags>>1,0);
   // Read padding size (padding):
   paddingLen=readVCL(lengthTypeFlags>>3,0);
   //
   replicaLenType=(propertyFlags>>0)&3;
   offsetLenType=(propertyFlags>>2)&3;
   mediaObjectNumberLenType=(propertyFlags>>4)&3;
   streamNumberLenType=(propertyFlags>>6)&3;
   
   // Send time
   sendTime=1000*read32(); // Send time (ms)
   aduration=read16(); // Duration (ms)
   printf(":: Time 1 %s\n",ADM_us2plain(sendTime));
   if(!packetLen)
   {
     // Padding (relative) size
     packetLen=pakSize-_offset;
     packetLen=packetLen-paddingLen;
   }
   int mediaObjectNumber, offset,replica,r;
   int32_t remaining;
   uint32_t payloadLen;
   uint32_t keyframe;
   // Multi payload
   if(multiplePayloadPresent)
    {
        uint8_t r=read8();
        nbSeg=r&0x3f;
        payloadLengthType=r>>6;
        printf("Multiple Payload :%d\n",(int)nbSeg);
       // Now read Segments....
       //
       for(int seg=0;seg<nbSeg;seg++)
       {
         r=read8(); // Read stream Id
         if(r&0x80) 
         {
            keyframe=AVI_KEY_FRAME;
            aprintf("KeyFrame\n");
         }
         else       keyframe=0;
         streamId=r&0x7f;
         //printf(">>>>>Stream Id : %x, duration %d ms, send time:%d ms <<<<<\n",streamId,aduration,sendTime);
         mediaObjectNumber=readVCL(mediaObjectNumberLenType,0); // Media object number
         offset=readVCL(offsetLenType,0);
         replica=readVCL(replicaLenType,0);
         if(replica==1) ADM_error("Replica==1 : Compressed data!\n");
         printf("replica                %d\n",replica);
         skip(replica);
         payloadLen=readVCL(payloadLengthType,0);
         remaining=pakSize-_offset;
         remaining=remaining-paddingLen;
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
           // else we read "payloadLen" bytes and put them at offset "offset"
           if(streamId==streamWanted|| streamWanted==0xff)
           {
             pushPacket(keyframe,currentPacket,offset,mediaObjectNumber,payloadLen,streamId,sendTime);   
             sendTime=ADM_NO_PTS;
           }else
            skip(payloadLen);
        }
    }
   else
    {  // single payload
         int r;
         r=read8(); // Read stream Id
         if(r&0x80) 
         {
            keyframe=AVI_KEY_FRAME;
            aprintf("KeyFrame\n");
         }
         else       
            keyframe=0;
         streamId=r&0x7f;
         //printf(">>>>>Stream Id : %x, duration %d ms, send time:%d ms <<<<<\n",streamId,aduration,sendTime);
         mediaObjectNumber=readVCL(mediaObjectNumberLenType,0); // Media object number
         offset=readVCL(offsetLenType,0);
         replica=readVCL(replicaLenType,0);
         printf("replica                %d\n",replica);
         skip(replica);
         remaining=pakSize-_offset;
         remaining=remaining-paddingLen;
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
           // else we read "payloadLen" bytes and put them at offset "offset"
           if(streamId==streamWanted|| streamWanted==0xff)
           {
             pushPacket(keyframe,currentPacket,offset,mediaObjectNumber,payloadLen,streamId,sendTime);   
             sendTime=ADM_NO_PTS;
           }else
            skip(payloadLen);
    }
   // Do some sanity check
   if(_offset+paddingLen!=pakSize)
   {
     ADM_warning("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %d+%d!=%d\n",_offset,paddingLen,pakSize);
     if(pakSize>(_offset+paddingLen))
     {
        skip(pakSize-_offset-paddingLen);
      }
   }
   currentPacket++;
   return 1;
  
 }
 /**
    \fn pushPacket
    \brief     Push a packet down the queue
    The packet could be a complete one or a fragement
    To know that, either look at the offset field which will be != for fragements
    Or look if the sequence number is increasing
 
 */

 uint8_t asfPacket::pushPacket(uint32_t keyframe,uint32_t packetnb,uint32_t offset,uint32_t sequence,uint32_t payloadLen,uint32_t stream,uint64_t dts)
 {
   asfBit *bit=new asfBit;
   printf("Pushing packet stream=%d len=%d offset=%d seq=%d packet=%d dts=%s \n",
                stream,payloadLen,offset,sequence,packetnb,ADM_us2plain(dts));
   bit->sequence=sequence;
   bit->offset=offset;
   bit->len=payloadLen;
   bit->data=new uint8_t[payloadLen];
   bit->stream=stream;
   bit->packet=packetnb;
   bit->flags=keyframe;
   bit->dts=dts;

   if(!read(bit->data,bit->len))
   {
		delete[] bit->data;
		delete bit;
		return 0; 
   }

   queue->push((void *)bit);
   return 1;
 }
 
 uint32_t asfPacket::readVCL(uint32_t bitwise,uint32_t def)
 {
   uint32_t r;
   switch(bitwise&3)
   {
     case 3: r=read32();break;  // dword
     case 2: r=read16();break;  // word
     case 1: r=read8();break;   // byte
     default: r=def;
   }
   return r;
 }
/**
   \fn skipPacket
*/
 uint8_t   asfPacket::skipPacket(void)
 {
   uint32_t go;
   go=packetStart+ pakSize;
   aprintf("Pos 0x%"LLX"\n",(uint64_t)ftello(_fd));
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
