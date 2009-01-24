/* bsumsq.c, this file is part of the
 * AltiVec optimized library for MJPEG tools MPEG-1/2 Video Encoder
 * Copyright (C) 2002  James Klicman <james@klicman.org>
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

#include "altivec_motion.h"
#include "vectorize.h"
#include "../mjpeg_logging.h"

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif

/*
 * squared error between a (16*h) block and a bidirectional
 * prediction
 *
 * p2: address of top left pel of block
 * pf,hxf,hyf: address and half pel flags of forward ref. block
 * pb,hxb,hyb: address and half pel flags of backward ref. block
 * h: height of block
 * rowstride: distance (in bytes) of vertically adjacent pels in p2,pf,pb
 * 
 * 
 *  Input hints:
 *  a) values for h[xy][fb] are 0 and 1
 *  b) h is 8 or 16
 * 
 * {
 *     d = ((((pf[i]+pfa[i]+pfb[i]+pfc[i]+2)>>2)+
 *           ((pb[i]+pba[i]+pbb[i]+pbc[i]+2)>>2)+1)>>1) - p2[i];
 *     sum += d * d;
 * }
 */

#define BSUMSQ_PDECL                                                         \
  uint8_t *pf,                                                               \
  uint8_t *pb,                                                               \
  uint8_t *p2,                                                               \
  int rowstride,                                                             \
  int hxf,                                                                   \
  int hyf,                                                                   \
  int hxb,                                                                   \
  int hyb,                                                                   \
  int h                                                                      \

#define BSUMSQ_ARGS pf, pb, p2, rowstride, hxf, hyf, hxb, hyb, h

