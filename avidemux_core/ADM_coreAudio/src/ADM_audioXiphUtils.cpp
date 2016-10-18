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
namespace ADMXiph
{
#define HEADER_LEN (4*3)
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