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
#define CHECK(x) if(fourCC::check(fourcc,(uint8_t *)x)) return true
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
  return false;
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
  CHECK ("MPEG");
  CHECK ("mpg1");
  CHECK ("mpg2");
  SWAP32 (fourcc);
  if (fourcc == 0x10000002 || fourcc==0x10000001) //Mplayer fourcc
    return true;
  return false;
}
/**
    \fn isH264Compatible
*/

bool isH264Compatible (uint32_t fourcc)
{
  CHECK ("X264");
  CHECK ("x264");
  CHECK ("h264");
  CHECK ("H264");
  CHECK ("AVC1");
  CHECK ("avc1");
  return false;
}

/**
    \fn isH265Compatible
*/

bool isH265Compatible (uint32_t fourcc)
{
  CHECK ("X265");
  CHECK ("x265");
  CHECK ("h265");
  CHECK ("H265");
  CHECK ("HEVC");
  CHECK ("hevc");
  CHECK ("HVC1");
  CHECK ("hvc1");
  return false;
}

/**
    \fn isMSMpeg4Compatible
*/

bool isMSMpeg4Compatible (uint32_t fourcc)
{
  CHECK ("MP43");
  CHECK ("mp43");
  CHECK ("div3");
  CHECK ("DIV3");
  CHECK ("DIV4");
  CHECK ("div4");
  CHECK ("COL1");
  return false;
}
/**
    \fn isVC1Compatible
*/
bool isVC1Compatible    (uint32_t fourcc)
{
  CHECK ("VC1 ");
  CHECK ("WVC1");
  return false;
}
/**
    \fn isVP6Compatible
*/

bool isVP6Compatible (uint32_t fourcc)
{
  CHECK ("VP6F");
  CHECK ("VP6 ");
  CHECK ("VP61");
  CHECK ("VP62");
  return false;
}

/**
    \fn isVP9Compatible
*/

bool isVP9Compatible (uint32_t fourcc)
{
  CHECK ("VP9 ");
  CHECK ("VP90");
  return false;
}

/**
    \fn isDVCompatible
*/

bool isDVCompatible (uint32_t fourcc)
{
  CHECK ("dvsd");
  CHECK ("DVSD");
  CHECK ("dvpp");
  CHECK ("CDVC");
  CHECK ("cdvc");
  return false;
}
//EOF
