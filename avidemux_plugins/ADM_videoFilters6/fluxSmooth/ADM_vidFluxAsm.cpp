/***************************************************************************
                          ADM_vidFlux.cpp  -  description
                             -------------------
    begin                : Tue Dec 31 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
    
    Ported from FluxSmooth
    (c)  Ross Thomas <ross@grinfinity.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include <math.h>


#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_vidFlux.h"

static int16_t FUNNY_MANGLE_ARRAY(scaletab, 16);
static uint64_t FUNNY_MANGLE_ARRAY(scaletab_MMX, 65535);
static bool tableInited=false;
/**
    \fn initScaleTab
*/
void initScaleTab( void )
{
//uint32_t i;
        if(tableInited==true) return;
		scaletab[1] = 32767;
		for(int i = 2; i < 16; ++i)
				scaletab[i] = (int)(32768.0 / i + 0.5);
		for(uint32_t  i = 0; i < 65536; ++i)
		{
			scaletab_MMX[i] = ( (uint64_t)scaletab[ i        & 15]       ) |
							  (((uint64_t)scaletab[(i >>  4) & 15]) << 16) |
							  (((uint64_t)scaletab[(i >>  8) & 15]) << 32) |
							  (((uint64_t)scaletab[(i >> 12) & 15]) << 48);
		}
        tableInited=true;
}
 

static uint64_t FUNNY_MANGLE(spat_thresh) ASM_CONST =0LL;
static uint64_t FUNNY_MANGLE(temp_thresh) ASM_CONST =0LL;
static uint64_t ASM_CONST FUNNY_MANGLE(_l_counter_init),
	FUNNY_MANGLE(_l_indexer), FUNNY_MANGLE(_l_prev_pels),
	FUNNY_MANGLE(_l_next_pels);
static long int FUNNY_MANGLE(_l_src_pitch) ASM_CONST =0;
static long int FUNNY_MANGLE(_l_dst_pitch) ASM_CONST =0;
static int FUNNY_MANGLE(_l_xmax) ASM_CONST=0;

static int FUNNY_MANGLE(ycnt);
static	uint8_t * FUNNY_MANGLE(_l_currp);
static	 uint8_t * FUNNY_MANGLE(_l_prevp);
static	 uint8_t * FUNNY_MANGLE(_l_nextp);
static	 uint8_t * FUNNY_MANGLE(_l_destp);

