#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_X86CPU
#include "mjpeg_types.h"
#include "mmx.h"

#define ROOT2OVER2 0.70710678118654757

#define W0 1
#define W1 1.3870398453221475  /* sqrt(2)*cos(1*pi/16) */
#define W2 1.3065629648763766  /* sqrt(2)*cos(2*pi/16) */
#define W3 1.1758756024193588  /* sqrt(2)*cos(3*pi/16) */
#define W4 1
#define W5 0.78569495838710235 /* sqrt(2)*cos(5*pi/16) */
#define W6 0.54119610014619712 /* sqrt(2)*cos(6*pi/16) */
#define W7 0.27589937928294311 /* sqrt(2)*cos(7*pi/16) */

#define WIFY(b,v) ((int)((b)*(v)+.5))

#define Wr 16384
#define Wr0 WIFY(Wr,W0)
#define Wr1 WIFY(Wr,W1)
#define Wr2 WIFY(Wr,W2)
#define Wr3 WIFY(Wr,W3)
#define Wr4 WIFY(Wr,W4)
#define Wr5 WIFY(Wr,W5)
#define Wr6 WIFY(Wr,W6)
#define Wr7 WIFY(Wr,W7)

#define Wrshift 11
#define Wrround (1<<((Wrshift)-1))

#define Wc 16384
#define Wc0 WIFY(Wc,W0)
#define Wc1 WIFY(Wc,W1)
#define Wc2 WIFY(Wc,W2)
#define Wc3 WIFY(Wc,W3)
#define Wc4 WIFY(Wc,W4)
#define Wc5 WIFY(Wc,W5)
#define Wc6 WIFY(Wc,W6)
#define Wc7 WIFY(Wc,W7)

#define Wcshift 20
#define Wcround (1<<((Wcshift)-1))

// NOTE: Wr*Wc == (1<<(Wrshift+Wcshift))
// also, Wr <= 8<<Wrshift (seems to be a good relationship to keep the intermediate matrix in bounds

static int16_t idct_mmx_row_table[32]={
    Wr0,  -Wr4,  Wr0, -Wr4,
    Wr4,   Wr0,  Wr4,  Wr0,
    -Wr2,  Wr6, -Wr2,  Wr6,
    Wr6,   Wr2,  Wr6,  Wr2,
    Wr5,   Wr3,  Wr5,  Wr3,
    Wr3,  -Wr5,  Wr3, -Wr5,
    Wr1,   Wr7,  Wr1,  Wr7,
    Wr7,  -Wr1,  Wr7, -Wr1
};

#if Wr==Wc
#define idct_mmx_col_table idct_mmx_row_table
#else
static int16_t idct_mmx_row_table[32]={
    Wc0,  -Wc4,  Wc0, -Wc4,
    Wc4,   Wc0,  Wc4,  Wc0,
    -Wc2,  Wc6, -Wc2,  Wc6,
    Wc6,   Wc2,  Wc6,  Wc2,
    Wc5,   Wc3,  Wc5,  Wc3,
    Wc3,  -Wc5,  Wc3, -Wc5,
    Wc1,   Wc7,  Wc1,  Wc7,
    Wc7,  -Wc1,  Wc7, -Wc1
};
#endif

static int32_t idct_mmx_row_round[2]={
    Wrround, Wrround
};

static int32_t idct_mmx_col_round[2]={
    Wcround, Wcround
};

#define RPT4(x) x, x, x, x
#define SSEMAT(A,B,C,D) RPT4((A)/(B)), RPT4(((D)*(A))/(C)-(B)), RPT4((B)), RPT4((C)/(A))
#define ALIGN_PTR(x,a) ((void *)( (((size_t)(x))+(a)-1)&(-(a))))
#define SHUFFLEMAP(A,B,C,D) ((A)*1+(B)*4+(C)*16+(D)*64)

static float idct_sse_table[64] __attribute__ ((aligned (16)))={
    SSEMAT(W0, -W4, W4,  W0),
    SSEMAT(-W2, W6, W6,  W2),
    SSEMAT(W5,  W3, W3, -W5),
    SSEMAT(W1,  W7, W7, -W1)
};

static float idct_sse_root2_over2[4] __attribute__ ((aligned (16)))={ RPT4(ROOT2OVER2) };
static float idct_sse_eighth[4] __attribute__ ((aligned (16)))={ RPT4(1./8.) };

