
#include "ADM_default.h"
#include "decimate.h"

#ifdef DECIMATE_MMX_BUILD_PLANE
//
//
//
//
void isse_blend_decimate_plane(uint8_t * dst, uint8_t* src,  uint8_t* src_next, 
			int w, int h)
{
uint32_t x;
	if (!h) return;  // Height == 0 - avoid silly crash.
	
	x=w>>3; // 8 pixels at a time
	for(;x>0;x--)
	{
	 __asm__(
                ADM_ALIGN16
	 	"movq  (%1), %%mm0 \n"
		"movq  (%2), %%mm2 \n"
		"pavgb %%mm0,%%mm1 \n"
		"movq  %%mm1,(%0) \n"

                   : : "r" (dst), "r" (src), "r" (src_next));
		
		dst+=8;
		src+=8;
		src_next+=8;
  	}
    	__asm__("emms");
  
}
int isse_scenechange_32(const uint8_t *c_plane, const uint8_t *tplane, int height, int width) 
{
  int wp=width>>5;
  int hp=height;
  int returnvalue=0xbadbad00;
    
    __asm__(
    ADM_ALIGN16
    "pxor %%mm6,%%mm6\n"
    "pxor %%mm7,%%mm7\n"
    ::);
    for(uint32_t y=0;y<hp;y++)
    {
	for(uint32_t x=0;x<wp;x++)
	{
		__asm__(
    		ADM_ALIGN16
    		"movq (%0),%%mm0 \n"
		"movq 8(%0),%%mm2 \n"
		"movq (%1),%%mm1 \n"
		"movq 8(%1),%%mm3 \n"
		"psadbw %%mm1,%%mm0\n"
		"psadbw %%mm3,%%mm2\n"
		"paddd %%mm0,%%mm6 \n"
		"paddd %%mm2,%%mm7 \n"
		
		"movq 16(%0),%%mm0 \n"
		"movq 24(%0),%%mm2 \n"
		"movq 16(%1),%%mm1 \n"
		"movq 24(%1),%%mm3 \n"
		"psadbw %%mm1,%%mm0\n"
		"psadbw %%mm3,%%mm2\n"
		"paddd %%mm0,%%mm6 \n"
		"paddd %%mm2,%%mm7 \n"
		
		
		: : "r" (c_plane) , "r" (tplane)
		);
		c_plane+=32;
		tplane+=32;
	}    
    
    	c_plane+=width-wp*32;
	tplane+=width-wp*32;
    }
    __asm__(
    ADM_ALIGN16
    "paddd %%mm6,%%mm7\n"
    "movd %%mm7,(%0)\n"
    "emms \n"
    : : "r" (&returnvalue)
    );
  
  return returnvalue;
}
int isse_scenechange_16(const uint8_t *c_plane, const uint8_t *tplane, int height, int width) 
{
  int wp=width>>4;
  int hp=height;
  int returnvalue=0xbadbad00;
    
    __asm__(
    ADM_ALIGN16
    "pxor %%mm6,%%mm6\n"
    "pxor %%mm7,%%mm7\n"
    ::);
    for(uint32_t y=0;y<hp;y++)
    {
	for(uint32_t x=0;x<wp;x++)
	{
		__asm__(
    		ADM_ALIGN16
    		"movq (%0),%%mm0 \n"
		"movq 8(%0),%%mm2 \n"
		"movq (%1),%%mm1 \n"
		"movq 8(%1),%%mm3 \n"
		"psadbw %%mm1,%%mm0\n"
		"psadbw %%mm3,%%mm2\n"
		"paddd %%mm0,%%mm6 \n"
		"paddd %%mm2,%%mm7 \n"				
		
		
		: : "r" (c_plane) , "r" (tplane)
		);
		c_plane+=16;
		tplane+=16;
	}    
    
    	c_plane+=width-wp*16;
	tplane+=width-wp*16;
    }
    __asm__(
    ADM_ALIGN16
    "paddd %%mm6,%%mm7\n"
    "movd %%mm7,(%0)\n"
    "emms \n"
    : : "r" (&returnvalue)
    );
  
  return returnvalue;
}
int isse_scenechange_8(const uint8_t *c_plane, const uint8_t *tplane, int height, int width) 
{
  int wp=width>>3;
  int hp=height;
  int returnvalue=0xbadbad00;
    
    __asm__(
    ADM_ALIGN16
    "pxor %%mm6,%%mm6\n"
    "pxor %%mm7,%%mm7\n"
    ::);
    for(uint32_t y=0;y<hp;y++)
    {
	for(uint32_t x=0;x<wp;x++)
	{
		__asm__(
    		ADM_ALIGN16
    		"movq (%0),%%mm0 \n"		
		"movq (%1),%%mm1 \n"		
		"psadbw %%mm1,%%mm0\n"		
		"paddd %%mm0,%%mm6 \n"
		
		: : "r" (c_plane) , "r" (tplane)
		);
		c_plane+=8;
		tplane+=8;
	}    
    
    	c_plane+=width-wp*8;
	tplane+=width-wp*8;
    }
    __asm__(
    ADM_ALIGN16
    "movd %%mm6,(%0)\n"
    "emms \n"
    : : "r" (&returnvalue)
    );
  
  return returnvalue;
}

#endif