/**
    \fn DoFilter_C
*/
void ADMVideoFlux::DoFilter_C(
 uint8_t * currp, 
 uint8_t * prevp,								  								  
 uint8_t * nextp, 
 int src_pitch,
 uint8_t * destp, 
 int dst_pitch,
 int row_size, 
 int height, const fluxsmooth &_param)
{

	 int skip = src_pitch - row_size + 1,
		dskip = dst_pitch - row_size + 1;
	int ycnt = height;

	do
	{
		*destp = *currp; // Copy left edge

		++currp;
		++prevp;
		++nextp;
		++destp;

		int xcnt = row_size - 2;

		do
		{
			int pbt = *prevp++, b = *currp, nbt = *nextp++;
			int pdiff = pbt - b, ndiff = nbt - b;
			if((pdiff < 0 && ndiff < 0) || (pdiff > 0 && ndiff > 0))
			{
				int pb1 = currp[-src_pitch - 1], pb2 = currp[-src_pitch],
					pb3 = currp[-src_pitch + 1], b1 = currp[-1], b2 = currp[1],
					nb1 = currp[src_pitch - 1], nb2 = currp[src_pitch],
					nb3 = currp[src_pitch + 1], sum = b, cnt = 1;

				if(abs(pbt - b) <= _param.temporal_threshold)
				{
					sum += pbt;
					++cnt;
				}
				if(abs(nbt - b) <= _param.temporal_threshold)
				{
					sum += nbt;
					++cnt;
				}
				if(abs(pb1 - b) <= _param.spatial_threshold)
				{
					sum += pb1;
					++cnt;
				}
				if(abs(pb2 - b) <= _param.spatial_threshold)
				{
					sum += pb2;
					++cnt;
				}
				if(abs(pb3 - b) <= _param.spatial_threshold)
				{
					sum += pb3;
					++cnt;
				}
				if(abs(b1 - b) <= _param.spatial_threshold)
				{
					sum += b1;
					++cnt;
				}
				if(abs(b2 - b) <= _param.spatial_threshold)
				{
					sum += b2;
					++cnt;
				}
				if(abs(nb1 - b) <= _param.spatial_threshold)
				{
					sum += nb1;
					++cnt;
				}
				if(abs(nb2 - b) <= _param.spatial_threshold)
				{
					sum += nb2;
					++cnt;
				}
				if(abs(nb3 - b) <= _param.spatial_threshold)
				{
					sum += nb3;
					++cnt;
				}

				ADM_assert(sum >= 0);
				ADM_assert(sum < 2806);
				ADM_assert(cnt > 0);
				ADM_assert(cnt < 12);

				*destp++ = (uint8_t )(((sum * 2 + cnt) * scaletab[cnt]) >> 16);
				++currp;
			} else
				*destp++ = *currp++;
		} while(--xcnt);
		ADM_assert(xcnt == 0);

		*destp = *currp; // Copy right edge

		currp += skip;
		prevp += skip;
		nextp += skip;
		destp += dskip;
	} while(--ycnt);
	ADM_assert(ycnt == 0);

}
#ifdef ADM_CPU_X86
/*
	__asm movq mm2, mm0 \
	__asm movq mm3, mm1 \
	__asm psubusw mm2, mm1 \
	__asm psubusw mm3, mm0 \
	__asm por mm2, mm3				/  mm2 = abs diff  / \
	__asm pcmpgtw mm2, threshold	/  Compare with threshold  / \
	__asm paddw mm6, mm2			/  -1 from counter if not within  / \
	__asm pandn mm2, mm1 \
	__asm paddw mm5, mm2			/  Add to sum  / \
*/
#define CHECK_AND_ADD(threshold) " movq %%mm0, %%mm2 \n\t" \
"movq    %%mm1, %%mm3 \n\t" \
"psubusw %%mm1, %%mm2 \n\t" \
"psubusw %%mm0, %%mm3 \n\t" \
"por     %%mm3, %%mm2				\n\t" /* mm2 = abs diff */ \
"pcmpgtw "Mangle(threshold)", %%mm2	\n\t "/* Compare with threshold */ \
"paddw   %%mm2, %%mm6	\n\t	"/* -1 from counter if not within */ \
"pandn   %%mm1, %%mm2 \n\t" \
"paddw   %%mm2, %%mm5	\n\t" /* Add to sum */ 

#define EXPAND(x) { x=x+(x<<8)+(x<<16)+(x<<24)+(x<<32)+(x<<40) \
										+(x<<48);}
/**
    \fn DoFilter_MMX
*/

