/* idct.c, this file is part of the
 * AltiVec optimized library for MJPEG tools MPEG-1/2 Video Encoder
 * Copyright (C) 2003  James Klicman <james@klicman.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "altivec_conf.h"
#include "vectorize.h"
#include "../mjpeg_logging.h"

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif


#define W1        1.38703989982604980468750000 /* sqrt(2)*cos(1*PI/16) */
#define W2        1.30656301975250244140625000 /* sqrt(2)*cos(2*PI/16) */
#define W3        1.17587554454803466796875000 /* sqrt(2)*cos(3*PI/16) */
#define W5        0.78569495677947998046875000 /* sqrt(2)*cos(5*PI/16) */
#define W6        0.54119610786437988281250000 /* sqrt(2)*cos(6*PI/16) */
#define W7        0.27589938044548034667968750 /* sqrt(2)*cos(7*PI/16) */
#define SQRT_0_5  0.70710676908493041992187500 /* sqrt(0.5) */
#define DIVBY8    0.125                        /* 1/8 */

static vector float idctconsts[3] = {
    (vector float)VCONST(       W7,    W1-W7,    W1+W7,       W3 ),
    (vector float)VCONST(    W3-W5,    W3+W5,       W6,    W2+W6 ),
    (vector float)VCONST(    W2-W6, SQRT_0_5,   DIVBY8,        0 )
};

#define LD_W7       vec_splat(cnsts0, 0)
#define LD_W1mW7    vec_splat(cnsts0, 1)
#define LD_W1pW7    vec_splat(cnsts0, 2)
#define LD_W3       vec_splat(cnsts0, 3)
#define LD_W3mW5    vec_splat(cnsts1, 0)
#define LD_W3pW5    vec_splat(cnsts1, 1)
#define LD_W6       vec_splat(cnsts1, 2)
#define LD_W2pW6    vec_splat(cnsts1, 3)
#define LD_W2mW6    vec_splat(cnsts2, 0)
#define LD_SQRT_0_5 vec_splat(cnsts2, 1)
#define LD_DIVBY8   vec_splat(cnsts2, 2)



