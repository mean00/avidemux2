/***************************************************************************
                          ADM_vidVlad.cpp  -  description
                             -------------------
    begin                : Fri Jan 3 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr
    
    Port from Vlad59 / Jim Casaburi TemporalCleaner from avisynth YV12
    
    Luma only
   	Patch by Daniel Glockner 
    
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
#include "ADM_videoFilterDynamic.h"
#include "DIA_factory.h"

#include "ADM_vidVlad.h"

#include "DIA_factory.h"

#ifdef ADM_CPU_X86_64
#define COUNTER long int
#else
#define COUNTER int
#endif

#ifdef ADM_CPU_X86
static void ProcessCPlane_mmxe(unsigned char *source,
				   unsigned char *prev,
				   unsigned char* dest,
				   unsigned char* mask,
				   int width, int height,
				   uint64_t  threshold);
static void ProcessYPlane_mmxe( unsigned char *source,
				    unsigned char *prev,
				    unsigned char* dest,
				    unsigned char* mask,
				    long  int width, long int height,
				    uint64_t  threshold);
#endif
static void ProcessCPlane_C(unsigned char *source,
				   unsigned char *prev,
				   unsigned char* dest,
				   unsigned char* mask,
				   int width, int height,
				   uint64_t  threshold);
static void ProcessYPlane_C( unsigned char *source,
				    unsigned char *prev,
				    unsigned char* dest,
				    unsigned char* mask,
				    long  int width, long int height,
				    uint64_t  threshold);

#define EXPAND(x) { x=x+(x<<8)+(x<<16)+(x<<24)+(x<<32)+(x<<40) \
										+(x<<48)+(x<<56);}
static FILTER_PARAM vladParam={2,{"ythresholdMask","cthresholdMask"}};


//*************************************
VF_DEFINE_FILTER(AVDMVideoVlad,vladParam,
                temporalcleaner,
                QT_TR_NOOP("Temporal Cleaner"),
                1,
                VF_NOISE,
                QT_TR_NOOP("Vlad59's Avisynth port of Jim Casaburi's denoiser."));
//*************************************

char *AVDMVideoVlad::printConf(void)
{
	ADM_FILTER_DECLARE_CONF(" Temporal Cleaner : Y: %02lu / c: %02lu",_param->ythresholdMask,
				_param->cthresholdMask	);
        
}


uint8_t AVDMVideoVlad::configure( AVDMGenericVideoStream *instream)
{
UNUSED_ARG(instream);
int i,j;

   diaElemUInteger luma(&(_param->ythresholdMask),QT_TR_NOOP("_Luma temporal threshold:"),0,255);
   diaElemUInteger chroma(&(_param->cthresholdMask),QT_TR_NOOP("Ch_roma temporal threshold:"),0,255);
    
    diaElem *elems[]={&luma,&chroma};
  
    if(diaFactoryRun(QT_TR_NOOP("Temporal Cleaner"),sizeof(elems)/sizeof(diaElem *),elems))
    {
      ythresholdMask = (uint64_t)_param->ythresholdMask;
      cthresholdMask = (uint64_t)_param->cthresholdMask;	   

      EXPAND(	ythresholdMask);
      EXPAND(	cthresholdMask);	
      return 1;
    }
    return 0;
}
AVDMVideoVlad::AVDMVideoVlad(  AVDMGenericVideoStream *in,CONFcouple *couples)
		

{
	_in=in;
	memcpy(&_info,_in->getInfo(),sizeof(_info));
	num_frame=0xFFFF0000;
	if(couples)
	{
		_param=NEW(VLAD_PARAM);
		GET(ythresholdMask);
		GET(cthresholdMask);
	}
	else
	{
		_param=NEW(VLAD_PARAM);
		ADM_assert(_param);
	  _param->ythresholdMask=5;
 	  _param->cthresholdMask=0;
   }
    	_mask=new uint8_t[_info.width*_info.height/4];
  	memset(_mask,0,	_info.width*_info.height/4);
	  
	   ythresholdMask=0;
	   ythresholdMask = (uint64_t)_param->ythresholdMask;
	   cthresholdMask = (uint64_t)_param->cthresholdMask;	   

		EXPAND(	ythresholdMask);
		EXPAND(	cthresholdMask);
	vidCache=new VideoCache(2,in);

	ProcessYPlane = ProcessYPlane_C;
	ProcessCPlane = ProcessCPlane_C;
#ifdef ADM_CPU_X86
#if 0
// Check
	if(CpuCaps::hasMMXEXT() && (_info.width&7) == 0)
	{
		ProcessYPlane = ProcessYPlane_mmxe;
		ProcessCPlane = ProcessCPlane_mmxe;
	}
#endif	
#endif
}


uint8_t	AVDMVideoVlad::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(2);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
	CSET(ythresholdMask);
	CSET(cthresholdMask);
	return 1;

}
AVDMVideoVlad::~AVDMVideoVlad()
{
		delete [] _mask;
		DELETE(_param);
		delete vidCache;
}  									

void ProcessYPlane_C( unsigned char *source,
				    unsigned char *prev,
				    unsigned char* dest,
				    unsigned char* mask, 
				    long  int width, long int height,
				    uint64_t  threshold)
{
	int thp=threshold&0xff;
	int thn=-thp;
	
	width >>= 1;

	for (int y = height >> 1; --y >= 0;)
	{
		int x2 = -width;
		int x = x2*2;

		source += 4*width;
		prev += 4*width;
		dest += 4*width;
		mask += width;

		do {
			int yc,yp;
			int m = 0;
			yc=source[x*2];
			yp=prev[x*2];
			if(thn<=yc-yp && yc-yp<=thp) {
				yc=(yc+yp+1)>>1;
				m=1;
			}
			source[x*2]=dest[x*2]=yc;

			yc=source[x*2+1];
			yp=prev[x*2+1];
			if(thn<=yc-yp && yc-yp<=thp) {
				yc=(yc+yp+1)>>1;
				m++;
			}
			source[x*2+1]=dest[x*2+1]=yc;

			x++;

			yc=source[x2*2];
			yp=prev[x2*2];
			if(thn<=yc-yp && yc-yp<=thp) {
				yc=(yc+yp+1)>>1;
				m++;
			}
			source[x2*2]=dest[x2*2]=yc;

			yc=source[x2*2+1];
			yp=prev[x2*2+1];
			if(thn<=yc-yp && yc-yp<=thp) {
			  yc=(yc+yp+1)>>1;
			  m++;
			}
			source[x2*2+1]=dest[x2*2+1]=yc;

			mask[x2]=m;
		} while(++x2 < 0);
	}
}

void ProcessCPlane_C( unsigned char *source,
				    unsigned char *prev,
				    unsigned char* dest,
				    unsigned char* mask, 
				    int width, int height,
				    uint64_t  threshold)
{
	int thp = threshold&0xff;
	int thn = -thp;
	long int i = width*height;
	
	source += i;
	prev += i;
	dest += i;
	mask += i;
	i = -i;
	
	do {
		int cc,cp;
		cc=source[i];
		cp=prev[i];
		if(thn<=cc-cp && cc-cp<=thp && mask[i]>3) {
			cc=(cc+cp+1)>>1;
		}
		source[i]=dest[i]=cc;
	} while(++i < 0);
}

#ifdef ADM_CPU_X86
void ProcessYPlane_mmxe( unsigned char *source,
				    unsigned char *prev,
				    unsigned char* dest,
				    unsigned char* mask, 
				    long  int width, long int height,
				    uint64_t  threshold)
{
	COUNTER tmp,tmp2;
	long int  h2, w8;

	w8 = -(width >> 3);
	width >>= 1;

__asm__ __volatile__(
			"movq (%0),%%mm6 \n\t"
			:
			: "r"(&threshold)
		);
	
	for (h2 = height >> 1; --h2 >= 0;)
	{
		source += 4*width;
		prev += 4*width;
		dest += 4*width;
		mask += width;

#define REG_source "%2"
#define REG_dest "%3"
#define REG_prev "%4"
#define REG_mask "%5"
#define REG_counter "%0"
#define REG_counter2 "%1"
	  
__asm__ __volatile__(
"prefetchnta ("REG_source","REG_counter",8) \n\t"
"prefetchnta ("REG_prev","REG_counter",8) \n\t"
".p2align 4\n\t"
"pxor %%mm7,%%mm7\n\t"
"HLine%=:  \n\t"

"prefetchnta ("REG_source","REG_counter2",8) \n\t"
"prefetchnta ("REG_prev","REG_counter2",8) \n\t"

"movq ("REG_source","REG_counter",8),%%mm0 \n\t"   // mm0 <- lsource+(size/8)*8
"movq ("REG_prev","REG_counter",8),%%mm1 \n\t"     // mm1 <- lprev+(size/8)*8
"movq %%mm0,%%mm2 \n\t"               // mm2 <- mm0  source
"movq %%mm1,%%mm3 \n\t"               // mm3 <-mm1   oold
"psubusb %%mm1,%%mm0 \n\t"            // mm0=mm0-mm1
"psubusb %%mm2,%%mm1 \n\t"            // mm1=mm1-mm2
"por %%mm1,%%mm0 \n\t"                // mm0=mm0 or mm1
"pavgb %%mm2,%%mm3 \n\t"              // mm3= 'mm2+mm3"/2
" \n\t"                               // mm0=mm6-mm0 diff to threshold
"psubusb %%mm6,%%mm0 \n\t"  // >0 ?
"pcmpeqb %%mm7,%%mm0 \n\t"
" \n\t"
"movq %%mm0,%%mm4 \n\t"               // masked diff >m4
" \n\t"
"pand %%mm0,%%mm3 \n\t"               // mm0=old and mask diff
"pandn %%mm2,%%mm0 \n\t"              // mm1= source and invert diff
"por %%mm3,%%mm0 \n\t"                // m0 = mix
"movq %%mm0,("REG_dest","REG_counter",8) \n\t"   // store to des+ecx*8
"movq %%mm0,("REG_source","REG_counter",8) \n\t" // store to mask+ecx*8

"movq ("REG_source","REG_counter2",8),%%mm0 \n\t"  // mm0 <- lsource+(size/8)*8
"movq ("REG_prev","REG_counter2",8),%%mm1 \n\t"    // mm1 <- lprev+(size/8)*8

"prefetchnta 8("REG_source","REG_counter",8) \n\t"
"prefetchnta 8("REG_prev","REG_counter",8) \n\t"
"add $1,"REG_counter" \n\t"

"movq %%mm0,%%mm2 \n\t"               // mm2 <- mm0  source
"movq %%mm1,%%mm3 \n\t"               // mm3 <- mm1  oold
"psubusb %%mm1,%%mm0 \n\t"            // mm0=mm0-mm1
"psubusb %%mm2,%%mm1 \n\t"            // mm1=mm1-mm2
"por %%mm1,%%mm0 \n\t"                // mm0=mm0 or mm1
"pavgb %%mm2,%%mm3 \n\t"              // mm3= 'mm2+mm3"/2
" \n\t"                               // mm0=mm6-mm0 diff to threshold
"psubusb %%mm6,%%mm0 \n\t"  // >0 ?
"pcmpeqb %%mm7,%%mm0 \n\t"
" \n\t"

"pand %%mm0,%%mm4 \n\t"
"movq %%mm4,%%mm1 \n\t"               // masked diff -> mm1
"psrlw $8,%%mm4 \n\t"                 // shift
"pand %%mm4,%%mm1 \n\t"               // if right & left triggered
"packuswb %%mm1,%%mm1 \n\t"           // packed to 4 bytes
"movd %%mm1,("REG_mask","REG_counter2",4) \n\t"     // store mask m4->mask+ecx*4

" \n\t"
"pand %%mm0,%%mm3 \n\t"               // mm3 = old and mask diff
"pandn %%mm2,%%mm0 \n\t"              // mm0 = source and invert diff
"por %%mm3,%%mm0 \n\t"                // mm0 = mix
"movq %%mm0,("REG_dest","REG_counter2",8) \n\t"   // store to des+ecx*8
"movq %%mm0,("REG_source","REG_counter2",8) \n\t" // store to mask+ecx*8


"add $1,"REG_counter2" \n\t"          // add 1 to ecv
"jnz HLine%="                         // while !=0
 : "=r"(tmp), "=r"(tmp2)
 : "r"(source), "r"(dest), "r"(prev),
   "r"(mask)
   , "0"(2*w8), "1"(w8)
 );

	}
 __asm__ __volatile__("emms \n\t");
}


//#pragma -O0

void ProcessCPlane_mmxe(unsigned char *source,
				   unsigned char *prev, 
				   unsigned char* dest, 
				   unsigned char* mask, 
				   int width, int height,
				   uint64_t  threshold)
{
	long int w8;
	COUNTER tmp;

	w8 = width*height;
	source += w8;
	dest += w8;
	prev += w8;
	mask += w8;
	w8 = -(w8>>3);

__asm__ __volatile__(
			"movq (%0),%%mm6 \n\t"
			:
			: "r"(&threshold)
		);
	
__asm__ __volatile__ (
"prefetchnta ("REG_source","REG_counter",8) \n\t"
"prefetchnta ("REG_prev","REG_counter",8) \n\t"
"prefetchnta ("REG_mask","REG_counter",8) \n\t"
".p2align 4\n\t"
"pxor %%mm7,%%mm7\n\t"
"Lfoo%=:  \n\t"
"prefetchnta 8("REG_source","REG_counter",8) \n\t"
"prefetchnta 8("REG_prev","REG_counter",8) \n\t"
"prefetchnta 8("REG_mask","REG_counter",8) \n\t"
"movq ("REG_source","REG_counter",8),%%mm0 \n\t"
"movq ("REG_prev","REG_counter",8),%%mm1 \n\t"
"movq %%mm0,%%mm2 \n\t"
"movq %%mm1,%%mm3 \n\t"
"psubusb %%mm1,%%mm0 \n\t"
"psubusb %%mm2,%%mm1 \n\t"
"por %%mm1,%%mm0 \n\t"
"pavgb %%mm2,%%mm3 \n\t"
" \n\t"
"psubusb %%mm6,%%mm0 \n\t"
"pcmpeqb %%mm7,%%mm0 \n\t"
" \n\t"
"pand ("REG_mask","REG_counter",8),%%mm0 \n\t"
" \n\t"
"pand %%mm0,%%mm3 \n\t"
"pandn %%mm2,%%mm0 \n\t"
"por %%mm3,%%mm0 \n\t"
"movq %%mm0,("REG_dest","REG_counter",8) \n\t"
"movq %%mm0,("REG_source","REG_counter",8) \n\t"
"add $1,"REG_counter" \n\t"
"jnz Lfoo%= \n\t"
 : "=r"(tmp)
 : "0"(w8), "r"(source), "r"(dest), "r"(prev),
   "r"(mask)
 
 );
	__asm__ __volatile__("emms \n\t");
}
#endif

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
uint8_t AVDMVideoVlad::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
	
	uint32_t page=_info.width*_info.height;
	ADMImage *cur,*prev;
	
		if(frame>(_info.nb_frames-1)) return 0;
		
		*len=(page*3)>>1;
		
		if(frame)
		{
		 	prev=vidCache->getImage(frame-1);
			if(!prev)
				return 0;
		}
		
		cur=vidCache->getImage(frame);
		if(!cur)
		{
			vidCache->unlockAll();
			return 0;
		}
		data->copyInfo(cur);
		if(!frame)
		{
			
			data->duplicate(cur);
			vidCache->unlockAll();
			return 1  ;
		}
		
		
			  
		ProcessYPlane (YPLANE(cur),
				YPLANE(prev),     	
				YPLANE(data), 
				_mask, 
				_info.width, 
		       		_info.height,
				ythresholdMask);
		if (0==_param->cthresholdMask)
		{
			//memcpy(data->data+page,_uncompressed->data+page,page>>1);
			memcpy(UPLANE(data),UPLANE(cur),page>>2);
			memcpy(VPLANE(data),VPLANE(cur),page>>2);
		}
		else
		{
			
				ProcessCPlane (UPLANE(cur),
							UPLANE(prev),     	
							UPLANE(data), 
							_mask, 
							_info.width>>1, 
							_info.height>>1,
							cthresholdMask);       
				
			
				ProcessCPlane (VPLANE(cur),
							VPLANE(prev),     	
							VPLANE(data), 
							_mask, 
							_info.width>>1, 
							_info.height>>1,
							cthresholdMask);       				
			
			}
		
		vidCache->unlockAll();
		return 1;

}


