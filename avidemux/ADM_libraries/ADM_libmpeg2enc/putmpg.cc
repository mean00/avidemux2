/* putmpg.c, block and motion vector encoding routines                      */

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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include "global.h"

/* generate variable length codes for an intra-coded block (6.2.6, 6.3.17) */
void putintrablk(Picture *picture, int16_t *blk, int cc)
{
  int n, dct_diff, run, signed_level;

  /* DC coefficient (7.2.1) */
  dct_diff = blk[0] - picture->dc_dct_pred[cc]; /* difference to previous block */
  picture->dc_dct_pred[cc] = blk[0];

  if (cc==0)
    putDClum(dct_diff);
  else
    putDCchrom(dct_diff);

  /* AC coefficients (7.2.2) */
  run = 0;
  const uint8_t *scan_tbl = (picture->altscan ? alternate_scan : zig_zag_scan);
  for (n=1; n<64; n++)
  {
    /* use appropriate entropy scanning pattern */
    signed_level = blk[scan_tbl[n]];
    if (signed_level!=0)
    {
      putAC(run,signed_level,picture->intravlc);
      run = 0;
    }
    else
      run++; /* count zero coefficients */
  }

  /* End of Block -- normative block punctuation */
  if (picture->intravlc)
    putbits(6,4); /* 0110 (Table B-15) */
  else
    putbits(2,2); /* 10 (Table B-14) */
}

/* generate variable length codes for a non-intra-coded block (6.2.6, 6.3.17) */
void putnonintrablk(Picture *picture, int16_t *blk)
{
  int n, run, signed_level, first;

  run = 0;
  first = 1;

  for (n=0; n<64; n++)
  {
    /* use appropriate entropy scanning pattern */
    signed_level = blk[(picture->altscan ? alternate_scan : zig_zag_scan)[n]];

    if (signed_level!=0)
    {
      if (first)
      {
        /* first coefficient in non-intra block */
        putACfirst(run,signed_level);
        first = 0;
      }
      else
        putAC(run,signed_level,0);

      run = 0;
    }
    else
      run++; /* count zero coefficients */
  }

  /* End of Block -- normative block punctuation  */
  putbits(2,2);
}

/* generate variable length code for a motion vector component (7.6.3.1) */
void putmv(int dmv, int f_code)
{
  int r_size, f, vmin, vmax, dv, temp, motion_code, motion_residual;

  r_size = f_code - 1; /* number of fixed length code ('residual') bits */
  f = 1<<r_size;
  vmin = -16*f; /* lower range limit */
  vmax = 16*f - 1; /* upper range limit */
  dv = 32*f;

  /* fold vector difference into [vmin...vmax] */
  if (dmv>vmax)
    dmv-= dv;
  else if (dmv<vmin)
    dmv+= dv;

  /* check value */
  if (dmv<vmin || dmv>vmax)
  {
      fprintf(stderr,"Too large MV %03d not in [%04d..:%03d]\n", dmv, vmin, vmax);
      exit(1);
  }

  /* split dmv into motion_code and motion_residual */
  temp = ((dmv<0) ? -dmv : dmv) + f - 1;
  motion_code = temp>>r_size;
  if (dmv<0)
    motion_code = -motion_code;
  motion_residual = temp & (f-1);

  putmotioncode(motion_code); /* variable length code */

  if (r_size!=0 && motion_code!=0)
    putbits(motion_residual,r_size); /* fixed length code */
}
