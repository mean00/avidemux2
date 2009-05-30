#include <ADM_default.h>
#ifdef HAVE_X86CPU
#include <math.h>
#include <unistd.h>
#include "mjpeg_types.h"
#include "mmx.h"

#define ALIGN_PTR(x,a) ((void *)( (((size_t)(x))+(a)-1)&(-(a))))
#define SHUFFLEMAP(A,B,C,D) ((A)*1+(B)*4+(C)*16+(D)*64)

#define NC_COS6      0.382683432365089771728459984030399//cos(6*pi/16)

#define NC_R_SQRT2   0.707106781186547524400844362104849// 1/sqrt(2)

#define NC_COS0SQRT2 1.0                                //cos(0*pi/16)*sqrt(2)
#define NC_COS1SQRT2 1.38703984532214746182161919156644 //cos(1*pi/16)*sqrt(2)
#define NC_COS2SQRT2 1.30656296487637652785664317342719 //cos(2*pi/16)*sqrt(2)
#define NC_COS3SQRT2 1.17587560241935871697446710461126 //cos(3*pi/16)*sqrt(2)
#define NC_COS4SQRT2 1.0                                //cos(4*pi/16)*sqrt(2)
#define NC_COS5SQRT2 0.785694958387102181277897367657217//cos(5*pi/16)*sqrt(2)
#define NC_COS6SQRT2 0.541196100146196984399723205366389//cos(6*pi/16)*sqrt(2)
#define NC_COS7SQRT2 0.275899379282943012335957563669373//cos(7*pi/16)*sqrt(2)

void init_fdct_sse( void );
void mp2_fdct_sse(int16_t *block);

static float aanscales[64];

#define SSECONST(n,x) static float n[4] __attribute__ ((aligned (16))) = { x, x, x, x }

SSECONST(fdct_sse_r_sqrt2,  NC_R_SQRT2);
SSECONST(fdct_sse_cos6,     NC_COS6);
SSECONST(fdct_sse_cos6sqrt2,NC_COS6SQRT2);
SSECONST(fdct_sse_cos2sqrt2,NC_COS2SQRT2);

void init_mp2_fdct_sse( void )
{
    int i, j;
    static const double aansf[8] = {
        1.0,           // sqrt(2) factor left out here...
        NC_COS1SQRT2,
        NC_COS2SQRT2,
        NC_COS3SQRT2,
        NC_COS4SQRT2,  // cos(4*pi/16) * sqrt(2) = 1.0 exactly
        NC_COS5SQRT2,
        NC_COS6SQRT2,
        NC_COS7SQRT2
    };

    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++)
            aanscales[(i << 3) + j] = 1.0 / (aansf[i] * aansf[j] * 8.0);
}

/*
 * Perform a floating point forward DCT on one block of samples.
 */

// computes A'=A-B
//          B'=A+B
#define ADDDIFF(A,B) { double x=A; A-=B; B+=x; }

#define MMXADDDIFF_Wt(A,B,C) \
        movq_r2r (A,C);      \
        psubw_r2r(B,A);      \
        paddw_r2r(C,B);

#define MMXADDDIFF_Dt(A,B,C) \
        movq_r2r (A,C);      \
        psubd_r2r(B,A);      \
        paddd_r2r(C,B);

#define SSEADDDIFF_t(A,B,C) \
        movaps_r2r(A,C);    \
        subps_r2r(B,A);     \
        addps_r2r(C,B);

#define SSEADDDIFF(x,y)  \
        subps_r2r(y, x); \
        addps_r2r(y, y); \
        addps_r2r(x, y);


#define MMXEXTEND_WD(S,DL,DH) \
        movq_r2r(S, DL);      \
        movq_r2r(S, DH);      \
        psraw_i2r(16, S);     \
        punpcklwd_r2r(S, DL); \
        punpckhwd_r2r(S, DH);

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


#define STOREXMM(x0, offs)               \
        mulps_m2r(aanptr[offs], x0);     \
        cvtps2pi_r2r(x0,  mm0);          \
        movhlps_r2r (x0,  x0);           \
        cvtps2pi_r2r(x0,  mm1);          \
        packssdw_r2r(mm1, mm0);          \
        movq_r2m    (mm0, blkptr[offs]);

