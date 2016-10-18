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
bool extraData2packets(uint8_t *extraData, int extraLen,uint8_t **packs,int *packLen)
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

    
    
    
    
}