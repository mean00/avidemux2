/***************************************************************************
                          ADM_vidDeintASM.cpp  -  description
                             -------------------
    begin                : Tue Jan 7 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr

    Slighlty faster ASM deinterlace
    Blend later	

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"

#include "ADM_videoFilter.h"

#include"ADM_vidField.h"

#ifdef ADM_CPU_X86
//	#define DEBUG_DEINT 1
//	#define MMX_TRACE 1
	#include "ADM_mmxMacros.h"

 void myDeintASM(void);

 static int32_t _l_w,_l_h,FUNNY_MANGLE(_l_all);
 static uint8_t * FUNNY_MANGLE(_l_p) , * FUNNY_MANGLE(_l_c) ,* FUNNY_MANGLE(_l_n);
 static uint8_t * FUNNY_MANGLE(_l_e) , * FUNNY_MANGLE(_l_e2);
#define EXPAND(x) (x)+((x)<<16)+((x)<<32) +((x)<<48)
static mmx_t _mmTHRESH1;
static mmx_t _mmTHRESH2;

#define COMPUTE_MMX \
punpcklbw_r2r(mm5,mm0);  /*c  expand 4 bytes -> 4 word */ \
punpcklbw_r2r(mm5,mm1);  /*p*/ \
punpcklbw_r2r(mm5,mm2); /* n*/ \
movq_r2r(mm0,mm3);		/* mm3 also c*/ \
psubw_r2r(mm1,mm0) ; /* mm0=mm0-mm1 =  c-p*/ \
psubw_r2r(mm2,mm3) ; /* mm3=mm3-mm2 =  c-n*/ \
psraw_i2r(1,mm0); /* to protect from overflow*/ \
psraw_i2r(1,mm3);\
pmullw_r2r(mm0,mm3); /* mm3=(c-p)*(c-n) / 4;*/ \
movq_r2r(mm3,mm0) ; /* mm0 also c-p*c-n */ \
pcmpgtw_r2r(mm4,mm3); /* keep only > size*/ \
pcmpgtw_r2r(mm6,mm0); /* keep only > size*/ \
packsswb_r2r(mm5,mm0); \
packsswb_r2r(mm5,mm3);

#endif

void ADMVideoFields::hasMotion_C(uint8_t *p,uint8_t *c,
								uint8_t *n,
								uint8_t *e,
								uint8_t *e2
								)
{
int32_t val,x,y;
//printf("\nC \n");
// other line
       	for(y=_info.height-2;y>0;y--)
        	{
               for(x=_info.width;x>0;x--)
               		{
                   		val= (*p-*c)*(*n-*c);
                        if(val>(int32_t)(_param->motion_trigger*_param->motion_trigger))
                        	{
                          	*e=0xff;
                         }
                         if(val>(int32_t)(_param->blend_trigger*_param->blend_trigger))
                         	*e2=0xff;
                         p++;c++;n++;e++;e2++;
                      }
           }
}
#ifdef ADM_CPU_X86
void ADMVideoFields::hasMotion_MMX(uint8_t *p,uint8_t *c,
									uint8_t *n,
									uint8_t *e,
									uint8_t *e2
									)
{


			 	_mmTHRESH1.uq=EXPAND((uint64_t ) ((_param->motion_trigger*_param->motion_trigger)>>2) );
				_mmTHRESH2.uq=EXPAND((uint64_t ) ((_param->blend_trigger*_param->blend_trigger)>>2) );

			_l_h=_info.height-2;
			_l_w=_info.width>>2;
			_l_all=_l_h*_l_w;
			_l_p=p;
			_l_c=c;
			_l_n=n;
			_l_e=e;
			_l_e2=e2;
//			printf("\n MMX \n");

			pxor_r2r(mm5,mm5);
			movq_m2r(_mmTHRESH1,mm4);
			movq_m2r(_mmTHRESH2,mm6);
			myDeintASM();
}
#if !defined(DEBUG_DEINT)
void myDeintASM(void)
{
                    __asm__ __volatile__ (
                            "push "REG_bx"\n\t" // Dont clobber ebx for macOsX
                            "mov "Mangle(_l_c)",	"REG_ax"\n\t"
                            "mov "Mangle(_l_p)",	"REG_bx"\n\t"
                            "mov "Mangle(_l_n)",	"REG_cx"\n\t"
                            "mov "Mangle(_l_all)",	"REG_si"\n\t"
                            "7:"
                            "movd ("REG_ax"),	%%mm0\n\t"
                            "movd ("REG_bx"),	%%mm1\n\t"
                            "movd ("REG_cx"),	%%mm2\n\t"
                            :
                            :
                            : "eax","ecx","edx","esi" 
                            );

                            COMPUTE_MMX; // Doint it like that should be safe as there should be no extra code inserted (...)

                            /* store result in e, e2 */

                    __asm__ __volatile__(
                            "mov 	"Mangle(_l_e)",	"REG_dx"\n\t"
                            "movd	%%mm3,("REG_dx")\n\t"

                            "mov 	"Mangle(_l_e2)",	"REG_dx"\n\t"
                            "movd	%%mm0,("REG_dx")\n\t"

                            "add 	$4,	"REG_ax"\n\t"
                            "add 	$4,	"REG_bx"\n\t"
                            "add 	$4,	"REG_cx"\n\t"
                            "add 	$4,	"Mangle(_l_e)"\n\t"
                            "add 	$4,	"Mangle(_l_e2)"\n\t"
                            "sub 	$1,	"REG_si"\n\t"
                            "jnz 7b\n\t"
                            "pop "REG_bx"\n\t" // Dont clobber ebx for macOsX
                            :
                            :
                            : "eax", "ecx","edx","esi"
                            );
	   emms();

}
#else
/*************************************************
***************DEBUG*************************
**************************************************
**************************************************/


void myDeintASM(void)
{

int32_t x,y;
	printf("\n using  MMX debug\n");

       	for(y=_l_h;y>0;y--)
        	{
		for(x=_l_w;x>0;x--)
               		{

			__asm__ __volatile__ (
				"mov _l_c,	"REG_ax"\n\t"
				"mov _l_p,	"REG_bx"\n\t"
				"mov _l_n,	"REG_cx"\n\t"
				"mov _l_w,	"REG_si"\n\t"
				"movd ("REG_ax"),	%%mm0\n\t"
				"movd ("REG_bx"),	%%mm1\n\t"
				"movd ("REG_cx"),	%%mm2\n\t"
				:
				:
				: "eax", "ebx", "ecx", "edx","esi"
				);
			emms();
				COMPUTE_MMX;
			emms();
				/* store result in e, e2 */

			__asm__ __volatile__(
				"mov 	_l_e,	"REG_dx"\n\t"
				"movd	%%mm3,("REG_dx")\n\t"
				//"movd	%%mm3,(%%eax)\n\t"

				"mov 	_l_e2,	"REG_dx"\n\t"
				"movd	%%mm0,("REG_dx")\n\t"

				:
				:
				: "eax","edx","esi"
				);

			emms();


			 _l_e+=4;
			 _l_e2+=4;
			 _l_c+=4;
			 _l_p+=4;
			 _l_n+=4;

           } // end for x
	  }
	   emms();
}


#endif
#endif