#define IDCTROW(b0,b1,b2,b3,b4,b5,b6,b7) /* {{{ */                   \
    x0 = b0;                                                         \
    x1 = b4;                                                         \
    x2 = b6;                                                         \
    x3 = b2;                                                         \
    x4 = b1;                                                         \
    x5 = b7;                                                         \
    x6 = b5;                                                         \
    x7 = b3;                                                         \
                                                                     \
    /* first stage */                                                \
    cnst = LD_W7;                                                    \
    x8 = vec_add(x4, x5);                                            \
    x8 = vec_madd(cnst, x8, mzero);     /* x8 = W7*(x4+x5); */       \
    cnst = LD_W1mW7;                                                 \
    x4 = vec_madd(cnst, x4, x8);        /* x4 = x8 + (W1-W7)*x4; */  \
    cnst = LD_W1pW7;                                                 \
    x5 = vec_nmsub(cnst, x5, x8);       /* x5 = x8 - (W1+W7)*x5; */  \
    cnst = LD_W3;                                                    \
    x8 = vec_add(x6, x7);                                            \
    x8 = vec_madd(cnst, x8, mzero);     /* x8 = W3*(x6+x7); */       \
    cnst = LD_W3mW5;                                                 \
    x6 = vec_nmsub(cnst, x6, x8);       /* x6 = x8 - (W3-W5)*x6; */  \
    cnst = LD_W3pW5;                                                 \
    x7 = vec_nmsub(cnst, x7, x8);       /* x7 = x8 - (W3+W5)*x7; */  \
                                                                     \
    /* second stage */                                               \
    x8 = vec_add(x0, x1);               /* x8 = x0 + x1; */          \
    x0 = vec_sub(x0, x1);               /* x0 -= x1; */              \
    cnst = LD_W6;                                                    \
    x1 = vec_add(x3, x2);                                            \
    x1 = vec_madd(cnst, x1, mzero);     /* x1 = W6*(x3+x2); */       \
    cnst = LD_W2pW6;                                                 \
    x2 = vec_nmsub(cnst, x2, x1);       /* x2 = x1 - (W2+W6)*x2; */  \
    cnst = LD_W2mW6;                                                 \
    x3 = vec_madd(cnst, x3, x1);        /* x3 = x1 + (W2-W6)*x3; */  \
    x1 = vec_add(x4, x6);               /* x1 = x4 + x6; */          \
    x4 = vec_sub(x4, x6);               /* x4 -= x6; */              \
    x6 = vec_add(x5, x7);               /* x6 = x5 + x7; */          \
    x5 = vec_sub(x5, x7);               /* x5 -= x7; */              \
                                                                     \
    /* third stage */                                                \
    x7 = vec_add(x8, x3);               /* x7 = x8 + x3; */          \
    x8 = vec_sub(x8, x3);               /* x8 -= x3; */              \
    x3 = vec_add(x0, x2);               /* x3 = x0 + x2; */          \
    x0 = vec_sub(x0, x2);               /* x0 -= x2; */              \
    cnst = LD_SQRT_0_5;                                              \
    x2 = vec_add(x4, x5);                                            \
    x2 = vec_madd(cnst, x2, mzero);     /* x2 = SQRT_0_5*(x4+x5); */ \
    x4 = vec_sub(x4, x5);                                            \
    x4 = vec_madd(cnst, x4, mzero);     /* x4 = SQRT_0_5*(x4-x5); */ \
                                                                     \
    /* fourth stage */                                               \
    b0 = vec_add(x7, x1);               /* x7+x1 */                  \
    b1 = vec_add(x3, x2);               /* x3+x2 */                  \
    b2 = vec_add(x0, x4);               /* x0+x4 */                  \
    b3 = vec_add(x8, x6);               /* x8+x6 */                  \
    b4 = vec_sub(x8, x6);               /* x8-x6 */                  \
    b5 = vec_sub(x0, x4);               /* x0-x4 */                  \
    b6 = vec_sub(x3, x2);               /* x3-x2 */                  \
    b7 = vec_sub(x7, x1);               /* x7-x1 */                  \
    /* }}} */


