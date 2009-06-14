/***************************************************************************
                          GUI_xvidparam.h  -  description
                             -------------------
    begin                : Sun Nov 17 2002
    copyright            : (C) 2002 by mean
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

#ifndef XVID4ENCPARAM__
#define XVID4ENCPARAM__

typedef struct xvid4EncParam
{

  uint32_t guiLevel;			// equivalent to divx level : Fast/medium/insane

  uint32_t min_key_interval;
  uint32_t max_key_interval;
  uint32_t bframes;

  uint32_t mpegQuantizer;		// 0 h263/ 1 Mpeg quantizer
  uint32_t interlaced;
  uint32_t inter4mv;
  uint32_t trellis;
  uint32_t cartoon;
  uint32_t greyscale;

  uint32_t qpel;
  uint32_t gmc;
  uint32_t bvhq;
  uint32_t hqac;

  uint32_t   chroma_opt;
  uint32_t qmin[3];			// IPB
  uint32_t qmax[3];			// IPB

  uint32_t par_as_input;
  uint32_t par_width;
  uint32_t par_height;


  // This if for 2 pass   
  uint32_t keyframe_boost;
  uint32_t curve_compression_high;
  uint32_t curve_compression_low;
  uint32_t overflow_control_strength;
  uint32_t max_overflow_improvement;
  uint32_t max_overflow_degradation;
  uint32_t kfreduction;
  uint32_t kfthreshold;

  int container_frame_overhead;
  //
  uint32_t bquant_ratio;
  uint32_t bquant_offset;
  uint32_t vhqmode;
  uint32_t chroma_me;
  uint32_t turbo;
  uint32_t packed;
  uint32_t closed_gop;
  uint32_t bframe_threshold;
  uint32_t useCustomIntra;
  uint32_t useCustomInter;
  uint8_t  intraMatrix[64];
  uint8_t  interMatrix[64];
  char logName[200];
} xvid4EncParam;



#endif
