/*
;;; 
;;;  mblock_sad_mmxe.s:  
;;; 
;;; Enhanced MMX optimized Sum Absolute Differences routines for macroblocks
;;; (interpolated, 1-pel, 2*2 sub-sampled pel and 4*4 sub-sampled pel)
;
;  Original MMX sad_* Copyright (C) 2000 Chris Atenasio <chris@crud.net>
;  Enhanced MMX and rest Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>

		;; Yes, I tried prefetch-ing.  It makes no difference or makes
		;; stuff *slower*.

;
;  This program is free software; you can reaxstribute it and/or
;  modify it under the terms of the GNU General Public License
;  as published by the Free Software Foundation; either version 2
;  of the License, or (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; if not, write to the Free Software
;  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
;
;
;
*/

#include <config.h>
#ifdef HAVE_X86CPU
#include "mjpeg_types.h"
#include "mmx.h"
#include "mmxsse_motion.h"

#define SHUFFLEMAP(A,B,C,D) ((A)*1+(B)*4+(C)*16+(D)*64)

int sad_00_mmxe(uint8_t *blk1,uint8_t *blk2,int rowstride,int h,int distlim)
{
    int32_t rv;

    pxor_r2r(mm0,mm0); // accumulator

    do {
        movq_m2r  (blk1[0],mm4);
        psadbw_m2r(blk2[0],mm4);
        movq_m2r  (blk1[8],mm6);
        psadbw_m2r(blk2[8],mm6);
        blk1+=rowstride;
        paddd_r2r (mm4, mm0);
        blk2+=rowstride;
        paddd_r2r (mm6, mm0);
        
        movq_m2r  (blk1[0],mm4);
        psadbw_m2r(blk2[0],mm4);
        movq_m2r  (blk1[8],mm6);
        psadbw_m2r(blk2[8],mm6);
        blk1+=rowstride;
        paddd_r2r (mm4, mm0);
        blk2+=rowstride;
        paddd_r2r (mm6, mm0);
        
        h-=2;
    } while(h);
    movd_r2g(mm0, rv);
    emms();
    return rv;
}

int sad_01_mmxe(uint8_t *blk1,uint8_t *blk2,int rowstride,int h)
{
    int32_t rv;

    pxor_r2r(mm0,mm0); // accumulator

    do {
        movq_m2r  (blk1[0],mm4);
        pavgb_m2r (blk1[1],mm4);
        psadbw_m2r(blk2[0],mm4);
        movq_m2r  (blk1[8],mm6);
        pavgb_m2r (blk1[9],mm6);
        psadbw_m2r(blk2[8],mm6);
        blk1+=rowstride;
        paddd_r2r (mm4, mm0);
        blk2+=rowstride;
        paddd_r2r (mm6, mm0);
        
        movq_m2r  (blk1[0],mm4);
        pavgb_m2r (blk1[1],mm4);
        psadbw_m2r(blk2[0],mm4);
        movq_m2r  (blk1[8],mm6);
        pavgb_m2r (blk1[9],mm6);
        psadbw_m2r(blk2[8],mm6);
        blk1+=rowstride;
        paddd_r2r (mm4, mm0);
        blk2+=rowstride;
        paddd_r2r (mm6, mm0);
        
        h-=2;
    } while(h);
    movd_r2g(mm0, rv);
    emms();
    return rv;
}

int sad_10_mmxe(uint8_t *blk1,uint8_t *blk2,int rowstride,int h)
{
    int32_t rv;

    pxor_r2r(mm0,mm0); // accumulator
    movq_m2r(blk1[0],mm4);
    movq_m2r(blk1[8],mm6);
    blk1+=rowstride;

    do {
        movq_m2r  (blk1[0],mm5);
        pavgb_r2r (mm5,mm4);
        psadbw_m2r(blk2[0],mm4);
        movq_m2r  (blk1[8],mm7);
        pavgb_r2r (mm7,mm6);
        psadbw_m2r(blk2[8],mm6);
        blk1+=rowstride;
        paddd_r2r (mm4, mm0);
        blk2+=rowstride;
        paddd_r2r (mm6, mm0);
        
        movq_m2r  (blk1[0],mm4);
        pavgb_r2r (mm4,mm5);
        psadbw_m2r(blk2[0],mm5);
        movq_m2r  (blk1[8],mm6);
        pavgb_r2r (mm6,mm7);
        psadbw_m2r(blk2[8],mm7);
        blk1+=rowstride;
        paddd_r2r (mm5, mm0);
        blk2+=rowstride;
        paddd_r2r (mm7, mm0);
        
        h-=2;
    } while(h);
    movd_r2g(mm0, rv);
    emms();
    return rv;
}

