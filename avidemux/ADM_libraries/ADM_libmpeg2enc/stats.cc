/* stats.c, coding statistics                                               */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */
/* Modifications and enhancements (C) 2000/2001 Andrew Stevens */

/* These modifications are free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */


#include <config.h>
#include <stdio.h>
#include <math.h>
#include "global.h"

#ifdef OUTPUT_STAT
static void calcSNR1(org,rec,lx,w,h,pv,pe)
unsigned char *org;
unsigned char *rec;
int lx,w,h;
double *pv,*pe;
{
  int i, j;
  double v1, s1, s2, e2;

  s1 = s2 = e2 = 0.0;

  for (j=0; j<h; j++)
  {
    for (i=0; i<w; i++)
    {
      v1 = org[i];
      s1+= v1;
      s2+= v1*v1;
      v1-= rec[i];
      e2+= v1*v1;
    }
    org += lx;
    rec += lx;
  }

  s1 /= w*h;
  s2 /= w*h;
  e2 /= w*h;

  /* prevent division by zero in calcSNR() */
  if(e2==0.0)
    e2 = 0.00001;

  *pv = s2 - s1*s1; /* variance */
  *pe = e2;         /* MSE */
}

#endif

void calcSNR(Picture *picture)
{

#ifdef OUTPUT_STAT
	uint8_t **org = picture->curorg;
	uint8_t **rec = picture->curref;
	int w,h,offs;
	double v,e;


  w = horizontal_size;
  h = (cur_picture.pict_struct==FRAME_PICTURE) ? vertical_size : (vertical_size>>1);
  offs = (cur_picture.pict_struct==BOTTOM_FIELD) ? width : 0;

  calcSNR1(org[0]+offs,rec[0]+offs,width2,w,h,&v,&e);
  fprintf(statfile,"Y: variance=%4.4g, MSE=%3.3g (%3.3g dB), SNR=%3.3g dB\n",
    v, e, 10.0*log10(255.0*255.0/e), 10.0*log10(v/e));
    
  if (chroma_format!=CHROMA444)
  {
    w >>= 1;
    offs >>= 1;
  }

  if (chroma_format==CHROMA420)
    h >>= 1;

  calcSNR1(org[1]+offs,rec[1]+offs,chrom_width2,w,h,&v,&e);
  fprintf(statfile,"U: variance=%4.4g, MSE=%3.3g (%3.3g dB), SNR=%3.3g dB\n",
    v, e, 10.0*log10(255.0*255.0/e), 10.0*log10(v/e));

  calcSNR1(org[2]+offs,rec[2]+offs,chrom_width2,w,h,&v,&e);
  fprintf(statfile,"V: variance=%4.4g, MSE=%3.3g (%3.3g dB), SNR=%3.3g dB\n",
    v, e, 10.0*log10(255.0*255.0/e), 10.0*log10(v/e));
    
  
#endif
}

