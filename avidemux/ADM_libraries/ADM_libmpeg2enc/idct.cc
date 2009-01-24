/* idct.c, inverse fast discrete cosine transform                           */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "mjpeg_types.h"
#include "mjpeg_logging.h"
#include "transfrm_ref.h"

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

/**********************************************************/
/* inverse two dimensional DCT, Chen-Wang algorithm       */
/* (cf. IEEE ASSP-32, pp. 803-816, Aug. 1984)             */
/* 32-bit integer arithmetic (8 bit coefficients)         */
/* 11 mults, 29 adds per DCT                              */
/*                                      sE, 18.8.91       */
/**********************************************************/
/* coefficients extended to 12 bit for IEEE1180-1990      */
/* compliance                           sE,  2.1.94       */
/**********************************************************/

/* this code assumes >> to be a two's-complement arithmetic */
/* right shift: (-2)>>1 == -1 , (-3)>>1 == -2               */


// define if you want to include idct testing code
#define IDCTTEST


#define W1 2841 /* 2048*sqrt(2)*cos(1*pi/16) */
#define W2 2676 /* 2048*sqrt(2)*cos(2*pi/16) */
#define W3 2408 /* 2048*sqrt(2)*cos(3*pi/16) */
#define W5 1609 /* 2048*sqrt(2)*cos(5*pi/16) */
#define W6 1108 /* 2048*sqrt(2)*cos(6*pi/16) */
#define W7 565  /* 2048*sqrt(2)*cos(7*pi/16) */

/* global declarations */

/* private data */
static int16_t iclip[1024]; /* clipping table */
static int16_t *iclp;

/* private prototypes */
static void idctrow (int16_t *blk);
static void idctcol (int16_t *blk);

/* row (horizontal) IDCT
 *
 *           7                       pi         1
 * dst[k] = sum c[l] * src[l] * cos( -- * ( k + - ) * l )
 *          l=0                      8          2
 *
 * where: c[0]    = 128
 *        c[1..7] = 128*sqrt(2)
 */

static void idctrow(int16_t *blk)
{
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;

  /* int16_tcut */
  if (!((x1 = blk[4]<<11) | (x2 = blk[6]) | (x3 = blk[2]) |
        (x4 = blk[1]) | (x5 = blk[7]) | (x6 = blk[5]) | (x7 = blk[3])))
  {
    blk[0]=blk[1]=blk[2]=blk[3]=blk[4]=blk[5]=blk[6]=blk[7]=blk[0]<<3;
    return;
  }

  x0 = (blk[0]<<11) + 128; /* for proper rounding in the fourth stage */

  /* first stage */
  x8 = W7*(x4+x5);
  x4 = x8 + (W1-W7)*x4;
  x5 = x8 - (W1+W7)*x5;
  x8 = W3*(x6+x7);
  x6 = x8 - (W3-W5)*x6;
  x7 = x8 - (W3+W5)*x7;
  
  /* second stage */
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6*(x3+x2);
  x2 = x1 - (W2+W6)*x2;
  x3 = x1 + (W2-W6)*x3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  
  /* third stage */
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181*(x4+x5)+128)>>8;
  x4 = (181*(x4-x5)+128)>>8;
  
  /* fourth stage */
  blk[0] = (x7+x1)>>8;
  blk[1] = (x3+x2)>>8;
  blk[2] = (x0+x4)>>8;
  blk[3] = (x8+x6)>>8;
  blk[4] = (x8-x6)>>8;
  blk[5] = (x0-x4)>>8;
  blk[6] = (x3-x2)>>8;
  blk[7] = (x7-x1)>>8;
}

/* column (vertical) IDCT
 *
 *             7                         pi         1
 * dst[8*k] = sum c[l] * src[8*l] * cos( -- * ( k + - ) * l )
 *            l=0                        8          2
 *
 * where: c[0]    = 1/1024
 *        c[1..7] = (1/1024)*sqrt(2)
 */
static void idctcol(int16_t *blk)
{
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;

  /* int16_tcut */
  if (!((x1 = (blk[8*4]<<8)) | (x2 = blk[8*6]) | (x3 = blk[8*2]) |
        (x4 = blk[8*1]) | (x5 = blk[8*7]) | (x6 = blk[8*5]) | (x7 = blk[8*3])))
  {
    blk[8*0]=blk[8*1]=blk[8*2]=blk[8*3]=blk[8*4]=blk[8*5]=blk[8*6]=blk[8*7]=
      iclp[(blk[8*0]+32)>>6];
    return;
  }

  x0 = (blk[8*0]<<8) + 8192;

  /* first stage */
  x8 = W7*(x4+x5) + 4;
  x4 = (x8+(W1-W7)*x4)>>3;
  x5 = (x8-(W1+W7)*x5)>>3;
  x8 = W3*(x6+x7) + 4;
  x6 = (x8-(W3-W5)*x6)>>3;
  x7 = (x8-(W3+W5)*x7)>>3;
  
  /* second stage */
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6*(x3+x2) + 4;
  x2 = (x1-(W2+W6)*x2)>>3;
  x3 = (x1+(W2-W6)*x3)>>3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  
  /* third stage */
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181*(x4+x5)+128)>>8;
  x4 = (181*(x4-x5)+128)>>8;
  
  /* fourth stage */
  blk[8*0] = iclp[(x7+x1)>>14];
  blk[8*1] = iclp[(x3+x2)>>14];
  blk[8*2] = iclp[(x0+x4)>>14];
  blk[8*3] = iclp[(x8+x6)>>14];
  blk[8*4] = iclp[(x8-x6)>>14];
  blk[8*5] = iclp[(x0-x4)>>14];
  blk[8*6] = iclp[(x3-x2)>>14];
  blk[8*7] = iclp[(x7-x1)>>14];
}