// computes A, B = A*x[0] + B*x[1], A*x[4] + B*x[5]
#define MULTADD(A,B,x) { int t=(A)*(x)[0]+(B)*(x)[1]; (B)=(A)*(x)[4]+(B)*(x)[5]; (A)=t; }
/*
  on register starved platforms, it can be exected as:

  x, y = A*x + B*y, C*x + D*y;

  x*=I;  I*x, y
  x+=y;  I*x + y, y
  y*=K;  I*x + y, K*y
  x*=B;  B*I*x + B*y, K*y
  y+=x;  B*I*x + B*y, A*x + (B+K)*y
  y*=L;  B*I*x + B*y, L*A*x + L*(B+K)*y
  
  I = A/B
  K = (D*A)/C-B
  B
  L = C/A
*/
#define MMXMULTADD(x,y,t)       \
        movq_r2r(x, y);         \
        pmaddwd_m2r((t)[0], x); \
        pmaddwd_m2r((t)[4], y);

#define SSEMULTADD(x,y,t)  \
      mulps_m2r((t)[0], x);  \
      addps_r2r(y, x);     \
      mulps_m2r((t)[4], y);  \
      mulps_m2r((t)[8], x);  \
      addps_r2r(x, y);     \
      mulps_m2r((t)[12], y);


// computes A, B = A-B, A+B;
#define ADDDIFF(A,B) { int t=(A); (A)-=(B); (B)+=t; }
/*
  on register starved platforms, it can be executed as:

  A-=B;  A-B, B
  B+=B;  A-B, 2B
  B+=A;  A-B, A+B
*/
#define MMXADDDIFF(x,y)  \
        psubd_r2r(y, x); \
        paddd_r2r(y, y); \
        paddd_r2r(x, y);

#define SSEADDDIFF_t(A,B,C) \
        movaps_r2r(A,C);    \
        subps_r2r(B,A);     \
        addps_r2r(C,B);

#define SSEADDDIFF(x,y)  \
        subps_r2r(y, x); \
        addps_r2r(y, y); \
        addps_r2r(x, y);

// the MMX and SSE versions effectively implement this for idctrow:

/*
        x0 = src[0];
        x1 = src[4];
        x2 = src[6];
        x3 = src[2];
        x4 = src[5];
        x5 = src[3];
        x6 = src[1];
        x7 = src[7];

        // first stage
        
        MULTADD(x0, x1, idct_mmx_row_table);
        x0+=Wrround;
        x1+=Wrround;
        
        MULTADD(x2, x3, idct_mmx_row_table+8);

        MULTADD(x4, x5, idct_mmx_row_table+16);

        MULTADD(x6, x7, idct_mmx_row_table+24);
  
        // second stage
        ADDDIFF(x6, x4);

        ADDDIFF(x7, x5);
  
        // third stage
        ADDDIFF(x1, x3);

        ADDDIFF(x0, x2);

        ADDDIFF(x6, x7);

        // fourth stage
        ADDDIFF(x3, x4);

        ADDDIFF(x1, x5);

        dst[0] = (x4)>>Wrshift;
        dst[3] = (x5)>>Wrshift;
        dst[4] = (x1)>>Wrshift;
        dst[7] = (x3)>>Wrshift;

        x7 = MUL_BY_ROOT_2_OVER_2(x7);
        x6 = MUL_BY_ROOT_2_OVER_2(x6);
  
        ADDDIFF(x2, x7);
        ADDDIFF(x0, x6);

        dst[1] = (x7)>>Wrshift;
        dst[2] = (x6)>>Wrshift;
        dst[5] = (x0)>>Wrshift;
        dst[6] = (x2)>>Wrshift;
*/