int sad_11_mmxe(uint8_t *blk1,uint8_t *blk2,int rowstride,int h)
{
    int l;
    int32_t rv;
    uint8_t *b1,*b2;
    static int16_t two[4]={2,2,2,2};

#define SAD11_UNPACK(src,r1,r2) \
        movq_m2r(src,r1);       \
        movq_r2r(r1,r2);        \
        punpcklbw_r2r(mm1,r1);  \
        punpckhbw_r2r(mm1,r2);

    pxor_r2r(mm0,mm0); // accumulator
    pxor_r2r(mm1,mm1); // zero value

    b1=blk1;
    b2=blk2;
    l=h;
    
    SAD11_UNPACK(b1[0],mm6,mm7);
    SAD11_UNPACK(b1[1],mm2,mm3);
    paddw_r2r(mm2,mm6);
    paddw_r2r(mm3,mm7);
    b1+=rowstride;

    do {
        SAD11_UNPACK(b1[0],mm2,mm3);
        SAD11_UNPACK(b1[1],mm4,mm5);

        paddw_m2r(*two,mm6);
        paddw_m2r(*two,mm7);

        paddw_r2r(mm4,mm2);
        paddw_r2r(mm5,mm3);

        paddw_r2r(mm2,mm6);
        paddw_r2r(mm3,mm7);

        psrlw_i2r(2,mm6);
        psrlw_i2r(2,mm7);

        packuswb_r2r(mm7,mm6);
        psadbw_m2r(b2[0],mm6);
        paddd_r2r(mm6,mm0);

        b1+=rowstride;
        b2+=rowstride;
       
        movq_r2r(mm2,mm6);
        movq_r2r(mm3,mm7);

        l--;
    } while(l);

    b1=blk1+8;
    b2=blk2+8;
    l=h;
    
    SAD11_UNPACK(b1[0],mm6,mm7);
    SAD11_UNPACK(b1[1],mm2,mm3);
    paddw_r2r(mm2,mm6);
    paddw_r2r(mm3,mm7);
    b1+=rowstride;

    do {
        SAD11_UNPACK(b1[0],mm2,mm3);
        SAD11_UNPACK(b1[1],mm4,mm5);

        paddw_m2r(*two,mm6);
        paddw_m2r(*two,mm7);

        paddw_r2r(mm4,mm2);
        paddw_r2r(mm5,mm3);

        paddw_r2r(mm2,mm6);
        paddw_r2r(mm3,mm7);

        psrlw_i2r(2,mm6);
        psrlw_i2r(2,mm7);

        packuswb_r2r(mm7,mm6);
        psadbw_m2r(b2[0],mm6);
        paddd_r2r(mm6,mm0);

        b1+=rowstride;
        b2+=rowstride;
       
        movq_r2r(mm2,mm6);
        movq_r2r(mm3,mm7);
       
        l--;
    } while(l);

    movd_r2g(mm0, rv);
    emms();
    return rv;
#undef SAD11_UNPACK
}

int sad_sub22_mmxe(uint8_t *blk1,uint8_t *blk2,int rowstride,int h)
{
    int32_t rv;

    pxor_r2r(mm0,mm0); // accumulator

    do {
        movq_m2r  (blk1[0],mm4);
        blk1+=rowstride;
        psadbw_m2r(blk2[0],mm4);
        blk2+=rowstride;
        paddd_r2r (mm4, mm0);
        
        movq_m2r  (blk1[0],mm4);
        blk1+=rowstride;
        psadbw_m2r(blk2[0],mm4);
        blk2+=rowstride;
        paddd_r2r (mm4, mm0);
        
        h-=2;
    } while(h);
    movd_r2g(mm0, rv);
    emms();
    return rv;
}

int sad_sub44_mmxe(uint8_t *blk1,uint8_t *blk2,int rowstride,int h)
{
    int32_t rv;

    pxor_r2r(mm0,mm0); // accumulator

    do {
        // only perform one at a time; h might be 1
        movd_m2r  (blk1[0],mm4);
        movd_m2r  (blk2[0],mm5);
        psadbw_r2r(mm5,mm4);
        blk1+=rowstride;
        blk2+=rowstride;
        paddd_r2r (mm4, mm0);
                
        h-=1;
    } while(h);
    movd_r2g(mm0, rv);
    emms();
    return rv;
}

/*
;;; 
;;;  mblock_*nearest4_sad_mmxe.s:  
;;; 
;;; Enhanced MMX optimized Sum Absolute Differences routines for
;;; quads macroblocks offset by (0,0) (0,1) (1,0) (1,1) pel
;;; 

;;; Explanation: the motion compensation search at 1-pel and 2*2 sub-sampled
;;; evaluates macroblock quads.  A lot of memory accesses can be saved
;;; if each quad is done together rather than each macroblock in the
;;; quad handled individually.

;;; TODO:		Really there ought to be MMX versions and the function's
;;; specification should be documented...
;
; Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>	
*/

