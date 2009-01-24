/**/
/*___________________________________________
	Do dual field conversion
		YUY2-> YV12
	i'm not that good with mmx stuff (sorry :) )

	-->Handle full screen conversion--->

M
___________________________________________

*/

#include <stdio.h>
#include <stdlib.h>
unsigned long long dummy_no_warning( void );

//
//	Here we produce full sized image
//	So just dual YUY2 -> YV12 conversion
//

static volatile unsigned long long
mmx_lowbyte=0x00ff00ff00ff00ffLL,
mmx_w1=0x0001000100010001LL,
mmx_vbyte  =0x000000ff000000ffLL;	

 unsigned long long dummy_no_warning( void )
{
 return mmx_lowbyte+mmx_w1+mmx_vbyte;		
}




/*
 *  422 -> 420 with 2 diff fields
 *
 *
 */
/*
 * 0-> in
 * 1-> y
 */

#define PACK_Y1 "\n\
# load y,u,v                                                 		\n\
movq 		(%0),	%%mm0    # load 4u     v1 y3 u1 y2 v0 y1 u0 y0   	\n\
movq		%%mm0,	%%mm4 # \n\
pand 		mmx_lowbyte,%%mm0 # 00 y3 00 y2 00 y1 00 y0 			\n\
packuswb	%%mm0,	%%mm0 # 000  y3 Y2 Y1 Y0 				\n\
movd            %%mm0,	(%1)	\n\
"	
#define PACK_Y2 "\n\
# load y,u,v                                                 		\n\
movq 		(%0),	%%mm0    # load 4u     v1 y3 u1 y2 v0 y1 u0 y0   	\n\
movq		%%mm0,	%%mm5 # \n\
pand 		mmx_lowbyte,%%mm0 # 00 y3 00 y2 00 y1 00 y0 			\n\
packuswb	%%mm0,	%%mm0 # 000  y3 Y2 Y1 Y0 				\n\
movd            %%mm0,	(%1)	\n\
"	

#define PREPARE_UV " \n\
psrlq 		$8,	%%mm4			# 00 V1 Y3 U1 Y2 V0 Y1 U0 \n\
psrlq 		$8,	%%mm5			# 00 V1 Y3 U1 Y2 V0 Y1 U0 \n\
pand 		mmx_lowbyte,	%%mm4 #    00 V1 00 U1 00 V0 00 U0 \n\
pand 		mmx_lowbyte,	%%mm5 #    00 V1 00 U1 00 V0 00 U0 \n\
paddw    	%%mm4,%%mm5 				# \n\
psrlw	 	$1,%%mm5 					# \n\
packuswb	%%mm5,	%%mm5 				# 00 00 00 00 V1 U1 V0 U0 \n\
psllq 		$32,	%%mm5			# x x x x 0 0 0 0  \n\
psrlq 		$32,	%%mm5			# 0 0 0 0 x x x x \n\
movq		%%mm5,	%%mm3 \n\
"
#define PREPARE_UV2 " \n\
psrlq 		$8,	%%mm4			# 00 V1 Y3 U1 Y2 V0 Y1 U0 \n\
psrlq 		$8,	%%mm5			# 00 V1 Y3 U1 Y2 V0 Y1 U0 \n\
pand 		mmx_lowbyte,	%%mm4 #    00 V1 00 U1 00 V0 00 U0 \n\
pand 		mmx_lowbyte,	%%mm5 #    00 V1 00 U1 00 V0 00 U0 \n\
paddw    	%%mm4,%%mm5 				# \n\
psrlw	 	$1,%%mm5 					# \n\
packuswb	%%mm5,	%%mm5 				# 00 00 00 00 V1 U1 V0 U0 \n\
psllq 		$32,	%%mm5			# x x x x 0 0 0 0  \n\
paddw		%%mm5,	%%mm3			# x x x x y y y y \n\
"

#define PACK_U " \n\
movq 		%%mm3,	%%mm0 \n\
psllw		$8,	%%mm0 \n\
psrlw		$8,	%%mm0   # 0 0 0 0 0 u1 0 u0 \n\
packuswb	%%mm0,	%%mm0 # 000  0 0 u1 u0 				\n\
movd 		%%mm0,(%0) \n\
"


#define PACK_V " \n\
movq 		%%mm3,	%%mm0 \n\
psrlw		$8,	%%mm0   # 0 0 0 0 0 u1 0 u0 \n\
packuswb	%%mm0,	%%mm0 # 000  0 0 u1 u0 				\n\
movd 		%%mm0,(%0) \n\
"

/*
		This is fullscreen Part

*/
/*
		For Xvid
*/
	
void	YU_YV12_mmx(unsigned char *in,unsigned char *out, int w,int h)
{
	unsigned char *y,*u,*v;
	unsigned char *y2,*in2;
	int yy,xx;
	
	in2=in+w*h;
	y=out;
	y2=y+w;
	v=y+(w*h);
	u=v+((w*h)>>2);
	
	for(yy=h>>1;yy>0;yy--)
	{
		for(xx=w>>3;xx>0;xx--)
		{
			__asm__( PACK_Y1 
				: : "r" (in),"r" (y)
				);
			__asm__( PACK_Y2 
				: : "r" (in2),"r" (y2)
				);
			__asm__( PREPARE_UV
					: : "r"(yy)
					); 
			in+=8;
			in2+=8;
			y+=4;
			y2+=4;
			
			/*______________*/
			__asm__( PACK_Y1 
				: : "r" (in),"r" (y)
			);
			__asm__( PACK_Y2 
				: : "r" (in2),"r" (y2)
			);
			__asm__( PREPARE_UV2
			: : "r"(yy)
					
			); 
			__asm__( PACK_U
				: : "r" (u)
				); 
			__asm__( PACK_V
				: : "r" (v)
				); 
			u+=4;
			v+=4;
			
			in+=8;
			in2+=8;
			y+=4;
			y2+=4;
			
			
		}
		y+=w;
		y2+=w;
	}

	__asm__("EMMS");
}

void	YU_YV12_mmx_swapped(unsigned char *in,unsigned char *out, int w,int h)
{
	unsigned char *y,*u,*v;
	unsigned char *y2,*in2;
	int yy,xx;
	
	in2=in+w*h;
	y=out;
	y2=y+w;
	u=y+(w*h);
	v=u+((w*h)>>2);
	
	for(yy=h>>1;yy>0;yy--)
	{
		for(xx=w>>3;xx>0;xx--)
		{
			__asm__( PACK_Y1 
				: : "r" (in),"r" (y)
				);
			__asm__( PACK_Y2 
				: : "r" (in2),"r" (y2)
				);
			__asm__( PREPARE_UV
					: : "r"(yy)
					); 
			in+=8;
			in2+=8;
			y+=4;
			y2+=4;
			
			/*______________*/
			__asm__( PACK_Y1 
				: : "r" (in),"r" (y)
			);
			__asm__( PACK_Y2 
				: : "r" (in2),"r" (y2)
			);
			__asm__( PREPARE_UV2
			: : "r"(yy)					
			); 
			__asm__( PACK_U
				: : "r" (u)
				); 
			__asm__( PACK_V
				: : "r" (v)
				); 
			u+=4;
			v+=4;
			in+=8;
			in2+=8;
			y+=4;
			y2+=4;
			
			
		}
		y+=w;
		y2+=w;
	}

	__asm__("EMMS");
}



//------------------

