/*
 *  mux_out.h
 *
 *  Copyright (C) Gerhard Monzel - October 2003
 *
 *  This file is part of lve, a linux mpeg video editor
 *      
 *  lve tools are free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  lve is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */
#ifndef _MUX_OUT_H
#define _MUX_OUT_H 1
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define VOB_PACK_LEN   2048

#define MP2_FRAME_SIZE 1152
#define AC3_FRAME_SIZE 1536

#define AUDIO_ID_MP2   0xC0
#define AUDIO_ID_AC3   0x80
#define VIDEO_ID       0xE0

#define MX_PCK_CODE       0xBA
#define MX_SYS_CODE       0xBB
#define MX_SEQ_CODE       0xB3
#define MX_GOP_CODE       0xB8
#define MX_PIC_CODE       0x00
#define MX_EXT_CODE       0xB5
#define MX_SEQ_END        0xB7

#define MX_START_CODE     0x00000100

#define MX_PCK_START_CODE (MX_START_CODE + MX_PCK_CODE)
#define MX_SYS_START_CODE (MX_START_CODE + MX_SYS_CODE)
#define MX_SEQ_START_CODE (MX_START_CODE + MX_SEQ_CODE)
#define MX_GOP_START_CODE (MX_START_CODE + MX_GOP_CODE)
#define MX_PIC_START_CODE (MX_START_CODE + MX_PIC_CODE)
#define MX_EXT_START_CODE (MX_START_CODE + MX_EXT_CODE)
#define MX_SEQ_END_CODE   (MX_START_CODE + MX_SEQ_END)

#define MX_PRIVATE_STREAM_1   0x1BD
#define MX_PADDING_STREAM     0x1BE
#define MX_PRIVATE_STREAM_2   0x1BF

#define V_PTS_MIN  120
#define A_PTS_MIN   80
#define V_PTS_DFLT 120
#define A_PTS_DFLT 120

#undef __EXTERN
#ifndef _MUX_OUT
#define __EXTERN extern 
#else
#define __EXTERN
#endif
#define MAX_PACK 100
typedef struct
{
  uint8_t *buf;
  int     byte_ofs;
  int     bits_left;
} BitPack;

typedef struct
{
  //-- input values: --
  //--   must be set before enter mux_open --
  int       pack_size;
  double    pict_duration;
  int       v_pts_ofs;
  int       a_pts_ofs;
  FILE      *fp;
  
  //-- output values: --
  //--   filled after mux_open / mux_write_packet --

  //-- diverse --
  int       is_stdout; 
  int	    framecount; //MEANX : need to get correct gop
  uint8_t   forceRestamp; // Meanx : If set force gop timestamp recomputation
  //--   pack'ed output stream buffer --
  uint8_t   stream_buf[256*1024];
  int	    pack_psize[MAX_PACK];
  int	    pack_level;
  int       buf_level;
  uint8_t   *pack_start;
  BitPack   bit_pack;
  int       pack_cnt;

  //--   A/V packet data --
  uint8_t   pkt_id;
  uint8_t   *pkt_buf;
  int       pkt_len;
   
  //--   A/V parameters --   
  int       frame_no;
  int       gop_cnt;
  double    pts;
  double    scr;  
  int       mux_rate;
  
  //--   video parameters --
  int       video_id;
  int       video_bound;
  int       video_max_buf_size;
  double    video_pts;
  
  //--   audio parameters --
  int       audio_id;
  int       audio_bound;
  int       audio_max_buf_size;
  int       audio_delay;
  int       audio_encoded_fs;
  double    audio_pts;
  double    audio_blocks;
  
} PackStream;

__EXTERN PackStream *mux_open(char *fn, 
                              int v_bit_rate, double frame_rate, 
                              int a_bit_rate, int sample_rate, uint8_t audio_id);

__EXTERN void mux_close(PackStream *ps);

__EXTERN int mux_write_packet(PackStream *ps, 
                              uint8_t pkt_id, uint8_t *pkt_buf, int pkt_len); 

#endif // _MUX_OUT_H
//eof