#define IDCTCOL(b0,b1,b2,b3,b4,b5,b6,b7) /* {{{ */                   \
    x0 = b0;                                                         \
    x1 = b4;                                                         \
    x2 = b6;                                                         \
    x3 = b2;                                                         \
    x4 = b1;                                                         \
    x5 = b7;                                                         \
    x6 = b5;                                                         \
    x7 = b3;                                                         \
                                                                     \
    /* first stage */                                                \
    cnst = LD_W7;                                                    \
    x8 = vec_add(x4, x5);                                            \
    x8 = vec_madd(cnst, x8, mzero);     /* x8 = W7*(x4+x5); */       \
    cnst = LD_W1mW7;                                                 \
    x4 = vec_madd(cnst, x4, x8);        /* x4 = (x8+(W1-W7)*x4); */  \
    cnst = LD_W1pW7;                                                 \
    x5 = vec_nmsub(cnst, x5, x8);       /* x5 = (x8-(W1+W7)*x5); */  \
    cnst = LD_W3;                                                    \
    x8 = vec_add(x6, x7);                                            \
    x8 = vec_madd(cnst, x8, mzero);     /* x8 = W3*(x6+x7); */       \
    cnst = LD_W3mW5;                                                 \
    x6 = vec_nmsub(cnst, x6, x8);       /* x6 = (x8-(W3-W5)*x6); */  \
    cnst = LD_W3pW5;                                                 \
    x7 = vec_nmsub(cnst, x7, x8);       /* x7 = (x8-(W3+W5)*x7); */  \
                                                                     \
    /* second stage */                                               \
    cnst = LD_DIVBY8;                                                \
    x8 = vec_add(x0, x1);                                            \
    x8 = vec_madd(x8, cnst, mzero);     /* x8 = (x0 + x1); */        \
    x0 = vec_sub(x0, x1);                                            \
    x0 = vec_madd(x0, cnst, mzero);     /* x0 = (x0 - x1); */        \
    cnst = LD_W6;                                                    \
    x1 = vec_add(x3, x2);                                            \
    x1 = vec_madd(cnst, x1, mzero);     /* x1 = W6*(x3+x2); */       \
    cnst = LD_W2pW6;                                                 \
    x2 = vec_nmsub(cnst, x2, x1);       /* x2 = (x1-(W2+W6)*x2); */  \
    cnst = LD_W2mW6;                                                 \
    x3 = vec_madd(cnst, x3, x1);        /* x3 = (x1+(W2-W6)*x3); */  \
    x1 = vec_add(x4, x6);               /* x1 = x4 + x6; */          \
    x4 = vec_sub(x4, x6);               /* x4 -= x6; */              \
    x6 = vec_add(x5, x7);               /* x6 = x5 + x7; */          \
    x5 = vec_sub(x5, x7);               /* x5 -= x7; */              \
                                                                     \
    /* third stage */                                                \
    x7 = vec_add(x8, x3);               /* x7 = x8 + x3; */          \
    x8 = vec_sub(x8, x3);               /* x8 -= x3; */              \
    x3 = vec_add(x0, x2);               /* x3 = x0 + x2; */          \
    x0 = vec_sub(x0, x2);               /* x0 -= x2; */              \
    cnst = LD_SQRT_0_5;                                              \
    x2 = vec_add(x4, x5);                                            \
    x2 = vec_madd(cnst, x2, mzero);     /* x2 = SQRT_0_5*(x4+x5); */ \
    x4 = vec_sub(x4, x5);                                            \
    x4 = vec_madd(cnst, x4, mzero);     /* x4 = SQRT_0_5*(x4-x5); */ \
                                                                     \
    /* fourth stage */                                               \
    b0 = vec_add(x7, x1); /* x7+x1 */                                \
    b1 = vec_add(x3, x2); /* x3+x2 */                                \
    b2 = vec_add(x0, x4); /* x0+x4 */                                \
    b3 = vec_add(x8, x6); /* x8+x6 */                                \
    b4 = vec_sub(x8, x6); /* x8-x6 */                                \
    b5 = vec_sub(x0, x4); /* x0-x4 */                                \
    b6 = vec_sub(x3, x2); /* x3-x2 */                                \
    b7 = vec_sub(x7, x1); /* x7-x1 */                                \
    /* }}} */


#define IDCT_PDECL short *block
#define IDCT_ARGS block
#define IDCT_PFMT "block=0x%X"


/* two dimensional inverse discrete cosine transform */