#if 0
void fdct_basis(int16_t *block)
{
    double tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    double tmp10, tmp11, tmp12, tmp13;
    double z1, z2, z3, z4, z5, z11, z13;
    double *dataptr;
    double data[64];
    int16_t *blkptr;
    int i;

    // each iteration basically performs the following code:
    blkptr = block;
    dataptr = data;
    for (i = 0; i < 8; i++)
    {
        tmp4 = blkptr[0] + blkptr[7];
        tmp0 = blkptr[0] - blkptr[7];
        tmp5 = blkptr[1] + blkptr[6];
        tmp1 = blkptr[1] - blkptr[6];
        tmp6 = blkptr[2] + blkptr[5];
        tmp2 = blkptr[2] - blkptr[5];
        tmp7 = blkptr[3] + blkptr[4];
        tmp3 = blkptr[3] - blkptr[4];

        /* Even part */

        ADDDIFF(tmp4,tmp7);
        ADDDIFF(tmp5,tmp6);

        ADDDIFF(tmp7,tmp6);

        dataptr[0] = tmp6; /* phase 3 */
        dataptr[4] = tmp7;

        tmp5+=tmp4;

        tmp5 = tmp5 * ((double) NC_R_SQRT2); /* c4 */
        ADDDIFF(tmp4,tmp5);

        dataptr[2] = tmp5;	/* phase 5 */
        dataptr[6] = tmp4;

        /* Odd part */

        tmp3 = tmp3 + tmp2;	/* phase 2 */
        tmp2 = tmp2 + tmp1;
        tmp1 = tmp1 + tmp0;

        /* The rotator is modified from fig 4-8 to avoid extra negations. */
        z5 = (tmp3 - tmp1) * ((double) NC_COS6); /* c6 */
        tmp3 = ((double) NC_COS6SQRT2) * tmp3 + z5; /* c2-c6 */
        tmp1 = ((double) NC_COS2SQRT2) * tmp1 + z5; /* c2+c6 */
        tmp2 = tmp2 * ((double) NC_R_SQRT2); /* c4 */

        ADDDIFF(tmp0,tmp2);

        ADDDIFF(tmp0,tmp3);
        ADDDIFF(tmp2,tmp1);

        dataptr[5] = tmp3;	/* phase 6 */
        dataptr[3] = tmp0;
        dataptr[1] = tmp1;
        dataptr[7] = tmp2;

        dataptr += 8;		/* advance pointer to next row */
        blkptr += 8;
    }
}
#endif