/*
; mm0 = SAD (x+0,y+0),SAD (x+2,y+0)
; mm1 = SAD (x+0,y+2),SAD (x+2,y+2)

; mm2 = blk1[0]   cache
; mm3 = blk1[8] cache
; mm4 = blk1[1] cache
; mm5 = blk1[9] cache
; mm6 = blk2[0]   cache
; mm7 = blk2[8] cache			
*/
int mblock_nearest4_sads_mmxe(uint8_t *blk1,uint8_t *blk2,int lx,int h,int32_t *weightvec,int peakerror)
{
    int32_t rv;

    movq_m2r(blk1[0],mm2);
    movq_m2r(blk1[8],mm3);
    movq_m2r(blk1[1],mm4);
    movq_m2r(blk1[9],mm5);
    pxor_r2r(mm0,mm0); // zero accumulators
    pxor_r2r(mm1,mm1);

    do {
        /* Yes, the idea of using packssdw is kinda silly.  But it efficiently moves
           the result into place, reducing the number of instructions.  Instead of:

           mm0 += mm2;
           mm4 <<= 32;
           mm0 += mm4;

           we get:

           mm2 = pack(mm4,mm2);
           mm0 += mm2;
           
           Yes, this makes a difference (though admittedly slight).
        */

        movq_m2r(blk2[0],mm6);
        psadbw_r2r(mm6,mm2);
        psadbw_r2r(mm6,mm4);
        packssdw_r2r(mm4,mm2);
        paddd_r2r(mm2,mm0);

        movq_m2r(blk2[8],mm7);
        psadbw_r2r(mm7,mm3);
        psadbw_r2r(mm7,mm5);
        packssdw_r2r(mm5,mm3);
        paddd_r2r(mm3,mm0);

        // advance pointers to next row
        blk1+=lx;
        blk2+=lx;

        // check for early exit
        movq_r2r(mm1, mm3);
        pminsw_r2r(mm0,mm3);
        pshufw_r2ri(mm3,mm2,SHUFFLEMAP(2,3,0,1));
        pminsw_r2r(mm2,mm3);
        movd_r2g(mm3,rv);
        if( rv > peakerror )
            goto safeexit;

        movq_m2r(blk1[0],mm2);
        movq_m2r(blk1[1],mm4);
        movq_r2r(mm6,mm3);
        psadbw_r2r(mm2,mm6);
        psadbw_r2r(mm4,mm3);
        packssdw_r2r(mm3,mm6);
        paddd_r2r(mm6,mm1);

        movq_m2r(blk1[8],mm3);
        movq_m2r(blk1[9],mm5);
        movq_r2r(mm7,mm6);
        psadbw_r2r(mm3,mm7);
        psadbw_r2r(mm5,mm6);
        packssdw_r2r(mm6,mm7);
        paddd_r2r(mm7,mm1);

        h--;
    } while(h);

    movq_r2m(mm0,weightvec[0]);
    movq_r2m(mm1,weightvec[2]);

    pminsw_r2r(mm1,mm0);
    pshufw_r2ri(mm0,mm1,SHUFFLEMAP(2,3,0,1));
    pminsw_r2r(mm1,mm0);
    movd_r2g(mm0,rv);
 safeexit:
    emms();
    return rv;
}

void mblock_sub22_nearest4_sads_mmxe(uint8_t *blk1, uint8_t *blk2,
                                     int rowstride, int h, int32_t *resvec)
{
    pxor_r2r(mm0, mm0);
    pxor_r2r(mm1, mm1);

    movq_m2r(blk1[0], mm2);
    movq_m2r(blk1[1], mm3);
    do {
        movq_m2r(blk2[0], mm5);

        psadbw_r2r(mm5, mm2);
        psadbw_r2r(mm5, mm3);
        packssdw_r2r(mm3,mm2);
        paddd_r2r(mm2,mm0);

        blk1+=rowstride;
        blk2+=rowstride;
        movq_m2r(blk1[0], mm2);
        movq_m2r(blk1[1], mm3);

        movq_r2r(mm5, mm6);
        psadbw_r2r(mm2, mm5);
        psadbw_r2r(mm3, mm6);
        packssdw_r2r(mm6,mm5);
        paddd_r2r(mm5,mm1);

        h--;
    } while(h);
    movq_r2m(mm0,resvec[0]);
    movq_r2m(mm1,resvec[2]);
    emms();
}
#endif