void stats(void)
{
#ifdef OUTPUT_STAT
  int i, j, k,  mb_type;
  int n_skipped, n_intra, n_ncoded, n_blocks, n_interp, n_forward, n_backward;
  struct mbinfo *mbi;


  /* Needs adjusting to reflect its move... */
#ifdef NOT_DONE_YET
		if (cur_picture.pict_type!=I_TYPE)
		{
			fprintf(statfile," forward search window: %d...%d / %d...%d\n",
					-sxf,sxf,-syf,syf);
			fprintf(statfile," forward vector range: %d...%d.5 / %d...%d.5\n",
					-(4<<cur_picture.forw_hor_f_code),(4<<cur_picture.forw_hor_f_code)-1,
					-(4<<cur_picture.forw_vert_f_code),(4<<cur_picture.forw_vert_f_code)-1);
		}

		if (cur_picture.pict_type==B_TYPE)
		{
			fprintf(statfile," backward search window: %d...%d / %d...%d\n",
					-sxb,sxb,-syb,syb);
			fprintf(statfile," backward vector range: %d...%d.5 / %d...%d.5\n",
					-(4<<cur_picture.back_hor_f_code),(4<<cur_picture.back_hor_f_code)-1,
					-(4<<cur_picture.back_vert_f_code),(4<<cur_picture.back_vert_f_code)-1);
		}
#endif

  n_skipped=n_intra=n_ncoded=n_blocks=n_interp=n_forward=n_backward=0;

  for (k=0; k<mb_per_pict; k++)
  {
    mbi = cur_picture.mbinfo+k;
    if (mbi->skipped)
      n_skipped++;
    else if (mbi->mb_type & MB_INTRA)
      n_intra++;
    else if (!(mbi->mb_type & MB_PATTERN))
      n_ncoded++;

    for (i=0; i<block_count; i++)
      if (mbi->cbp & (1<<i))
        n_blocks++;

    if (mbi->mb_type & MB_FORWARD)
    {
      if (mbi->mb_type & MB_BACKWARD)
        n_interp++;
      else
        n_forward++;
    }
    else if (mbi->mb_type & MB_BACKWARD)
      n_backward++;
  }

  fprintf(statfile,"\npicture statistics:\n");
  fprintf(statfile," # of intra coded macroblocks:  %4d (%.1f%%)\n",
    n_intra,100.0*(double)n_intra/mb_per_pict);
  fprintf(statfile," # of coded blocks:             %4d (%.1f%%)\n",
    n_blocks,100.0*(double)n_blocks/(block_count*mb_per_pict));
  fprintf(statfile," # of not coded macroblocks:    %4d (%.1f%%)\n",
    n_ncoded,100.0*(double)n_ncoded/mb_per_pict);
  fprintf(statfile," # of skipped macroblocks:      %4d (%.1f%%)\n",
    n_skipped,100.0*(double)n_skipped/mb_per_pict);
  fprintf(statfile," # of forw. pred. macroblocks:  %4d (%.1f%%)\n",
    n_forward,100.0*(double)n_forward/mb_per_pict);
  fprintf(statfile," # of backw. pred. macroblocks: %4d (%.1f%%)\n",
    n_backward,100.0*(double)n_backward/mb_per_pict);
  fprintf(statfile," # of interpolated macroblocks: %4d (%.1f%%)\n",
    n_interp,100.0*(double)n_interp/mb_per_pict);

  fprintf(statfile,"\nmacroblock_type map:\n");

  k = 0;

  for (j=0; j<mb_height2; j++)
  {
    for (i=0; i<mb_width; i++)
    {
      mbi = cur_picture.mbinfo + k;
      mb_type = mbi->mb_type;
      if (mbi->skipped)
        putc('S',statfile);
      else if (mb_type & MB_INTRA)
        putc('I',statfile);
      else switch (mb_type & (MB_FORWARD|MB_BACKWARD))
      {
      case MB_FORWARD:
        putc(mbi->motion_type==MC_FIELD ? 'f' :
             mbi->motion_type==MC_DMV   ? 'p' :
                                          'F',statfile); break;
      case MB_BACKWARD:
        putc(mbi->motion_type==MC_FIELD ? 'b' :
                                          'B',statfile); break;
      case MB_FORWARD|MB_BACKWARD:
        putc(mbi->motion_type==MC_FIELD ? 'd' :
                                          'D',statfile); break;
      default:
        putc('0',statfile); break;
      }

      if (mb_type & MB_QUANT)
        putc('Q',statfile);
      else if (mb_type & (MB_PATTERN|MB_INTRA))
        putc(' ',statfile);
      else
        putc('N',statfile);

      putc(' ',statfile);

      k++;
    }
    putc('\n',statfile);
  }

  fprintf(statfile,"\nmquant map:\n");

  k=0;
  for (j=0; j<mb_height2; j++)
  {
    for (i=0; i<mb_width; i++)
    {
      if (i==0 || cur_picture.mbinfo[k].mquant!=cur_picture.mbinfo[k-1].mquant)
        fprintf(statfile,"%3d",cur_picture.mbinfo[k].mquant);
      else
        fprintf(statfile,"   ");

      k++;
    }
    putc('\n',statfile);
  }

#if 0

  fprintf(statfile,"\ncbp map:\n");

  k=0;
  for (j=0; j<mb_height2; j++)
  {
    for (i=0; i<mb_width; i++)
    {
      fprintf(statfile,"%02x ",mbinfo[k].cbp);

      k++;
    }
    putc('\n',statfile);
  }

  if (pict_struct==FRAME_PICTURE && !frame_pred_dct)
  {
    fprintf(statfile,"\ndct_type map:\n");

    k=0;
    for (j=0; j<mb_height2; j++)
    {
      for (i=0; i<mb_width; i++)
      {
        if (mbinfo[k].mb_type & (MB_PATTERN|MB_INTRA))
          fprintf(statfile,"%d  ",mbinfo[k].dct_type);
        else
          fprintf(statfile,"   ");
  
        k++;
      }
      putc('\n',statfile);
    }
  }
#endif

  if (cur_picture.pict_type!=I_TYPE )
  {
    fprintf(statfile,"\nforward motion vectors (first vector, horizontal):\n");

    k=0;
    for (j=0; j<mb_height2; j++)
    {
      for (i=0; i<mb_width; i++)
      {
        if (cur_picture.mbinfo[k].mb_type & MB_FORWARD)
          fprintf(statfile,"%4d",cur_picture.mbinfo[k].MV[0][0][0]);
        else
          fprintf(statfile,"   .");
  
        k++;
      }
      putc('\n',statfile);
    }

    fprintf(statfile,"\nforward motion vectors (first vector, vertical):\n");

    k=0;
    for (j=0; j<mb_height2; j++)
    {
      for (i=0; i<mb_width; i++)
      {
        if (cur_picture.mbinfo[k].mb_type & MB_FORWARD)
          fprintf(statfile,"%4d",cur_picture.mbinfo[k].MV[0][0][1]);
        else
          fprintf(statfile,"   .");
  
        k++;
      }
      putc('\n',statfile);
    }

    fprintf(statfile,"\nforward motion vectors (second vector, horizontal):\n");

    k=0;
    for (j=0; j<mb_height2; j++)
    {
      for (i=0; i<mb_width; i++)
      {
        if (cur_picture.mbinfo[k].mb_type & MB_FORWARD
            && ((cur_picture.pict_struct==FRAME_PICTURE && cur_picture.mbinfo[k].motion_type==MC_FIELD) ||
                (cur_picture.pict_struct!=FRAME_PICTURE && cur_picture.mbinfo[k].motion_type==MC_16X8)))
          fprintf(statfile,"%4d",cur_picture.mbinfo[k].MV[1][0][0]);
        else
          fprintf(statfile,"   .");
  
        k++;
      }
      putc('\n',statfile);
    }

    fprintf(statfile,"\nforward motion vectors (second vector, vertical):\n");

    k=0;
    for (j=0; j<mb_height2; j++)
    {
      for (i=0; i<mb_width; i++)
      {
        if (cur_picture.mbinfo[k].mb_type & MB_FORWARD
            && ((cur_picture.pict_struct==FRAME_PICTURE && cur_picture.mbinfo[k].motion_type==MC_FIELD) ||
                (cur_picture.pict_struct!=FRAME_PICTURE && cur_picture.mbinfo[k].motion_type==MC_16X8)))
          fprintf(statfile,"%4d",cur_picture.mbinfo[k].MV[1][0][1]);
        else
          fprintf(statfile,"   .");
  
        k++;
      }
      putc('\n',statfile);
    }


  }
   
  if (cur_picture.pict_type==B_TYPE)
  {
    fprintf(statfile,"\nbackward motion vectors (first vector, horizontal):\n");

    k=0;
    for (j=0; j<mb_height2; j++)
    {
      for (i=0; i<mb_width; i++)
      {
        if (cur_picture.mbinfo[k].mb_type & MB_BACKWARD)
          fprintf(statfile,"%4d",cur_picture.mbinfo[k].MV[0][1][0]);
        else
          fprintf(statfile,"   .");
  
        k++;
      }
      putc('\n',statfile);
    }

    fprintf(statfile,"\nbackward motion vectors (first vector, vertical):\n");

    k=0;
    for (j=0; j<mb_height2; j++)
    {
      for (i=0; i<mb_width; i++)
      {
        if (cur_picture.mbinfo[k].mb_type & MB_BACKWARD)
          fprintf(statfile,"%4d",cur_picture.mbinfo[k].MV[0][1][1]);
        else
          fprintf(statfile,"   .");
  
        k++;
      }
      putc('\n',statfile);
    }

    fprintf(statfile,"\nbackward motion vectors (second vector, horizontal):\n");

    k=0;
    for (j=0; j<mb_height2; j++)
    {
      for (i=0; i<mb_width; i++)
      {
        if (cur_picture.mbinfo[k].mb_type & MB_BACKWARD
            && ((cur_picture.pict_struct==FRAME_PICTURE && cur_picture.mbinfo[k].motion_type==MC_FIELD) ||
                (cur_picture.pict_struct!=FRAME_PICTURE && cur_picture.mbinfo[k].motion_type==MC_16X8)))
          fprintf(statfile,"%4d",cur_picture.mbinfo[k].MV[1][1][0]);
        else
          fprintf(statfile,"   .");
  
        k++;
      }
      putc('\n',statfile);
    }

    fprintf(statfile,"\nbackward motion vectors (second vector, vertical):\n");

    k=0;
    for (j=0; j<mb_height2; j++)
    {
      for (i=0; i<mb_width; i++)
      {
        if (cur_picture.mbinfo[k].mb_type & MB_BACKWARD
            && ((cur_picture.pict_struct==FRAME_PICTURE && cur_picture.mbinfo[k].motion_type==MC_FIELD) ||
                (cur_picture.pict_struct!=FRAME_PICTURE && cur_picture.mbinfo[k].motion_type==MC_16X8)))
          fprintf(statfile,"%4d",cur_picture.mbinfo[k].MV[1][1][1]);
        else
          fprintf(statfile,"   .");
  
        k++;
      }
      putc('\n',statfile);
    }


  }

  if( frame_num < 10 || frame_num > 24 )
	  return;
  /* useful for debugging */
  fprintf(statfile,"\nmacroblock info dump:\n");

  k=0;
 for (j=10; j<mb_height2; j++)
  {
    for (i=0; i<mb_width; i++)
    {
			  k = j*mb_width+i;
      fprintf(statfile,"(%d,%d): %02x %1.1f %d %3.1f %02x %d\n",
			  i,j,
		        cur_picture.mbinfo[k].mb_type,
			  cur_picture.mbinfo[k].N_act,
      cur_picture.mbinfo[k].mquant,
	  cur_picture.mbinfo[k].act,
      cur_picture.mbinfo[k].cbp,
      cur_picture.mbinfo[k].skipped			  );
	}
  }

#if 0
 k=0;
  for (j=8; j<10; j++)
  {
    for (i=14; i<17; i++)
    {
			  k = j*mb_width+i;

      fprintf(statfile,"(%d,%d): %02x %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
      i,j,
      cur_picture.mbinfo[k].motion_type,
      cur_picture.mbinfo[k].dct_type,
      cur_picture.mbinfo[k].MV[0][0][0],
      cur_picture.mbinfo[k].MV[0][0][1],
      cur_picture.mbinfo[k].MV[0][1][0],
      cur_picture.mbinfo[k].MV[0][1][1],
      cur_picture.mbinfo[k].MV[1][0][0],
      cur_picture.mbinfo[k].MV[1][0][1],
      cur_picture.mbinfo[k].MV[1][1][0],
      cur_picture.mbinfo[k].MV[1][1][1],
      cur_picture.mbinfo[k].mv_field_sel[0][0],
      cur_picture.mbinfo[k].mv_field_sel[0][1],
      cur_picture.mbinfo[k].mv_field_sel[1][0],
      cur_picture.mbinfo[k].mv_field_sel[1][1]);

    }
  }

#endif
#endif
}
