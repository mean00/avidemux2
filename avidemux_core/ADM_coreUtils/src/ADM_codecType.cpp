/** *************************************************************************
    \file ADM_codecType
    \brief Return codec family (mpeg1/2/4/h264/DV/...) from fourcc
                      
    copyright            : (C) 2009 by mean
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_frameType.h"
#include "ADM_codecType.h"
#include "fourcc.h"

/**
    \fn isMpeg4Compatible
*/
bool  isMpeg4Compatible (uint32_t fourcc)
{
#define CHECK(x) if(fourCC::check(fourcc,(uint8_t *)x)) \
						{divx4=1; }

  uint8_t divx4 = 0;

  CHECK ("FMP4");
  CHECK ("fmp4");
  CHECK ("DIVX");
  CHECK ("divx");
  CHECK ("DX50");
  CHECK ("xvid");
  CHECK ("XVID");
  CHECK ("BLZ0");
  CHECK ("M4S2");
  CHECK ("3IV2");
  CHECK ("SEDG");

  return divx4;

#undef CHECK
}
#ifdef ADM_BIG_ENDIAN
#define SWAP32(x) x=R32(x)
#else
#define SWAP32(x) ;
#endif
/**
    \fn isMpeg12Compatible
*/

bool isMpeg12Compatible (uint32_t fourcc)
{
#define CHECK(x) if(fourCC::check(fourcc,(uint8_t *)x)) \
						{mpeg=1; }

  uint8_t mpeg = 0;
  CHECK ("MPEG");
  CHECK ("mpg1");
  CHECK ("mpg2");
  SWAP32 (fourcc);
  if (fourcc == 0x10000002 || fourcc==0x10000001) //Mplayer fourcc
    mpeg = 1;
  return mpeg;
#undef CHECK
}
/**
    \fn isH264Compatible
*/

bool isH264Compatible (uint32_t fourcc)
{
#define CHECK(x) if(fourCC::check(fourcc,(uint8_t *)x)) \
                                                {h264=1; }

  uint8_t h264 = 0;

  CHECK ("X264");
  CHECK ("x264");
  CHECK ("h264");
  CHECK ("H264");
  CHECK ("AVC1");
  CHECK ("avc1");
  return h264;

#undef CHECK
}
/**
    \fn isMSMpeg4Compatible
*/

bool isMSMpeg4Compatible (uint32_t fourcc)
{
#define CHECK(x) if(fourCC::check(fourcc,(uint8_t *)x)) \
						{divx3=1; }

  uint8_t divx3 = 0;

  CHECK ("MP43");
  CHECK ("mp43");
  CHECK ("div3");
  CHECK ("DIV3");
  CHECK ("DIV4");
  CHECK ("div4");
  CHECK ("COL1");

  return divx3;

#undef CHECK
}
/**
    \fn isVC1Compatible
*/
bool isVC1Compatible    (uint32_t fourcc)
{

#define CHECK(x) if(fourCC::check(fourcc,(uint8_t *)x)) \
						{divx3=1; }

  uint8_t divx3 = 0;

  CHECK ("VC1 ");
  CHECK ("WVC1");
  return divx3;

#undef CHECK
}
/**
    \fn isVP6Compatible
*/

bool isVP6Compatible (uint32_t fourcc)
{

#define CHECK(x) if(fourCC::check(fourcc,(uint8_t *)x)) \
						{divx3=1; }

  uint8_t divx3 = 0;

  CHECK ("VP6F");
  CHECK ("VP6 ");
  CHECK ("VP61");
  CHECK ("VP62");

  return divx3;

#undef CHECK
}
/**
    \fn isDVCompatible
*/

bool isDVCompatible (uint32_t fourcc)
{
#define CHECK(x) if(fourCC::check(fourcc,(uint8_t *)x)) \
						{dv=1; }

  uint8_t dv = 0;

  CHECK ("dvsd");
  CHECK ("DVDS");
  CHECK ("dvpp");

  return dv;

#undef CHECK
}
//EOF
