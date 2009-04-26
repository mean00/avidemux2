/***************************************************************************
  GPL License
  Get info from frames
  (C) Mean 2007
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_VIDEO_INFO_EXTRACTOR
#define ADM_VIDEO_INFO_EXTRACTOR

uint8_t extractMpeg4Info(uint8_t *data,uint32_t dataSize,uint32_t *w,uint32_t *h,uint32_t *time_inc);
uint8_t extractH263Info(uint8_t *data,uint32_t dataSize,uint32_t *w,uint32_t *h);
uint8_t extractVopInfo(uint8_t *data, uint32_t len,uint32_t timeincbits,uint32_t *vopType,uint32_t *modulo, uint32_t *time_inc);
uint8_t extractSPSInfo(uint8_t *data, uint32_t len,uint32_t *wwidth,uint32_t *hheight, uint32_t *fps1000, uint32_t *darNum, uint32_t *darDen);
uint8_t extractH264FrameType(uint32_t nalSize,uint8_t *buffer,uint32_t len,uint32_t *flags);
uint8_t extractH264FrameType_startCode(uint32_t nalSize,uint8_t *buffer,uint32_t len,uint32_t *flags);

#endif
//EOF