void mp2_fdct_sse(int16_t *block)
{
    float *dataptr,*aanptr;
    float data[67];
    int16_t *blkptr;
    int i;

    /* Pass 1: process rows. */

    blkptr = block;
    dataptr = (float *)ALIGN_PTR(data,16);
    for (i = 0; i < 2; i++)
    {
        // mm0=0a,1a,2a,3a
        // mm1=4a,5a,6a,7a
        // mm2=0b,1b,2b,3b
        // mm3=4b,5b,6b,7b
        movq_m2r(blkptr[16], mm0);
        movq_m2r(blkptr[20], mm1);
        movq_m2r(blkptr[24], mm2);
        movq_m2r(blkptr[28], mm3);

        pshufw_r2ri(mm1, mm1, SHUFFLEMAP(3,2,1,0));
        pshufw_r2ri(mm3, mm3, SHUFFLEMAP(3,2,1,0));

        MMXADDDIFF_Wt(mm0, mm1, mm4);
        MMXADDDIFF_Wt(mm2, mm3, mm5);

        // mm1=4a,4b,5a,5b
        // mm4=7a,7b,6a,6b
        movq_r2r(mm1, mm4);
        punpcklwd_r2r(mm3, mm1);
        punpckhwd_r2r(mm3, mm4);
        pshufw_r2ri(mm4, mm4, SHUFFLEMAP(2,3,0,1));

        MMXADDDIFF_Wt(mm1, mm4, mm3);

        // mm6 = 6a,6b
        // mm7 = 7a,7b
        MMXEXTEND_WD(mm4, mm7, mm6);

        MMXADDDIFF_Dt(mm7, mm6, mm4);

        cvtpi2ps_r2r( mm6, xmm2 );
        cvtpi2ps_r2r( mm7, xmm3 );

        movlps_r2m( xmm2, dataptr[0*8+2] );
        movlps_r2m( xmm3, dataptr[4*8+2] );

        // mm6 = 4a,4b
        // mm7 = 5a,5b
        MMXEXTEND_WD(mm1, mm6, mm7);

        paddd_r2r( mm6, mm7 );

        cvtpi2ps_r2r( mm6, xmm0 );
        cvtpi2ps_r2r( mm7, xmm1 );

        // mm0 = 0a,0b,1a,1b
        // mm1 = 2a,2b,3a,3b
        movq_r2r(mm0, mm1);
        punpcklwd_r2r(mm2, mm0);
        punpckhwd_r2r(mm2, mm1);

        // mm4 = 0a,0b
        // mm5 = 1a,1b
        // mm6 = 2a,2b
        // mm7 = 3a,3b
        MMXEXTEND_WD(mm0, mm4, mm5);
        MMXEXTEND_WD(mm1, mm6, mm7);

        cvtpi2ps_r2r( mm4, xmm4 );
        paddd_r2r( mm6, mm7 );
        cvtpi2ps_r2r( mm7, xmm7 );
        paddd_r2r( mm5, mm6 );
        cvtpi2ps_r2r( mm6, xmm6 );
        paddd_r2r( mm4, mm5 );
        cvtpi2ps_r2r( mm5, xmm5 );

        movlhps_r2r( xmm0, xmm0 );
        movlhps_r2r( xmm1, xmm1 );
        movlhps_r2r( xmm4, xmm4 );
        movlhps_r2r( xmm5, xmm5 );
        movlhps_r2r( xmm6, xmm6 );
        movlhps_r2r( xmm7, xmm7 );




        // mm0=0a,1a,2a,3a
        // mm1=4a,5a,6a,7a
        // mm2=0b,1b,2b,3b
        // mm3=4b,5b,6b,7b
        movq_m2r(blkptr[ 0], mm0);
        movq_m2r(blkptr[ 4], mm1);
        movq_m2r(blkptr[ 8], mm2);
        movq_m2r(blkptr[12], mm3);

        pshufw_r2ri(mm1, mm1, SHUFFLEMAP(3,2,1,0));
        pshufw_r2ri(mm3, mm3, SHUFFLEMAP(3,2,1,0));

        MMXADDDIFF_Wt(mm0, mm1, mm4);
        MMXADDDIFF_Wt(mm2, mm3, mm5);

        // mm1=4a,4b,5a,5b
        // mm4=7a,7b,6a,6b
        movq_r2r(mm1, mm4);
        punpcklwd_r2r(mm3, mm1);
        punpckhwd_r2r(mm3, mm4);
        pshufw_r2ri(mm4, mm4, SHUFFLEMAP(2,3,0,1));

        MMXADDDIFF_Wt(mm1, mm4, mm3);

        // mm6 = 6a,6b
        // mm7 = 7a,7b
        MMXEXTEND_WD(mm4, mm7, mm6);

        MMXADDDIFF_Dt(mm7, mm6, mm4);

        cvtpi2ps_r2r( mm6, xmm2 );
        cvtpi2ps_r2r( mm7, xmm3 );

        movlps_r2m( xmm2, dataptr[0*8+0] );
        movlps_r2m( xmm3, dataptr[4*8+0] );

        // mm6 = 4a,4b
        // mm7 = 5a,5b
        MMXEXTEND_WD(mm1, mm6, mm7);

        paddd_r2r( mm6, mm7 );

        cvtpi2ps_r2r( mm6, xmm0 );
        cvtpi2ps_r2r( mm7, xmm1 );

        // mm0 = 0a,0b,1a,1b
        // mm1 = 2a,2b,3a,3b
        movq_r2r(mm0, mm1);
        punpcklwd_r2r(mm2, mm0);
        punpckhwd_r2r(mm2, mm1);

        // mm4 = 0a,0b
        // mm5 = 1a,1b
        // mm6 = 2a,2b
        // mm7 = 3a,3b
        MMXEXTEND_WD(mm0, mm4, mm5);
        MMXEXTEND_WD(mm1, mm6, mm7);

        cvtpi2ps_r2r( mm4, xmm4 );
        paddd_r2r( mm6, mm7 );
        cvtpi2ps_r2r( mm7, xmm7 );
        paddd_r2r( mm5, mm6 );
        cvtpi2ps_r2r( mm6, xmm6 );
        paddd_r2r( mm4, mm5 );
        cvtpi2ps_r2r( mm5, xmm5 );





        // z1 = (tmp11 + tmp10) * ((double) NC_R_SQRT2); /* c4 */
        // dataptr[2] = tmp10 + z1;	/* phase 5 */
        // dataptr[6] = tmp10 - z1;

        mulps_m2r(fdct_sse_r_sqrt2[0],xmm1);
        SSEADDDIFF_t(xmm0, xmm1, xmm2);
        movaps_r2m( xmm0, dataptr[6*8] );
        movaps_r2m( xmm1, dataptr[2*8] );
        

        /* The rotator is modified from fig 4-8 to avoid extra negations. */
        // z5 = (tmp7 - tmp5) * ((double) NC_COS6); /* c6 */
        movaps_r2r( xmm7, xmm2 );
        subps_r2r( xmm5, xmm2 );
        mulps_m2r(fdct_sse_cos6[0], xmm2 );

        // tmp7 = ((double) NC_COS6SQRT2) * tmp7 + z5; /* c2-c6 */
        // tmp5 = ((double) NC_COS2SQRT2) * tmp5 + z5; /* c2+c6 */
        // tmp6 = tmp6 * ((double) NC_R_SQRT2); /* c4 */
        mulps_m2r(fdct_sse_cos6sqrt2[0], xmm7);
        mulps_m2r(fdct_sse_cos2sqrt2[0], xmm5);
        mulps_m2r(fdct_sse_r_sqrt2[0], xmm6 );
        addps_r2r(xmm2, xmm7);
        addps_r2r(xmm2, xmm5);

        // z16 = tmp4 + tmp6;		/* phase 5 */
        // z14 = tmp4 - tmp6;
        SSEADDDIFF_t(xmm4, xmm6, xmm2);

        SSEADDDIFF_t(xmm4, xmm7, xmm2);
        SSEADDDIFF_t(xmm6, xmm5, xmm2);
        
        // dataptr[5] = z14 + tmp7;	/* phase 6 */
        // dataptr[3] = z14 - tmp7;
        // dataptr[1] = z16 + tmp5;
        // dataptr[7] = z16 - tmp5;

        movaps_r2m( xmm7, dataptr[5*8] );
        movaps_r2m( xmm4, dataptr[3*8] );
        movaps_r2m( xmm5, dataptr[1*8] );
        movaps_r2m( xmm6, dataptr[7*8] );

        dataptr += 4;		/* advance pointer to next row */
        blkptr += 8*4;
    }

    /* Pass 2: process columns. */

    dataptr = (float *)ALIGN_PTR(data,16);
    aanptr  = aanscales;
    blkptr = block;
    for (i = 0; i < 2; i++)
    {
        LOADROTATEff(dataptr,   xmm4, xmm0, xmm1, xmm2, xmm3);
        movaps_r2m(xmm0, dataptr[0]);
        LOADROTATEff(dataptr+4, xmm0, xmm7, xmm6, xmm5, xmm4);
        movaps_m2r(dataptr[0], xmm0);

        SSEADDDIFF(xmm0,xmm4);
        SSEADDDIFF(xmm1,xmm5);
        SSEADDDIFF(xmm2,xmm6);
        SSEADDDIFF(xmm3,xmm7);

        /* Even part */

        SSEADDDIFF(xmm4, xmm7);
        SSEADDDIFF(xmm5, xmm6);

        SSEADDDIFF(xmm7, xmm6);

        STOREXMM(xmm6,0);
        STOREXMM(xmm7,32);

        addps_r2r(xmm4, xmm5);
        mulps_m2r(fdct_sse_r_sqrt2[0],xmm5);
        
        SSEADDDIFF_t(xmm4, xmm5, xmm6);

        STOREXMM(xmm5,16);
        STOREXMM(xmm4,48);

        /* Odd part */

        addps_r2r(xmm2, xmm3);
        addps_r2r(xmm1, xmm2);
        addps_r2r(xmm0, xmm1);

        /* The rotator is modified from fig 4-8 to avoid extra negations. */
        movaps_r2r(xmm3, xmm4);
        subps_r2r(xmm1, xmm4);
        mulps_m2r(fdct_sse_cos6[0], xmm4);
        mulps_m2r(fdct_sse_cos6sqrt2[0], xmm3);
        mulps_m2r(fdct_sse_cos2sqrt2[0], xmm1);
        mulps_m2r(fdct_sse_r_sqrt2[0], xmm2);
        addps_r2r(xmm4, xmm3);
        addps_r2r(xmm4, xmm1);

        SSEADDDIFF_t(xmm0, xmm2, xmm5);

        SSEADDDIFF_t(xmm0, xmm3, xmm6);
        STOREXMM(xmm0,24);
        STOREXMM(xmm3,40);

        SSEADDDIFF_t(xmm2, xmm1, xmm7);
        STOREXMM(xmm2,56);
        STOREXMM(xmm1, 8);

        dataptr+=4*8;			/* advance pointer to next column */
        aanptr+=4;
        blkptr+=4;
    }
    emms();
}
#endif