void mp2_idct_mmx(int16_t *block)
{
    int16_t temp[64], *src, *dst;
    int i;

    src=block;
    dst=temp;
    for( i=0; i<4; src+=16, dst+=2, i++ ) {

#define STOREMM(r, d, s)    \
        psrad_i2r(s, r);    \
        packssdw_r2r(r, r); \
        movd_r2m(r, d);


        /* first stage */
        // x0, x1 =  W0*x0 + W1*x1 + 128, W1*x0 - W0*x1 + 128;
        // x2, x3 = -W2*x2 + W6*x3, W6*x2 + W2*x3;
        // x4, x5 =  W1*x4 + W7*x5, W7*x4 - W1*x5;    
        // x6, x7 =  W5*x6 + W3*x7, W3*x6 - W5*x7;
    

        movq_m2r(src[0], mm0);
        movq_m2r(src[8], mm1);
        movq_r2r(mm0, mm2);
        punpckldq_r2r(mm1, mm0);
        punpckhdq_r2r(mm1, mm2);

        movq_m2r(src[4], mm4);
        movq_m2r(src[12], mm5);
        movq_r2r(mm4, mm6);
        punpckldq_r2r(mm5, mm4);
        punpckhdq_r2r(mm5, mm6);

        // mm5 = low word set
        // mm7 = high word set
        pxor_r2r(mm5, mm5);
        pcmpeqw_r2r(mm5, mm5);
        movq_r2r(mm5, mm7);
        psrld_i2r(16, mm5);
        pxor_r2r(mm5, mm7);

        movq_r2r(mm4, mm1);

        // 0,1 / 2,3 / 4,5 / 6,7 / 1=4,5
        movq_r2r(mm0, mm3);
        pand_r2r(mm5, mm0);
        pslld_i2r(16, mm1);
        por_r2r(mm1, mm0);

        // 0,4 / 2,3 / 4,5 / 6,7 / 3=0,1
        movq_r2r(mm6, mm1);
        pand_r2r(mm7, mm6);
        psrld_i2r(16, mm3);
        por_r2r(mm3, mm6);

        // 0,4 / 2,3 / 4,5 / 1,7 / 1=6,7
        movq_r2r(mm2, mm3);
        pslld_i2r(16, mm2);
        pand_r2r(mm5, mm1);
        por_r2r(mm1, mm2);

        // 0,4 / 6,2 / 4,5 / 1,7 / 3=2,3
        psrld_i2r(16, mm4);
        pand_r2r(mm7, mm3);
        por_r2r(mm3, mm4);

        // 0,4 / 6,2 / 5,3 / 1,7           

        MMXMULTADD(mm0, mm1, idct_mmx_row_table);
        MMXMULTADD(mm2, mm3, idct_mmx_row_table+8);
        MMXMULTADD(mm4, mm5, idct_mmx_row_table+16);
        MMXMULTADD(mm6, mm7, idct_mmx_row_table+24);

        paddd_m2r(idct_mmx_row_round[0], mm0);
        paddd_m2r(idct_mmx_row_round[0], mm1);

        /* second stage */

        MMXADDDIFF(mm6, mm4);    
        MMXADDDIFF(mm7, mm5);
        
        /* third stage */
        
        MMXADDDIFF( mm1, mm3 );
        MMXADDDIFF( mm0, mm2 );
        MMXADDDIFF( mm6, mm7 );
    
        /* fourth stage */

        MMXADDDIFF( mm3, mm4 );
        MMXADDDIFF( mm1, mm5 );

        /* fifth stage */

        STOREMM(mm4, dst[0*8], Wrshift);
        STOREMM(mm5, dst[3*8], Wrshift);
        STOREMM(mm1, dst[4*8], Wrshift);
        STOREMM(mm3, dst[7*8], Wrshift);

        /* sixth stage */
        // x6 = (181*x6+128)>>8;
        // x7 = (181*x7+128)>>8;
      
        // actually, this computes, roughly: x6 -= (x6>>8)*75
        movq_r2r(mm6, mm4);
        movq_r2r(mm7, mm5);

        psrad_i2r(2, mm6);
        psrad_i2r(2, mm7);
        psubd_r2r(mm6, mm4);
        psubd_r2r(mm7, mm5);

        psrad_i2r(3, mm6);
        psrad_i2r(3, mm7);
        psubd_r2r(mm6, mm4);
        psubd_r2r(mm7, mm5);

        psrad_i2r(2, mm6);
        psrad_i2r(2, mm7);
        psubd_r2r(mm6, mm4);
        psubd_r2r(mm7, mm5);

        psrad_i2r(1, mm6);
        psrad_i2r(1, mm7);
        psubd_r2r(mm6, mm4);
        psubd_r2r(mm7, mm5);
    
        /* seventh stage */

        MMXADDDIFF( mm2, mm5 );
        MMXADDDIFF( mm0, mm4 );
        
        /* eighth stage */

        STOREMM(mm5, dst[1*8], Wrshift);
        STOREMM(mm4, dst[2*8], Wrshift);
        STOREMM(mm0, dst[5*8], Wrshift);
        STOREMM(mm2, dst[6*8], Wrshift);
    }

    src=temp;
    dst=block;
    for( i=0; i<4; src+=16, dst+=2, i++ ) {

        /* first stage */
        // x0, x1 =  W0*x0 + W1*x1 + 128, W1*x0 - W0*x1 + 128;
        // x2, x3 = -W2*x2 + W6*x3, W6*x2 + W2*x3;
        // x4, x5 =  W1*x4 + W7*x5, W7*x4 - W1*x5;    
        // x6, x7 =  W5*x6 + W3*x7, W3*x6 - W5*x7;
    
        movq_m2r(src[0], mm0);
        movq_m2r(src[8], mm1);
        movq_r2r(mm0, mm2);
        punpckldq_r2r(mm1, mm0);
        punpckhdq_r2r(mm1, mm2);

        movq_m2r(src[4], mm4);
        movq_m2r(src[12], mm5);
        movq_r2r(mm4, mm6);
        punpckldq_r2r(mm5, mm4);
        punpckhdq_r2r(mm5, mm6);

        // mm5 = low word set
        // mm7 = high word set
        pxor_r2r(mm5, mm5);
        pcmpeqw_r2r(mm5, mm5);
        movq_r2r(mm5, mm7);
        psrld_i2r(16, mm5);
        pxor_r2r(mm5, mm7);

        movq_r2r(mm4, mm1);

        // 0,1 / 2,3 / 4,5 / 6,7 / 1=4,5
        movq_r2r(mm0, mm3);
        pand_r2r(mm5, mm0);
        pslld_i2r(16, mm1);
        por_r2r(mm1, mm0);

        // 0,4 / 2,3 / 4,5 / 6,7 / 3=0,1
        movq_r2r(mm6, mm1);
        pand_r2r(mm7, mm6);
        psrld_i2r(16, mm3);
        por_r2r(mm3, mm6);

        // 0,4 / 2,3 / 4,5 / 1,7 / 1=6,7
        movq_r2r(mm2, mm3);
        pslld_i2r(16, mm2);
        pand_r2r(mm5, mm1);
        por_r2r(mm1, mm2);

        // 0,4 / 6,2 / 4,5 / 1,7 / 3=2,3
        psrld_i2r(16, mm4);
        pand_r2r(mm7, mm3);
        por_r2r(mm3, mm4);

        // 0,4 / 6,2 / 5,3 / 1,7           

        MMXMULTADD(mm0, mm1, idct_mmx_col_table);
        MMXMULTADD(mm2, mm3, idct_mmx_col_table+8);
        MMXMULTADD(mm4, mm5, idct_mmx_col_table+16);
        MMXMULTADD(mm6, mm7, idct_mmx_col_table+24);

        paddd_m2r(idct_mmx_col_round[0], mm0);
        paddd_m2r(idct_mmx_col_round[0], mm1);

        /* second stage */

        MMXADDDIFF(mm6, mm4);    
        MMXADDDIFF(mm7, mm5);
        
        /* third stage */
        
        MMXADDDIFF( mm1, mm3 );
        MMXADDDIFF( mm0, mm2 );
        MMXADDDIFF( mm6, mm7 );
    
        /* fourth stage */

        MMXADDDIFF( mm3, mm4 );
        MMXADDDIFF( mm1, mm5 );

        /* fifth stage */

        STOREMM(mm4, dst[0*8], Wcshift);
        STOREMM(mm5, dst[3*8], Wcshift);
        STOREMM(mm1, dst[4*8], Wcshift);
        STOREMM(mm3, dst[7*8], Wcshift);

        /* sixth stage */
        // x6 = (181*x6+128)>>8;
        // x7 = (181*x7+128)>>8;
      
        // actually, this computes, roughly: x6 -= (x6>>8)*75
        movq_r2r(mm6, mm4);
        movq_r2r(mm7, mm5);

        psrad_i2r(2, mm6);
        psrad_i2r(2, mm7);
        psubd_r2r(mm6, mm4);
        psubd_r2r(mm7, mm5);

        psrad_i2r(3, mm6);
        psrad_i2r(3, mm7);
        psubd_r2r(mm6, mm4);
        psubd_r2r(mm7, mm5);

        psrad_i2r(2, mm6);
        psrad_i2r(2, mm7);
        psubd_r2r(mm6, mm4);
        psubd_r2r(mm7, mm5);

        psrad_i2r(1, mm6);
        psrad_i2r(1, mm7);
        psubd_r2r(mm6, mm4);
        psubd_r2r(mm7, mm5);
    
        /* seventh stage */

        MMXADDDIFF( mm2, mm5 );
        MMXADDDIFF( mm0, mm4 );

        /* eighth stage */

        STOREMM(mm5, dst[1*8], Wcshift);
        STOREMM(mm4, dst[2*8], Wcshift);
        STOREMM(mm0, dst[5*8], Wcshift);
        STOREMM(mm2, dst[6*8], Wcshift);
    }

    emms();
}

