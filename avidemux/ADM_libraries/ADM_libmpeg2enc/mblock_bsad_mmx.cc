/*
 *   bdist1_mmx.s:  mmX optimized bidirectional absolute distance sum
 * 
 *   Original believed to be Copyright (C) 2000 Brent Byeler
 * 
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <ADM_default.h>
#include <stdio.h>
#include "mjpeg_types.h"
#ifdef HAVE_X86CPU
#include "mmx.h"

// These rely on mm7 being zero

// load 8 bytes at *ptr, and expand into 16-bit words in r1 and r2
#define BSAD_LOAD(ptr,r1,r2)    \
	movq_m2r(ptr, r1);      \
	movq_r2r(r1, r2);       \
 	punpcklbw_r2r(mm7, r1); \
	punpckhbw_r2r(mm7, r2);

// load 8 bytes at *ptr, and add to 16-bit words in r1 and r2, using t1 and t2 as temporary registers
#define BSAD_LOAD_ACC(ptr,t1,t2,r1,r2) \
        BSAD_LOAD(ptr,t1,t2);          \
        paddw_r2r(t1,r1);              \
        paddw_r2r(t2,r2);
        


/*
 * absolute difference error between a (16*h) block and a bidirectional
 * prediction
 *
 * p2: address of top left pel of block
 * pf,hxf,hyf: address and half pel flags of forward ref. block
 * pb,hxb,hyb: address and half pel flags of backward ref. block
 * h: height of block
 * lx: distance (in bytes) of vertically adjacent pels in p2,pf,pb
 * mmX version
 */

int bsad_mmx(uint8_t *pf, uint8_t *pb, uint8_t *p2, int lx, int hxf, int hyf, int hxb, int hyb, int h)
{
    uint8_t *pfa,*pfb,*pfc,*pba,*pbb,*pbc;
    int s, s1, s2;

    pfa = pf + hxf;
    pfb = pf + lx * hyf;
    pfc = pfb + hxf;

    pba = pb + hxb;
    pbb = pb + lx * hyb; 
    pbc = pbb + hxb;

    s = 0; /* the accumulator */

    if (h > 0)
    {
        pxor_r2r(mm7, mm7);
        pxor_r2r(mm6, mm6);
        pcmpeqw_r2r(mm5, mm5);
        psubw_r2r(mm5, mm6);
        psllw_i2r(1, mm6);
		
        do {
            BSAD_LOAD(pf[0],mm0,mm1);
            BSAD_LOAD_ACC(pfa[0],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pfb[0],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pfc[0],mm2,mm3,mm0,mm1);
            paddw_r2r(mm6, mm0);
            paddw_r2r(mm6, mm1);
            psrlw_i2r(2, mm0);
            psrlw_i2r(2, mm1);
			
            BSAD_LOAD(pb[0],mm2,mm3);
            BSAD_LOAD_ACC(pba[0],mm4,mm5,mm2,mm3);
            BSAD_LOAD_ACC(pbb[0],mm4,mm5,mm2,mm3);
            BSAD_LOAD_ACC(pbc[0],mm4,mm5,mm2,mm3);
            paddw_r2r(mm6, mm2);
            paddw_r2r(mm6, mm3);
            psrlw_i2r(2, mm2);
            psrlw_i2r(2, mm3);
			
            paddw_r2r(mm2, mm0);
            paddw_r2r(mm3, mm1);
            psrlw_i2r(1, mm6);
            paddw_r2r(mm6, mm0);
            paddw_r2r(mm6, mm1);
            psllw_i2r(1, mm6);
            psrlw_i2r(1, mm0);
            psrlw_i2r(1, mm1);
            packuswb_r2r(mm1, mm0);
			
            movq_m2r(p2[0], mm1);
            movq_r2r(mm0, mm2);
            psubusb_r2r(mm1, mm0);
            psubusb_r2r(mm2, mm1);
            por_r2r(mm1, mm0);
            movq_r2r(mm0, mm1);
            punpcklbw_r2r(mm7, mm0);
            punpckhbw_r2r(mm7, mm1);
            paddw_r2r(mm1, mm0);
            movq_r2r(mm0, mm1);
            punpcklwd_r2r(mm7, mm0);
            punpckhwd_r2r(mm7, mm1);
			
            paddd_r2r(mm1, mm0);
            movd_r2g(mm0, s1);
            psrlq_i2r(32, mm0);
            movd_r2g(mm0, s2);
            s += s1 + s2;

            BSAD_LOAD(pf[8],mm0,mm1);
            BSAD_LOAD_ACC(pfa[8],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pfb[8],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pfc[8],mm2,mm3,mm0,mm1);
            paddw_r2r(mm6, mm0);
            paddw_r2r(mm6, mm1);
            psrlw_i2r(2, mm0);
            psrlw_i2r(2, mm1);
			
            BSAD_LOAD(pb[8],mm2,mm3);
            BSAD_LOAD_ACC(pba[8],mm4,mm5,mm2,mm3);
            BSAD_LOAD_ACC(pbb[8],mm4,mm5,mm2,mm3);
            BSAD_LOAD_ACC(pbc[8],mm4,mm5,mm2,mm3);
            paddw_r2r(mm6, mm2);
            paddw_r2r(mm6, mm3);
            psrlw_i2r(2, mm2);
            psrlw_i2r(2, mm3);
						
            paddw_r2r(mm2, mm0);
            paddw_r2r(mm3, mm1);
            psrlw_i2r(1, mm6);
            paddw_r2r(mm6, mm0);
            paddw_r2r(mm6, mm1);
            psllw_i2r(1, mm6);
            psrlw_i2r(1, mm0);
            psrlw_i2r(1, mm1);
            packuswb_r2r(mm1, mm0);
			
            movq_m2r(p2[8], mm1);
            movq_r2r(mm0, mm2);
            psubusb_r2r(mm1, mm0);
            psubusb_r2r(mm2, mm1);
            por_r2r(mm1, mm0);
            movq_r2r(mm0, mm1);
            punpcklbw_r2r(mm7, mm0);
            punpckhbw_r2r(mm7, mm1);
            paddw_r2r(mm1, mm0);
            movq_r2r(mm0, mm1);
            punpcklwd_r2r(mm7, mm0);
            punpckhwd_r2r(mm7, mm1);
			
            paddd_r2r(mm1, mm0);
            movd_r2g(mm0, s1);
            psrlq_i2r(32, mm0);
            movd_r2g(mm0, s2);
            s += s1 + s2;
			
            p2  += lx;
            pf  += lx;
            pfa += lx;
            pfb += lx;
            pfc += lx;
            pb  += lx;
            pba += lx;
            pbb += lx;
            pbc += lx;

            h--;
        } while (h > 0);	
	
    }
	
    emms();

    return s;
}

