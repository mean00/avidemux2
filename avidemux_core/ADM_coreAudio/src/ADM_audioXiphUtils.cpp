/**
    \file  ADM_audioXiphUtils
    \brief Xiph lacing utilities

    \author copyright            : (C) 2016 by mean
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
#include "ADM_audiodef.h"
#include "ADM_audioCodecEnum.h"
#include "ADM_audioIdentify.h"
#include "fourcc.h"
#include "ADM_audioXiphUtils.h"

/**
 * 
 * @param hd
 * @return 
 */
static int xypheLacingRead(uint8_t **hd)
{
      int x=0; 
      uint8_t *p=*hd;
      while(*p==0xff)  
      { 
        x+=0xff; 
        p++; 
      } 
      x+=*p;
      p++;
      *hd=p;
      return x;
}




namespace ADMXiph
{
#define HEADER_LEN (4*3)
    

/**
    \fn xiphExtraData2Adm
    \brief reformat vorbis extra data to avidemux style
*/
/*
  The private data contains the first three Vorbis packet in order. The lengths of the packets precedes them. The actual layout is:
Byte 1: number of distinct packets '#p' minus one inside the CodecPrivate block. This should be '2' for current Vorbis headers.
Bytes 2..n: lengths of the first '#p' packets, coded in Xiph-style lacing. The length of the last packet is the length of the CodecPrivate block minus the lengths coded in these bytes minus one.
Bytes n+1..: The Vorbis identification header, followed by the Vorbis comment header followed by the codec setup header.
*/
    
bool xiphExtraData2Adm(uint8_t *extraData, int extraLen,uint8_t **newExtra,int *newExtraLen)    
{    
  *newExtra=NULL;
  *newExtraLen=0;
  uint8_t *oldata=extraData;
  int oldlen=extraLen;
  int len1,len2,len3;
  uint8_t *head;
      if(*oldata!=2) // 3 packets -1 = 2
      {
          ADM_warning("[MKV] weird vorbis audio, expect problems\n");
          return false;
      }
      // First packet length
      head=oldata+1;

      len1=xypheLacingRead(&head);
      len2=xypheLacingRead(&head);   
      
      int consumed=head-oldata;      
      len3=oldlen-consumed; // left in extradata
      
      if(len3<0)
      {
        ADM_warning("Error in vorbis header, len3 too small %d %d / %d\n",len1,len2,len3);
        return false;
      }
      len3-=(len1+len2);
      ADM_info("Found packets len : %d- %d- %d, total size %d\n",len1,len2,len3,oldlen);
      // Now build our own packet...
      // Allocate uint32 for alignment purpose
      
      uint32_t *buffer=new uint32_t[3+(4+len1+len2+len3)/4];        
      uint32_t nwlen=len1+len2+len3+sizeof(uint32_t)*3; // in bytes
      
      uint8_t *cp=(uint8_t *)(buffer+3); // data part
      memcpy(cp,head,len1);
      cp+=len1;head+=len1;
      
      memcpy(cp,head,len2);
      cp+=len2;head+=len2;
      
      memcpy(cp,head,len3);
      
      buffer[0]=len1;
      buffer[1]=len2;
      buffer[2]=len3;
      // Destroy old datas
      *newExtra=(uint8_t *)(buffer);
      *newExtraLen=nwlen;
  return true;
}    
    
    
/**
 *  \fn extraData2packets
 *  \brief converts adm extradata to 3 packets
 */    
bool admExtraData2packets(uint8_t *extraData, int extraLen,uint8_t **packs,int *packLen)
{

  uint32_t *ptr=(uint32_t *)extraData;
  int sum=0;
  for(int i=0;i<3;i++)
  {
      packLen[i]=ptr[i];
      sum+=ptr[i];
  }
  if((sum+HEADER_LEN)!=extraLen)
  {
      ADM_warning("Incorrect xiph extra data (%d vs %d)\n",sum+HEADER_LEN,extraLen);
      return false;
  }
  extraData+=HEADER_LEN;
  
  
  packs[0]=extraData;
  packs[1]=extraData+packLen[0];
  packs[2]=extraData+packLen[0]+packLen[1];
  return true;
}

    
/**
 * \fn admExtraData2xiph
 * \brief convert adm extradata to xiph extradata
 * @param l
 * @param src
 * @param dst
 * @return 
 */
int admExtraData2xiph(int l, uint8_t *src, uint8_t *dstOrg)
{
    int outLen=1;
    int length[3];
    uint8_t *dst=dstOrg;
    ADM_info("insize=%d\n",l);
    *dst++=0x2;
    for(int i=0;i<3;i++)
    {
        length[i]=(src[3]<<24)+(src[2]<<16)+(src[1]<<8)+src[0];
        src+=4;
        //printf("Packet %d size %d\n",i,length[i]);
        // encode length
        if(i!=2)
        {
            int encode=length[i];
            while(encode>=255) 
            {
                *dst++=0xff;
                encode-=0xff;
            }
            *dst++=encode;
        }
    }
    // now copy blocks
    for(int i=0;i<3;i++)
    {
        int block=length[i];
        memcpy(dst,src,block);
        src+=block;
        dst+=block;
    }
    int outSize= (int)(dst-dstOrg);
    ADM_info("OutSize=%d\n",outSize);
    return outSize;
}

    
    
    
}