void ADMVideoFlux::DoFilter_MMX(
uint8_t * currp, 
 uint8_t * prevp,
 uint8_t * nextp, 
 int src_pitch,
 uint8_t * destp, 
 int dst_pitch,
 int row_size, 
 int height, const fluxsmooth &_param)
{
	  _l_xmax = row_size - 4;
	 	ycnt 		= height;
	
	
      _l_currp = currp;
      _l_prevp = prevp;								  								  
      _l_nextp = nextp;  
      _l_destp = destp; 
      _l_src_pitch =src_pitch;
      _l_dst_pitch =dst_pitch;
     
		_l_counter_init = 0x000b000b000b000bLL,
		_l_indexer = 0x1000010000100001LL;
		
		spat_thresh = _param.spatial_threshold;
		temp_thresh = _param.temporal_threshold;
		EXPAND( spat_thresh);
		EXPAND( temp_thresh);

UNUSED_ARG(_l_prev_pels);
UNUSED_ARG(_l_next_pels);
asm(
"                mov "Mangle(_l_currp)", "REG_si" \n\t"
"                mov "Mangle(_l_destp)", "REG_di" \n\t"
"                pxor %%mm7,%%mm7 \n\t"
" \n\t"
"yloop%=:  \n\t"
"                # Copy first dword \n\t"
" \n\t"
"                mov ("REG_si"),"REG_ax" \n\t"
"                mov "REG_ax",("REG_di") \n\t"
" \n\t"
"                mov $4,"REG_cx" \n\t"
" \n\t"
"xloop%=:  \n\t"
"                # Get current pels, init sum and counter \n\t"
" \n\t"
"                movd ("REG_si","REG_cx"),%%mm0 \n\t"
"                punpcklbw %%mm7,%%mm0 \n\t"
"                movq %%mm0,%%mm5 \n\t"
"                movq "Mangle(_l_counter_init)",%%mm6 \n\t"
" \n\t"
"                # Middle left \n\t"
" \n\t"
"                movq %%mm0,%%mm1 \n\t"
"                psllq $16,%%mm1 \n\t"
"                movd -4("REG_si","REG_cx"),%%mm2 \n\t"
"                punpcklbw %%mm7,%%mm2 \n\t"
"                psrlq $48,%%mm2 \n\t"
"                por %%mm2,%%mm1 \n\t"
" \n\t"
CHECK_AND_ADD(spat_thresh)
" \n\t"
"                # Middle right \n\t"
" \n\t"
"                movq %%mm0,%%mm1 \n\t"
"                psrlq $16,%%mm1 \n\t"
"                movd 4("REG_si","REG_cx"),%%mm2 \n\t"
"                punpcklbw %%mm7,%%mm2 \n\t"
"                psllq $48,%%mm2 \n\t"
"                por %%mm2,%%mm1 \n\t"
" \n\t"
CHECK_AND_ADD(spat_thresh)
" \n\t"
"                # Top left \n\t"
" \n\t"
"                mov "REG_si","REG_ax" \n\t"
"                sub "Mangle(_l_src_pitch)", "REG_ax" \n\t"
"                movd ("REG_ax","REG_cx"),%%mm1 \n\t"
"                punpcklbw %%mm7,%%mm1 \n\t"
"                psllq $16,%%mm1 \n\t"
"                movd -4("REG_ax","REG_cx"),%%mm2 \n\t"
"                punpcklbw %%mm7,%%mm2 \n\t"
"                psrlq $48,%%mm2 \n\t"
"                por %%mm2,%%mm1 \n\t"
" \n\t"
CHECK_AND_ADD(spat_thresh)
" \n\t"
"                # Top centre \n\t"
" \n\t"
"                movd ("REG_ax","REG_cx"),%%mm1 \n\t"
"                punpcklbw %%mm7,%%mm1 \n\t"
" \n\t"
CHECK_AND_ADD(spat_thresh)
" \n\t"
"                # Top right \n\t"
" \n\t"
"                psrlq $16,%%mm1 \n\t"
"                movd 4("REG_ax","REG_cx"),%%mm2 \n\t"
"                punpcklbw %%mm7,%%mm2 \n\t"
"                psllq $48,%%mm2 \n\t"
"                por %%mm2,%%mm1 \n\t"
" \n\t"
CHECK_AND_ADD(spat_thresh)
" \n\t"
"                # Bottom left \n\t"
" \n\t"
"                mov "REG_si","REG_ax" \n\t"
"                add "Mangle(_l_src_pitch)", "REG_ax" \n\t"
"                movd ("REG_ax","REG_cx"),%%mm1 \n\t"
"                punpcklbw %%mm7,%%mm1 \n\t"
"                psllq $16,%%mm1 \n\t"
"                movd -4("REG_ax","REG_cx"),%%mm2 \n\t"
"                punpcklbw %%mm7,%%mm2 \n\t"
"                psrlq $48,%%mm2 \n\t"
"                por %%mm2,%%mm1 \n\t"
" \n\t"
CHECK_AND_ADD(spat_thresh)
" \n\t"
"                # Bottom centre \n\t"
" \n\t"
"                movd ("REG_ax","REG_cx"),%%mm1 \n\t"
"                punpcklbw %%mm7,%%mm1 \n\t"
" \n\t"
CHECK_AND_ADD(spat_thresh)
" \n\t"
"                # Bottom right \n\t"
" \n\t"
"                psrlq $16,%%mm1 \n\t"
"                movd 4("REG_ax","REG_cx"),%%mm2 \n\t"
"                punpcklbw %%mm7,%%mm2 \n\t"
"                psllq $48,%%mm2 \n\t"
"                por %%mm2,%%mm1 \n\t"
" \n\t"
CHECK_AND_ADD(spat_thresh)
" \n\t"
"                # Previous frame \n\t"
" \n\t"
"                mov "Mangle(_l_prevp)", "REG_ax" \n\t"
"                movd ("REG_ax","REG_cx"),%%mm1 \n\t"
"                punpcklbw %%mm7,%%mm1 \n\t"
"                movq %%mm1, "Mangle(_l_prev_pels)" \n\t"
" \n\t"
CHECK_AND_ADD(temp_thresh)
" \n\t"
"                # Next frame \n\t"
" \n\t"
"                mov "Mangle(_l_nextp)", "REG_ax" \n\t"
"                movd ("REG_ax","REG_cx"),%%mm1 \n\t"
"                punpcklbw %%mm7,%%mm1 \n\t"
"                movq %%mm1, "Mangle(_l_next_pels)" \n\t"
" \n\t"
CHECK_AND_ADD(temp_thresh)
" \n\t"
"                # Average \n\t"
" \n\t"
"                psllw $1,%%mm5                                  # sum *= 2 \n\t"
"                paddw %%mm6,%%mm5                               # sum += count \n\t"
" \n\t"
"                pmaddwd "Mangle(_l_indexer)",%%mm6                  # Make index into lookup \n\t"
"                movq %%mm6,%%mm1 \n\t"
"                punpckhdq %%mm6,%%mm6 \n\t"
"                mov "Mangle(scaletab_MMX)", "REG_ax" \n\t"
"                paddd %%mm6,%%mm1 \n\t"
"                movd %%mm1,"REG_bx" \n\t"
" \n\t"
"            movq ("REG_ax","REG_bx",8),%%mm2          # Do lookup \n\t"
"            pmulhw %%mm2,%%mm5                                  # mm5 = average \n\t"
" \n\t"
"                # Apply smoothing only to fluctuating pels \n\t"
" \n\t"
"                movq %%mm0,%%mm1 \n\t"
"                movq "Mangle(_l_prev_pels)",%%mm2 \n\t"
"                movq %%mm0,%%mm3 \n\t"
"                movq "Mangle(_l_next_pels)",%%mm4 \n\t"
" \n\t"
"                pcmpgtw %%mm2,%%mm1                             # curr > prev \n\t"
"                pcmpgtw %%mm4,%%mm3                             # curr > next \n\t"
"                pcmpgtw %%mm0,%%mm2                             # prev > curr \n\t"
"                pcmpgtw %%mm0,%%mm4                             # next > curr \n\t"
" \n\t"
"                pand %%mm3,%%mm1                                # (curr > prev) and (curr > next) \n\t"
"                pand %%mm4,%%mm2                                # (prev > curr) and (next > curr) \n\t"
"                por %%mm2,%%mm1                                 # mm1 = FFh if fluctuating, else 00h \n\t"
" \n\t"
"                movq %%mm1,%%mm2 \n\t"
"                pand %%mm5,%%mm1                                # mm1 = smoothed pels \n\t"
"                pandn %%mm0,%%mm2                               # mm2 = unsmoothed pels \n\t"
"                por %%mm2,%%mm1                                 # mm1 = result \n\t"
" \n\t"
"                # Store \n\t"
" \n\t"
"                packuswb %%mm7,%%mm1 \n\t"
"                movntq %%mm1,("REG_di","REG_cx") \n\t"
" \n\t"
"                # Advance \n\t"
" \n\t"
"                add $4,"REG_cx" \n\t"
"                cmp "Mangle(_l_xmax)", "REG_cx" \n\t"
"                jl xloop%= \n\t"
" \n\t"
"                # Copy last dword \n\t"
" \n\t"
"                mov ("REG_si","REG_cx"),"REG_ax" \n\t"
"                mov "REG_ax",("REG_di","REG_cx") \n\t"
" \n\t"
"                # Next row \n\t"
" \n\t"
"                add "Mangle(_l_src_pitch)", "REG_si" \n\t"
"                mov "Mangle(_l_prevp)", "REG_ax" \n\t"
"                add "Mangle(_l_src_pitch)", "REG_ax" \n\t"
"                mov "REG_ax", "Mangle(_l_prevp)" \n\t"
"                mov "Mangle(_l_nextp)", "REG_bx" \n\t"
"                add "Mangle(_l_src_pitch)", "REG_bx" \n\t"
"                mov "REG_bx", "Mangle(_l_nextp)" \n\t"
"                add "Mangle(_l_dst_pitch)", "REG_di" \n\t"
" \n\t"
"                subl $1, "Mangle(ycnt)" \n\t"
"                jnz yloop%= \n\t"
" \n\t"
//"MISMATCH: "                sfence" \n\t"
"                emms \n\t"
" \n\t"

 : : );
}
#endif
//