int bsumsq_altivec(BSUMSQ_PDECL)
{
    int i;
    uint8_t *pfy, *pby;
    vector unsigned char l0, l1, lR;
    vector unsigned char permF0, permF1, permB0, permB1;
    vector unsigned char vf, vfa, vfb, vfc;
    vector unsigned char vb, vba, vbb, vbc;
    vector unsigned short tH, tL, fH, fL, bH, bL;
    vector unsigned char zero;
    vector unsigned short one, two;
    vector unsigned char max, min, dif;
    vector unsigned int sum;
    union {
	vector signed int v;
	struct {
	    signed int pad[3];
	    signed int sum;
	} s;
    } vo;


 
#ifdef ALTIVEC_VERIFY
    if (hxf != 0 && hxf != 1)
	mjpeg_error_exit1("bsumsq: hxf != [0|1], (hxf=%d)", hxf);

    if (hyf != 0 && hyf != 1)
	mjpeg_error_exit1("bsumsq: hyf != [0|1], (hyf=%d)", hyf);

    if (hxb != 0 && hxb != 1)
	mjpeg_error_exit1("bsumsq: hxb != [0|1], (hxb=%d)", hxb);

    if (hyb != 0 && hyb != 1)
	mjpeg_error_exit1("bsumsq: hyb != [0|1], (hyb=%d)", hyb);

    if (NOT_VECTOR_ALIGNED(p2))
	mjpeg_error_exit1("bsumsq: p2 %% 16 != 0, (0x%X)", p2);

    if (NOT_VECTOR_ALIGNED(rowstride))
	mjpeg_error_exit1("bsumsq: rowstride %% 16 != 0, (%d)", rowstride);

    if (h != 8 && h != 16)
	mjpeg_error_exit1("bsumsq: h != [8|16], (%d)", h);
#endif

    AMBER_START;


    /* start loading first set  */
    vfb = vec_ld(0, pf);	 /* use vfb & vfc as temp for vf & vfa */
    vfc = vec_ld(16, pf);

    pfy = pf + (rowstride * hyf);
    l0 = vec_ld(0, pfy);
    l1 = vec_ld(16, pfy);


    pby = pb + (rowstride * hyb);


    zero  = vec_splat_u8(0);
    one = vec_splat_u16(1);
    two = vec_splat_u16(2);

    sum = vec_splat_u32(0);


    permF0 = vec_lvsl(0, pf);
    permF1 = vec_lvsl(hxf, (unsigned char*)0);
    permF1 = vec_splat(permF1, 0);
    permF1 = vec_add(permF0, permF1);
    
    permB0 = vec_lvsl(0, pb);
    permB1 = vec_lvsl(hxb, (unsigned char*)0);
    permB1 = vec_splat(permB1, 0);
    permB1 = vec_add(permB0, permB1);

    
    i = h - 1;
    do { /* while (--i) */

	vf = vec_perm(vfb, vfc, permF0);
	vfa = vec_perm(vfb, vfc, permF1);
	vfb = vec_perm(l0, l1, permF0);
	vfc = vec_perm(l0, l1, permF1);

	vbb = vec_ld(0, pb);	 /* use vbb & vbc as temp for vb & vba */
	vbc = vec_ld(16, pb);
	l0 = vec_ld(0, pby);
	l1 = vec_ld(16, pby);

	pb += rowstride;
	pby += rowstride;

	/* (unsigned short[]) pf[0-7] */    
	fH = vu16(vec_mergeh(zero, vf));
			
	/* (unsigned short[]) pf[8-15] */   
	fL = vu16(vec_mergel(zero, vf));
			
	/* (unsigned short[]) pfa[0-7] */    
	tH = vu16(vec_mergeh(zero, vfa));
			
	/* (unsigned short[]) pfa[8-15] */   
	tL = vu16(vec_mergel(zero, vfa));

	/* pf[i] + pfa[i] */                                                 
	fH = vec_add(fH, tH);                                               
	fL = vec_add(fL, tL);                                               

	/* (unsigned short[]) pfb[0-7] */  
	tH = vu16(vec_mergeh(zero, vfb));
			
	/* (unsigned short[]) pfb[8-15] */ 
	tL = vu16(vec_mergel(zero, vfb));

	/* (pf[i]+pfa[i]) + pfb[i] */                                       
	fH = vec_add(fH, tH);                                                
	fL = vec_add(fL, tL);                                                
			
	/* (unsigned short[]) pfc[0-7] */  
	tH = vu16(vec_mergeh(zero, vfc));
			
	/* (unsigned short[]) pfc[8-15] */ 
	tL = vu16(vec_mergel(zero, vfc));

	/* (pf[i]+pfa[i]+pfb[i]) + pfc[i] */
	fH = vec_add(fH, tH);                                                
	fL = vec_add(fL, tL);                                                

							
	/* (pf[i]+pfa[i]+pfb[i]+pfc[i]) + 2 */
	fH = vec_add(fH, two);                                                
	fL = vec_add(fL, two);                                                
							
	/* (pf[i]+pfa[i]+pfb[i]+pfc[i]+2) >> 2 */
	fH = vec_sra(fH, two);                                                
	fL = vec_sra(fL, two);                                                


	lR = vec_ld(0, p2);
	p2 += rowstride;

	vb = vec_perm(vbb, vbc, permB0);
	vba = vec_perm(vbb, vbc, permB1);
	vbb = vec_perm(l0, l1, permB0);
	vbc = vec_perm(l0, l1, permB1);


	pf += rowstride;
	vfb = vec_ld(0, pf);	 /* use vfb & vfc as temp for vf & vfa */
	vfc = vec_ld(16, pf);
	pfy += rowstride;
	l0 = vec_ld(0, pfy);
	l1 = vec_ld(16, pfy);

	/* (unsigned short[]) pb[0-7] */    
	bH = vu16(vec_mergeh(zero, vb));

	/* (unsigned short[]) pb[8-15] */   
	bL = vu16(vec_mergel(zero, vb));

	/* (unsigned short[]) pba[0-7] */    
	tH = vu16(vec_mergeh(zero, vba));

	/* (unsigned short[]) pba[8-15] */   
	tL = vu16(vec_mergel(zero, vba));

	/* pb[i] + pba[i] */                                                 
	bH = vec_add(bH, tH);                                               
	bL = vec_add(bL, tL);                                               

	/* (unsigned short[]) pbb[0-7] */  
	tH = vu16(vec_mergeh(zero, vbb));

	/* (unsigned short[]) pbb[8-15] */ 
	tL = vu16(vec_mergel(zero, vbb));

	/* (pb[i]+pba[i]) + pbb[i] */                                       
	bH = vec_add(bH, tH);                                                
	bL = vec_add(bL, tL);                                                
			
	/* (unsigned short[]) pbc[0-7] */  
	tH = vu16(vec_mergeh(zero, vbc));

	/* (unsigned short[]) pbc[8-15] */ 
	tL = vu16(vec_mergel(zero, vbc));

	/* (pb[i]+pba[i]+pbb[i]) + pbc[i] */
	bH = vec_add(bH, tH);                                                
	bL = vec_add(bL, tL);                                                

							
	/* (pb[i]+pba[i]+pbb[i]+pbc[i]) + 2 */
	bH = vec_add(bH, two);                                                
	bL = vec_add(bL, two);                                                

	/* (pb[i]+pba[i]+pbb[i]+pbc[i]+2) >> 2 */
	bH = vec_sra(bH, two);                                                
	bL = vec_sra(bL, two);                                                

	/* ((pf[i]+pfa[i]+pfb[i]+pfc[i]+2)>>2) +
	 * ((pb[i]+pba[i]+pbb[i]+pbc[i]+2)>>2)
         */
	tH = vec_add(fH, bH);                                                
	tL = vec_add(fL, bL);                                                

	/* (((pf[i]+pfa[i]+pfb[i]+pfc[i]+2)>>2)+
	 *  ((pb[i]+pba[i]+pbb[i]+pbc[i]+2)>>2)) + 1
         */
	tH = vec_add(tH, one);                                                
	tL = vec_add(tL, one);                                                

	/* (((pf[i]+pfa[i]+pfb[i]+pfc[i]+2)>>2)+
	 *  ((pb[i]+pba[i]+pbb[i]+pbc[i]+2)>>2)+1) >> 1
         */
	tH = vec_sra(tH, one);                                                
	tL = vec_sra(tL, one);                                                

	/* absolute value increases parallelism (x16 instead of x8)
	 * since a bit isn't lost on the sign.
	 * 
	 * d = abs( ((((pf[i]+pfa[i]+pfb[i]+pfc[i]+2)>>2)+
	 *            ((pb[i]+pba[i]+pbb[i]+pbc[i]+2)>>2)+1)>>1) - p2[i] )
         */
	tH = vu16(vec_packsu(tH, tL));
	min = vec_min(vu8(tH), lR);                                           
	max = vec_max(vu8(tH), lR);                                           
	dif = vec_sub(max, min);                                              

	/* sum += (d * d) */                                                   
	sum = vec_msum(dif, dif, sum);                                        

    } while (--i);

    vf = vec_perm(vfb, vfc, permF0);
    vfa = vec_perm(vfb, vfc, permF1);
    vfb = vec_perm(l0, l1, permF0);
    vfc = vec_perm(l0, l1, permF1);

    vbb = vec_ld(0, pb);	 /* use vbb & vbc as temp for vb & vba */
    vbc = vec_ld(16, pb);
    l0 = vec_ld(0, pby);
    l1 = vec_ld(16, pby);

    /* (unsigned short[]) pf[0-7] */    
    fH = vu16(vec_mergeh(zero, vf));
			
    /* (unsigned short[]) pf[8-15] */   
    fL = vu16(vec_mergel(zero, vf));
			
    /* (unsigned short[]) pfa[0-7] */    
    tH = vu16(vec_mergeh(zero, vfa));
			
    /* (unsigned short[]) pfa[8-15] */   
    tL = vu16(vec_mergel(zero, vfa));

    /* pf[i] + pfa[i] */                                                 
    fH = vec_add(fH, tH);                                               
    fL = vec_add(fL, tL);                                               

    /* (unsigned short[]) pfb[0-7] */  
    tH = vu16(vec_mergeh(zero, vfb));

    /* (unsigned short[]) pfb[8-15] */ 
    tL = vu16(vec_mergel(zero, vfb));

    /* (pf[i]+pfa[i]) + pfb[i] */                                       
    fH = vec_add(fH, tH);                                                
    fL = vec_add(fL, tL);                                                

    /* (unsigned short[]) pfc[0-7] */  
    tH = vu16(vec_mergeh(zero, vfc));
			
    /* (unsigned short[]) pfc[8-15] */ 
    tL = vu16(vec_mergel(zero, vfc));

    /* (pf[i]+pfa[i]+pfb[i]) + pfc[i] */
    fH = vec_add(fH, tH);                                                
    fL = vec_add(fL, tL);                                                

    /* (pf[i]+pfa[i]+pfb[i]+pfc[i]) + 2 */
    fH = vec_add(fH, two);
    fL = vec_add(fL, two);

    /* (pf[i]+pfa[i]+pfb[i]+pfc[i]+2) >> 2 */
    fH = vec_sra(fH, two);
    fL = vec_sra(fL, two);

    lR = vec_ld(0, p2);

    vb = vec_perm(vbb, vbc, permB0);
    vba = vec_perm(vbb, vbc, permB1);
    vbb = vec_perm(l0, l1, permB0);
    vbc = vec_perm(l0, l1, permB1);

    /* (unsigned short[]) pb[0-7] */    
    bH = vu16(vec_mergeh(zero, vb));
			
    /* (unsigned short[]) pb[8-15] */   
    bL = vu16(vec_mergel(zero, vb));

    /* (unsigned short[]) pba[0-7] */
    tH = vu16(vec_mergeh(zero, vba));

    /* (unsigned short[]) pba[8-15] */   
    tL = vu16(vec_mergel(zero, vba));

    /* pb[i] + pba[i] */                                                 
    bH = vec_add(bH, tH);                                               
    bL = vec_add(bL, tL);                                               

    /* (unsigned short[]) pbb[0-7] */  
    tH = vu16(vec_mergeh(zero, vbb));

    /* (unsigned short[]) pbb[8-15] */ 
    tL = vu16(vec_mergel(zero, vbb));

    /* (pb[i]+pba[i]) + pbb[i] */                                       
    bH = vec_add(bH, tH);                                                
    bL = vec_add(bL, tL);                                                
			
    /* (unsigned short[]) pbc[0-7] */  
    tH = vu16(vec_mergeh(zero, vbc));
			
    /* (unsigned short[]) pbc[8-15] */ 
    tL = vu16(vec_mergel(zero, vbc));

    /* (pb[i]+pba[i]+pbb[i]) + pbc[i] */
    bH = vec_add(bH, tH);                                                
    bL = vec_add(bL, tL);                                                

							
    /* (pb[i]+pba[i]+pbb[i]+pbc[i]) + 2 */
    bH = vec_add(bH, two);                                                
    bL = vec_add(bL, two);                                                
							
    /* (pb[i]+pba[i]+pbb[i]+pbc[i]+2) >> 2 */
    bH = vec_sra(bH, two);                                                
    bL = vec_sra(bL, two);                                                

    /* ((pf[i]+pfa[i]+pfb[i]+pfc[i]+2)>>2) +
     * ((pb[i]+pba[i]+pbb[i]+pbc[i]+2)>>2)
     */
    tH = vec_add(fH, bH);                                                
    tL = vec_add(fL, bL);                                                

    /* (((pf[i]+pfa[i]+pfb[i]+pfc[i]+2)>>2)+
     *  ((pb[i]+pba[i]+pbb[i]+pbc[i]+2)>>2)) + 1
     */
    tH = vec_add(tH, one);                                                
    tL = vec_add(tL, one);

    /* (((pf[i]+pfa[i]+pfb[i]+pfc[i]+2)>>2)+
     *  ((pb[i]+pba[i]+pbb[i]+pbc[i]+2)>>2)+1) >> 1
     */
    tH = vec_sra(tH, one);                                                
    tL = vec_sra(tL, one);                                                

    /* absolute value increases parallelism (x16 instead of x8)
     * since a bit isn't lost on the sign.
     * 
     * d = abs( ((((pf[i]+pfa[i]+pfb[i]+pfc[i]+2)>>2)+
     *            ((pb[i]+pba[i]+pbb[i]+pbc[i]+2)>>2)+1)>>1) - p2[i] )
     */
    tH = vu16(vec_packsu(tH, tL));
    min = vec_min(vu8(tH), lR);                                           
    max = vec_max(vu8(tH), lR);                                           
    dif = vec_sub(max, min);                                              

    /* sum += (d * d) */                                                   
    sum = vec_msum(dif, dif, sum);                                        

    /* sum all parts of difference into one 32 bit quantity */
    vo.v = vec_sums(vs32(sum), vs32(zero));

    AMBER_STOP;
    return vo.s.sum;
}

#if ALTIVEC_TEST_FUNCTION(bsumsq)

#undef BENCHMARK_FREQUENCY
#define BENCHMARK_FREQUENCY  1000   /* benchmark every (n) calls */

ALTIVEC_TEST(bsumsq, int, (BSUMSQ_PDECL),
  "pf=0x%X, pb=0x%X, p2=0x%X, rowstride=%d, "
  "hxf=%d, hyf=%d, hxb=%d, hyb=%d, h=%d",
  BSUMSQ_ARGS);
#endif
/* vim:set foldmethod=marker foldlevel=0: */