static int bsad_2quad_mmxe(uint8_t *pf, uint8_t *pb, uint8_t *p2, int lx, int h)
{
    int s;

    s = 0; /* the accumulator */

    if (h > 0)
    {
        pcmpeqw_r2r(mm6, mm6);
        psrlw_i2r(15, mm6);
        paddw_r2r(mm6, mm6);

        pxor_r2r(mm7, mm7);
        pxor_r2r(mm5, mm5);
		
        do {
            BSAD_LOAD(pf[0],mm0,mm1);
            BSAD_LOAD_ACC(pf[1],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pf[lx],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pf[lx+1],mm2,mm3,mm0,mm1);
            paddw_r2r(mm6, mm0);
            paddw_r2r(mm6, mm1);
            psrlw_i2r(2, mm0);
            psrlw_i2r(2, mm1);
            packuswb_r2r(mm1, mm0);
			
            BSAD_LOAD(pb[0],mm1,mm2);
            BSAD_LOAD_ACC(pb[1],mm3,mm4,mm1,mm2);
            BSAD_LOAD_ACC(pb[lx],mm3,mm4,mm1,mm2);
            BSAD_LOAD_ACC(pb[lx+1],mm3,mm4,mm1,mm2);
            paddw_r2r(mm6, mm1);
            paddw_r2r(mm6, mm2);
            psrlw_i2r(2, mm1);
            psrlw_i2r(2, mm2);
            packuswb_r2r(mm2, mm1);

            pavgb_r2r(mm1, mm0);
            psadbw_m2r(p2[0],mm0);
            paddd_r2r(mm0,mm5);

            BSAD_LOAD(pf[8],mm0,mm1);
            BSAD_LOAD_ACC(pf[9],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pf[lx+8],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pf[lx+9],mm2,mm3,mm0,mm1);
            paddw_r2r(mm6, mm0);
            paddw_r2r(mm6, mm1);
            psrlw_i2r(2, mm0);
            psrlw_i2r(2, mm1);
            packuswb_r2r(mm1, mm0);
			
            BSAD_LOAD(pb[8],mm1,mm2);
            BSAD_LOAD_ACC(pb[9],mm3,mm4,mm1,mm2);
            BSAD_LOAD_ACC(pb[lx+8],mm3,mm4,mm1,mm2);
            BSAD_LOAD_ACC(pb[lx+9],mm3,mm4,mm1,mm2);
            paddw_r2r(mm6, mm1);
            paddw_r2r(mm6, mm2);
            psrlw_i2r(2, mm1);
            psrlw_i2r(2, mm2);
            packuswb_r2r(mm2, mm1);
						
            pavgb_r2r(mm1, mm0);
            psadbw_m2r(p2[8],mm0);
            paddd_r2r(mm0,mm5);
			
            p2  += lx;
            pf  += lx;
            pb  += lx;

            h--;
        } while (h > 0);	
	
    }
    movd_r2g(mm5,s);
	
    emms();

    return s;
}