/* two dimensional inverse discrete cosine transform */
void idct(int16_t *block)
{
  int i;

  for (i=0; i<8; i++)
    idctrow(block+8*i);

  for (i=0; i<8; i++)
    idctcol(block+i);
}

#ifdef IDCTTEST

extern void idct_mmx(int16_t *blk);
extern void idct_sse(int16_t *blk);

static double coslu[8][8];

/* reference idct taken from "ieeetest.c"
 * Written by Tom Lane (tgl@cs.cmu.edu).
 * Released to public domain 11/22/93.
 */   
void idct_ref(int16_t *block)
{
    int x,y,u,v;
    double tmp, tmp2;
    double res[8][8];

    for (y=0; y<8; y++) {
        for (x=0; x<8; x++) {
            tmp = 0.0;
            for (v=0; v<8; v++) {
                tmp2 = 0.0;
                for (u=0; u<8; u++) {
                    tmp2 += (double) block[v*8+u] * coslu[x][u];
                }
                tmp += coslu[y][v] * tmp2;
            }
            res[y][x] = tmp;
        }
    }

    for (v=0; v<8; v++) {
        for (u=0; u<8; u++) {
            tmp = res[v][u];
            if (tmp < 0.0) {
                x = - ((int) (0.5 - tmp));
            } else {
                x = (int) (tmp + 0.5);
            }
            block[v*8+u] = x;
        }
    }
}

void init_idct_ref(void)
{
  int a,b;
  double tmp;

  for(a=0;a<8;a++)
    for(b=0;b<8;b++) {
      tmp = cos((double)((a+a+1)*b) * (3.14159265358979323846 / 16.0));
      if(b==0)
	tmp /= sqrt(2.0);
      coslu[a][b] = tmp * 0.5;
    }
}

struct dct_test {
    int bounds,maxerr,iter;
    int me[64],mse[64];
};

void dct_test_and_print(struct dct_test *dt,int range,int16_t *origblock,int16_t *block)
{
    int b,m,i;

    b=0;
    m=0;
    for( i=0; i<64; i++ ) {
        int x=block[i]-origblock[i];
        int ax=abs(x);
        dt->me[i]+=x;
        dt->mse[i]+=x*x;
        if( ax>m )
            m=ax;
        if( block[i]<-range || block[i]>=range )
            b++;
        if( origblock[i]<-range || origblock[i]>=range ) {
            // mjpeg_info("*********** REFERENCE VERSION OUT OF BOUNDS\n");
        }
    }
    dt->bounds+=b;
    if (m > dt->maxerr )
        dt->maxerr = m;
    dt->iter++;
    if( !(dt->iter&65535) ) {
        int sme=0,srms=0;

        for( i=0; i<64; i++ ) {
            sme+=dt->me[i];
            srms+=dt->mse[i];
        }
        mjpeg_info("dct_test[%d]: max error=%d, mean error=%.8f, rms error=%.8f; bounds err=%d\n",
                   dt->iter,dt->maxerr,
                   sme/(dt->iter*64.),
                   srms/(dt->iter*64.),
                   dt->bounds);
        for( i=0; i<8; i++ ) {
            int j;

            for( j=0; j<8; j++ )
                fprintf(stderr,"%9.6f%c",((double)dt->me[i*8+j])/dt->iter,j==7?'\n':' ');
            for( j=0; j<8; j++ )
                fprintf(stderr,"%9.6f%c",((double)dt->mse[i*8+j])/dt->iter,j==7?'\n':' ');
            fprintf(stderr,"\n");
        }
    }
}

static struct dct_test idct_res;

void idct_test(int16_t *block)
{
    int16_t origblock[64];

    memcpy(origblock,block,64*sizeof(int16_t));

    idct_ref(origblock);
    // idct(origblock);

    idct(block);
    // idct_mmx(block);
    // idct_sse(block);

    dct_test_and_print(&idct_res,256,origblock,block);
}
#endif

void init_idct(void)
{
  int i;

  iclp = iclip+512;
  for (i= -512; i<512; i++)
    iclp[i] = (i<-256) ? -256 : ((i>255) ? 255 : i);

#ifdef IDCTTEST
  memset(&idct_res,0,sizeof(idct_res));
  init_idct_ref();
#endif
}