void idct_altivec(IDCT_PDECL)
{
    vector signed short *bp;
    vector float *cp;
    vector float b00, b10, b20, b30, b40, b50, b60, b70;
    vector float b01, b11, b21, b31, b41, b51, b61, b71;
    vector float mzero, cnst, cnsts0, cnsts1, cnsts2;
    vector float x0, x1, x2, x3, x4, x5, x6, x7, x8;

#ifdef ALTIVEC_VERIFY
    if (NOT_VECTOR_ALIGNED(block))
	mjpeg_error_exit1("idct: block %% 16 != 0, (%d)\n", block);
#endif

    AMBER_START;


    /* 8x8 matrix transpose (vector short[8]) {{{ */

#define MERGE_S16(hl,a,b) vec_merge##hl(vs16(a), vs16(b))

    bp = (vector signed short*)block;
    x0 = vfp(vec_ld(0,    bp));
    x4 = vfp(vec_ld(16*4, bp));
    b00 = vfp(MERGE_S16(h, x0, x4));
    b10 = vfp(MERGE_S16(l, x0, x4));
    bp++;
    x1 = vfp(vec_ld(0,    bp));
    x5 = vfp(vec_ld(16*4, bp));
    b20 = vfp(MERGE_S16(h, x1, x5));
    b30 = vfp(MERGE_S16(l, x1, x5));
    bp++;
    x2 = vfp(vec_ld(0,    bp));
    x6 = vfp(vec_ld(16*4, bp));
    b40 = vfp(MERGE_S16(h, x2, x6));
    b50 = vfp(MERGE_S16(l, x2, x6));
    bp++;
    x3 = vfp(vec_ld(0,    bp));
    x7 = vfp(vec_ld(16*4, bp));
    b60 = vfp(MERGE_S16(h, x3, x7));
    b70 = vfp(MERGE_S16(l, x3, x7));

    b01 = vfp(MERGE_S16(h, b00, b40));
    b11 = vfp(MERGE_S16(l, b00, b40));
    b21 = vfp(MERGE_S16(h, b10, b50));
    b31 = vfp(MERGE_S16(l, b10, b50));
    b41 = vfp(MERGE_S16(h, b20, b60));
    b51 = vfp(MERGE_S16(l, b20, b60));
    b61 = vfp(MERGE_S16(h, b30, b70));
    b71 = vfp(MERGE_S16(l, b30, b70));

    x0 = vfp(MERGE_S16(h, b01, b41));
    x1 = vfp(MERGE_S16(l, b01, b41));
    x2 = vfp(MERGE_S16(h, b11, b51));
    x3 = vfp(MERGE_S16(l, b11, b51));
    x4 = vfp(MERGE_S16(h, b21, b61));
    x5 = vfp(MERGE_S16(l, b21, b61));
    x6 = vfp(MERGE_S16(h, b31, b71));
    x7 = vfp(MERGE_S16(l, b31, b71));

#undef MERGE_S16
    /* }}} */


    /* convert to float {{{ */
#define CTF(n) \
    b##n##0 = vfp(vec_unpackh(vs16(x##n))); \
    b##n##1 = vfp(vec_unpackl(vs16(x##n))); \
    b##n##0 = vfp(vec_ctf(vs32(b##n##0), 0)); \
    b##n##1 = vfp(vec_ctf(vs32(b##n##1), 0)); \

    CTF(0);
    CTF(1);
    CTF(2);
    CTF(3);
    CTF(4);
    CTF(5);
    CTF(6);
    CTF(7);

#undef CTF
    /* }}} */


    /* setup constants {{{ */
    /* mzero = -0.0 */
    mzero = (vector float)vec_splat_u32(-1);
    mzero = (vector float)vec_sl(vu32(mzero), vu32(mzero));
    cp = idctconsts;
    cnsts0 = vec_ld(0, cp); cp++;
    cnsts1 = vec_ld(0, cp); cp++;
    cnsts2 = vec_ld(0, cp);
    /* }}} */


    IDCTROW(b00, b10, b20, b30, b40, b50, b60, b70);
    IDCTROW(b01, b11, b21, b31, b41, b51, b61, b71);


    /* 8x8 matrix transpose (vector float[8][2]) {{{ */
    x0 = vec_mergel(b00, b20);
    x1 = vec_mergeh(b00, b20);
    x2 = vec_mergel(b10, b30);
    x3 = vec_mergeh(b10, b30);

    b00 = vec_mergeh(x1, x3);
    b10 = vec_mergel(x1, x3);
    b20 = vec_mergeh(x0, x2);
    b30 = vec_mergel(x0, x2);

    x4 = vec_mergel(b41, b61);
    x5 = vec_mergeh(b41, b61);
    x6 = vec_mergel(b51, b71);
    x7 = vec_mergeh(b51, b71);

    b41 = vec_mergeh(x5, x7);
    b51 = vec_mergel(x5, x7);
    b61 = vec_mergeh(x4, x6);
    b71 = vec_mergel(x4, x6);

    x0 = vec_mergel(b01, b21);
    x1 = vec_mergeh(b01, b21);
    x2 = vec_mergel(b11, b31);
    x3 = vec_mergeh(b11, b31);

    x4 = vec_mergel(b40, b60);
    x5 = vec_mergeh(b40, b60);
    x6 = vec_mergel(b50, b70);
    x7 = vec_mergeh(b50, b70);

    b40 = vec_mergeh(x1, x3);
    b50 = vec_mergel(x1, x3);
    b60 = vec_mergeh(x0, x2);
    b70 = vec_mergel(x0, x2);

    b01 = vec_mergeh(x5, x7);
    b11 = vec_mergel(x5, x7);
    b21 = vec_mergeh(x4, x6);
    b31 = vec_mergel(x4, x6);
    /* }}} */


    /* divide constants by 8 {{{ */
    cnst = LD_DIVBY8;
    cnsts0 = vec_madd(cnsts0, cnst, mzero);
    cnsts1 = vec_madd(cnsts1, cnst, mzero);
    /* cnts2 = (cnsts2[0]*DIVBY8, cnsts2[1], cnsts2[2], cnsts[3]) */
    x0 = vec_sld(cnsts2, cnsts2, 4);
    x1 = vec_madd(x0, cnst, mzero);
    cnsts2 = vec_sld(x1, x0, 12);
    /* }}} */


    IDCTCOL(b00, b10, b20, b30, b40, b50, b60, b70);
    IDCTCOL(b01, b11, b21, b31, b41, b51, b61, b71);


    /* round, convert back to short and clip {{{ */

    /* cnsts0 = max = 255 = 0x00ff, cnsts2 = min = -256 = 0xff00 {{{ */
    cnsts0 = vfp(vec_splat_u8(0));
    x8 = vfp(vec_splat_u8(-1));
    cnsts2 = vfp(vec_mergeh(vu8(x8), vu8(cnsts0)));
    cnsts0 = vfp(vec_mergeh(vu8(cnsts0), vu8(x8)));
    /* }}} */

#define CTS(n) \
    b##n##0 = vfp(vec_round(b##n##0)); \
    b##n##1 = vfp(vec_round(b##n##1)); \
    b##n##0 = vfp(vec_cts(b##n##0, 0)); \
    b##n##1 = vfp(vec_cts(b##n##1, 0)); \
    b##n##0 = vfp(vec_pack(vs32(b##n##0), vs32(b##n##1))); \
    b##n##0 = vfp(vec_min(vs16(b##n##0), vs16(cnsts0))); \
    b##n##0 = vfp(vec_max(vs16(b##n##0), vs16(cnsts2))); \
    vec_st(vs16(b##n##0), 0, bp);


    bp = (vector signed short*)block;
    CTS(0); bp++;
    CTS(1); bp++;
    CTS(2); bp++;
    CTS(3); bp++;
    CTS(4); bp++;
    CTS(5); bp++;
    CTS(6); bp++;
    CTS(7);

#undef CTS
    /* }}} */


    AMBER_STOP;
}


#if ALTIVEC_TEST_FUNCTION(idct) /* {{{ */
#ifdef ALTIVEC_VERIFY
void idct_altivec_verify(IDCT_PDECL)
{
    int i;

    idct_altivec(IDCT_ARGS);

    for (i = 0; i < 64; i++) {
	if (block[i] < -256)
	    mjpeg_warn("idct: block[%d]=%d < -256\n", i, block[i]);
	else if (block[i] > 255)
	    mjpeg_warn("idct: block[%d]=%d > 255\n", i, block[i]);
    }
}
#else
ALTIVEC_TEST(idct, void, (IDCT_PDECL), IDCT_PFMT, IDCT_ARGS);
#endif
#endif /* }}} */

/* vim:set foldmethod=marker foldlevel=0: */