static int bsad_1quad_mmxe(uint8_t *pf, uint8_t *pb, uint8_t *pb2, uint8_t *p2, int lx, int h)
{
    int s;

    s = 0; /* the accumulator */

    if (h > 0)
    {
        pcmpeqw_r2r(mm6, mm6);
        psrlw_i2r(15, mm6);
        paddw_r2r(mm6, mm6);

        pxor_r2r(mm7, mm7);
        pxor_r2r(mm5, mm5);
		
        do {
            BSAD_LOAD(pf[0],mm0,mm1);
            BSAD_LOAD_ACC(pf[1],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pf[lx],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pf[lx+1],mm2,mm3,mm0,mm1);
            paddw_r2r(mm6, mm0);
            paddw_r2r(mm6, mm1);
            psrlw_i2r(2, mm0);
            psrlw_i2r(2, mm1);
            packuswb_r2r(mm1, mm0);
			
            movq_m2r(pb2[0],mm1);
            pavgb_m2r(pb[0],mm1);

            pavgb_r2r(mm1, mm0);
            psadbw_m2r(p2[0],mm0);
            paddd_r2r(mm0,mm5);

            BSAD_LOAD(pf[8],mm0,mm1);
            BSAD_LOAD_ACC(pf[9],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pf[lx+8],mm2,mm3,mm0,mm1);
            BSAD_LOAD_ACC(pf[lx+9],mm2,mm3,mm0,mm1);
            paddw_r2r(mm6, mm0);
            paddw_r2r(mm6, mm1);
            psrlw_i2r(2, mm0);
            psrlw_i2r(2, mm1);
            packuswb_r2r(mm1, mm0);
			
            movq_m2r(pb2[8],mm1);
            pavgb_m2r(pb[8],mm1);
						
            pavgb_r2r(mm1, mm0);
            psadbw_m2r(p2[8],mm0);
            paddd_r2r(mm0,mm5);
			
            p2  += lx;
            pf  += lx;
            pb  += lx;
            pb2 += lx;

            h--;
        } while (h > 0);	
	
    }
    movd_r2g(mm5,s);
	
    emms();

    return s;
}

/* For a 16*h block, this computes
   (((((*pf + *pf2 + 1)>>1) + ((*pb + *pb2 + 1)>>1) + 1)>>1) + *p2 + 1)>>1
*/
static int bsad_0quad_mmxe(uint8_t *pf,uint8_t *pf2,uint8_t *pb,uint8_t *pb2,uint8_t *p2,int lx,int h)
{
    int32_t s=0;

    pxor_r2r(mm7, mm7);
    do {
        movq_m2r(pf2[0],mm0);
        movq_m2r(pf2[8],mm2);
        movq_m2r(pb2[0],mm1);
        movq_m2r(pb2[8],mm3);
        pavgb_m2r(pf[0],mm0);
        pavgb_m2r(pf[8],mm2);
        pavgb_m2r(pb[0],mm1);
        pavgb_m2r(pb[8],mm3);
        pavgb_r2r(mm1,mm0);
        pavgb_r2r(mm3,mm2);
        psadbw_m2r(p2[0],mm0);
        psadbw_m2r(p2[8],mm2);
        paddd_r2r(mm0,mm7);
        paddd_r2r(mm2,mm7);

        pf+=lx;
        pf2+=lx;
        pb+=lx;
        pb2+=lx;
        p2+=lx;

        h--;
    } while (h);
    movd_r2g(mm7,s);
    emms();
    return s;
}


int bsad_mmxe(uint8_t *pf, uint8_t *pb, uint8_t *p2, int lx, int hxf, int hyf, int hxb, int hyb, int h)
{
    uint8_t *pf2,*pb2;
#if 0
    static int c0=0,c1=0,c2=0,ct=0;

    if( hxf & hyf & hxb & hyb )
        c2++;
    else if( (hxf & hyf) | (hxb & hyb) )
        c1++;
    else 
        c0++;
    ct++;
    if( !(ct&65535) )
        fprintf(stderr,"bsad_mmxe: c0=%d%%, c1=%d%%, c2=%d%%\n",c0*100/ct,c1*100/ct,c2*100/ct);
#endif

    if( hyf )
        pf2=pf+lx;
    else
        pf2=pf+hxf;

    if( hyb )
        pb2=pb+lx;
    else
        pb2=pb+hxb;

    // if either pf or pb have half pels in BOTH directions, then
    // use the slow routine
    if( (hxf & hyf) ) {
        if( (hxb & hyb) )
            return bsad_2quad_mmxe(pf,pb,p2,lx,h);
        else
            return bsad_1quad_mmxe(pf,pb,pb2,p2,lx,h);
    } else {
        if( (hxb & hyb) )
            return bsad_1quad_mmxe(pb,pf,pf2,p2,lx,h);
        else
            return bsad_0quad_mmxe(pf,pf2,pb,pb2,p2,lx,h);
    }
}
#endif