void mp2_idct_sse(int16_t *block)
{
    float temp[64+3], *dst, *altemp;
    int i;
    int16_t *src;

    altemp=(float *)ALIGN_PTR(temp,16);

    src=block;
    dst=altemp;
    for( i=0; i<2; src+=32, dst+=4, i++) {
#define MM2XMMl(mm,x0,x1)          \
        movq_r2r     ( mm,  mm6);  \
        movq_r2r     ( mm,  mm7);  \
        psraw_i2r    ( 16,  mm);   \
        punpcklwd_r2r( mm,  mm6);  \
        punpckhwd_r2r( mm,  mm7);  \
        cvtpi2ps_r2r ( mm6, x0);   \
        cvtpi2ps_r2r ( mm7, x1);

#define MM2XMM(ml,mh,x0,x1)  \
        MM2XMMl(mh,x0,x1);   \
        movlhps_r2r(x0, x0); \
        movlhps_r2r(x1, x1); \
        MM2XMMl(ml,x0,x1);

#define LOADROTATEif(src, x0, x1, x2, x3)           \
                                                    \
        movq_m2r((src)[16], mm2); /* 0c 1c 2c 3c */ \
        movq_m2r((src)[24], mm3); /* 0d 1d 2d 3d */ \
        movq_m2r((src)[ 0], mm0); /* 0a 1a 2a 3a */ \
        movq_m2r((src)[ 8], mm1); /* 0b 1b 2b 3b */ \
                                                    \
        /* mm5 = 0c 0d 1c 1d */                     \
        /* mm2 = 2c 2d 3c 3d */                     \
        movq_r2r(mm2, mm5);                         \
        punpcklwd_r2r(mm3, mm5);                    \
        punpckhwd_r2r(mm3, mm2);                    \
                                                    \
        /* mm4 = 0a 0b 1a 1b */                     \
        /* mm0 = 2a 2b 3a 3b */                     \
        movq_r2r(mm0, mm4);                         \
        punpcklwd_r2r(mm1, mm4);                    \
        punpckhwd_r2r(mm1, mm0);                    \
                                                    \
        MM2XMM(mm4, mm5, x0, x1);                   \
        MM2XMM(mm0, mm2, x2, x3);

        LOADROTATEif(src,   xmm0, xmm6, xmm3, xmm5);
        LOADROTATEif(src+4, xmm1, xmm4, xmm2, xmm7);

        // first stage

        SSEADDDIFF( xmm0, xmm1 );
        SSEMULTADD( xmm2, xmm3, idct_sse_table+16 );
        SSEMULTADD( xmm4, xmm5, idct_sse_table+32 );
        SSEMULTADD( xmm6, xmm7, idct_sse_table+48 );
        
        // third stage

        SSEADDDIFF( xmm1, xmm3 );

        // second stage

        SSEADDDIFF( xmm6, xmm4 );
        SSEADDDIFF( xmm7, xmm5 );
  
        // fourth stage

        SSEADDDIFF( xmm3, xmm4 );
        SSEADDDIFF( xmm1, xmm5 );

        movaps_r2m( xmm3, dst[7*8]);
        movaps_r2m( xmm4, dst[0*8]);
        movaps_r2m( xmm1, dst[4*8]);
        movaps_r2m( xmm5, dst[3*8]);

        SSEADDDIFF_t( xmm6, xmm7, xmm1 );
        SSEADDDIFF_t( xmm0, xmm2, xmm1 );

        // x7 = MUL_BY_ROOT_2_OVER_2(x7);
        // x6 = MUL_BY_ROOT_2_OVER_2(x6);
        movaps_m2r( idct_sse_root2_over2[0], xmm1 );
        mulps_r2r( xmm1, xmm7 );
        mulps_r2r( xmm1, xmm6 );
  
        SSEADDDIFF_t(xmm2, xmm7, xmm1);
        SSEADDDIFF_t(xmm0, xmm6, xmm1);

        movaps_r2m( xmm2, dst[6*8] );
        movaps_r2m( xmm7, dst[1*8] );
        movaps_r2m( xmm0, dst[5*8] );
        movaps_r2m( xmm6, dst[2*8] );
        
    }

    src=block;
    dst=altemp;
    for( i=0; i<2; src+=4, dst+=32, i++) {

#define LOADROTATEff(src, t, x0, x1, x2, x3)           \
        movaps_m2r((src)[ 0], x0); /* 0a 1a 2a 3a */   \
        movaps_m2r((src)[ 8], t);  /* 0b 1b 2b 3b */   \
        movaps_m2r((src)[16], x3); /* 0c 1c 2c 3c */   \
        movaps_m2r((src)[24], x1); /* 0d 1d 2d 3d */   \
                                                       \
        /* mm0 = 0a 0b 1a 1b */                        \
        /* mm2 = 2a 2b 3a 3b */                        \
        movaps_r2r  (x0, x2);                          \
        unpcklps_r2r(t,  x0);                          \
        unpckhps_r2r(t,  x2);                          \
                                                       \
        /* mm3 = 0c 0d 1c 1d */                        \
        /* mmt = 2c 2d 3c 3d */                        \
        movaps_r2r  (x3, t);                           \
        unpcklps_r2r(x1, x3);                          \
        unpckhps_r2r(x1, t);                           \
                                                       \
        /* mm0 = 0a 0b 0c 0d */                        \
        /* mm1 = 1a 1b 1c 1d */                        \
        movaps_r2r (x3, x1);                           \
        movhlps_r2r(x0, x1);                           \
        movlhps_r2r(x3, x0);                           \
                                                       \
        /* mm2 = 2a 2b 2c 2d */                        \
        /* mm3 = 3a 3b 3c 3d */                        \
        movaps_r2r ( t, x3);                           \
        movhlps_r2r(x2, x3);                           \
        movlhps_r2r( t, x2);

#define STOREXMM(x0, dst)                \
        mulps_m2r(*idct_sse_eighth, x0); \
        cvtps2pi_r2r(x0,  mm0);          \
        movhlps_r2r (x0,  x0);           \
        cvtps2pi_r2r(x0,  mm1);          \
        packssdw_r2r(mm1, mm0);          \
        movq_r2m    (mm0, dst);


        LOADROTATEff(dst,   xmm1, xmm0, xmm6, xmm3, xmm5);
        movaps_r2m(xmm0, dst[0]);
        LOADROTATEff(dst+4, xmm0, xmm1, xmm4, xmm2, xmm7);
        movaps_m2r(dst[0], xmm0);

        // first stage
        
        SSEADDDIFF( xmm0, xmm1 );
        SSEMULTADD( xmm2, xmm3, idct_sse_table+16 );
        SSEMULTADD( xmm4, xmm5, idct_sse_table+32 );
        SSEMULTADD( xmm6, xmm7, idct_sse_table+48 );
  
        // third stage

        SSEADDDIFF( xmm1, xmm3 );
        // second stage

        SSEADDDIFF( xmm6, xmm4 );
        SSEADDDIFF( xmm7, xmm5 );  

        // fourth stage

        SSEADDDIFF( xmm3, xmm4 );
        SSEADDDIFF( xmm1, xmm5 );

        STOREXMM( xmm3, src[7*8]);
        STOREXMM( xmm4, src[0*8] );
        STOREXMM( xmm1, src[4*8]);
        STOREXMM( xmm5, src[3*8]);

        SSEADDDIFF_t( xmm6, xmm7, xmm1 );
        SSEADDDIFF_t( xmm0, xmm2, xmm1 );

        // x7 = MUL_BY_ROOT_2_OVER_2(x7);
        // x6 = MUL_BY_ROOT_2_OVER_2(x6);
        movaps_m2r( idct_sse_root2_over2[0], xmm1 );
        mulps_r2r( xmm1, xmm7 );
        mulps_r2r( xmm1, xmm6 );
  
        SSEADDDIFF_t(xmm2, xmm7, xmm1);
        SSEADDDIFF_t(xmm0, xmm6, xmm1);

        STOREXMM( xmm2, src[6*8] );        
        STOREXMM( xmm7, src[1*8] );
        STOREXMM( xmm0, src[5*8] );
        STOREXMM( xmm6, src[2*8] );
    }
    emms();
}
#endif

