/**
    \file ADM_pp
    \brief Wrapper around libavcodec libpostproc
    \author mean fixounet@free.fr 2004/2010
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//	PostProc : 1 Horiz deblock
//             2 Verti deblock
//             4 Dering
// strength between 0 and 5

#include "ADM_includeFfmpeg.h"
#include "ADM_lavcodec.h"
extern "C" {
#include "ADM_ffmpeg/libpostproc/postprocess.h"
}
#include "ADM_default.h"
#include "ADM_image.h"
#include "ADM_imageFlags.h"
#include "ADM_pp.h"


#define aprintf ADM_info
/**
    \fn ctor
*/
ADM_PP::ADM_PP(uint32_t width, uint32_t height)
{
    memset(this,0,sizeof(*this));
	this->w=width;
	this->h=height;
    swapuv=0;
	aprintf("Initializing postproc\n");
}
/**
    \fn cleanup
*/
bool ADM_PP::cleanup(void)
{
	 aprintf("Deleting post proc\n");
	 if(ppMode) {pp_free_mode(ppMode);ppMode=NULL;}
	 if(ppContext) {pp_free_context(ppContext);ppContext=NULL;}
     return true;
}

/**
    \fn dtor
*/
ADM_PP::~ADM_PP()
{
	 cleanup();
}
/**
    \fn updatePostProc
*/
bool ADM_PP::update(void)
{
char stringMode[60];
char stringFQ[60];

	stringMode[0]=0;
	cleanup();
	aprintf("updating post proc\n");

	if(postProcType&1) strcat(stringMode,"ha:a:128:7,");
	if(postProcType&2) strcat(stringMode,"va:a:128:7,");
	if(postProcType&4) strcat(stringMode,"dr:a,");
	if(forcedQuant)  
		{
			sprintf(stringFQ,"fq:%d,",forcedQuant);
			strcat(stringMode,stringFQ);
		}
			
	if(strlen(stringMode))  // something to do ?
		{
		uint32_t ppCaps=0;
		
#ifdef ADM_CPU_X86
		
	#define ADD(x,y) if( CpuCaps::has##x()) ppCaps|=PP_CPU_CAPS_##y;
		
		ADD(MMX,MMX);		
		ADD(3DNOW,3DNOW);
		ADD(MMXEXT,MMX2);
#endif		
#ifdef ADM_CPU_ALTIVEC
		ppCaps|=PP_CPU_CAPS_ALTIVEC;
#endif	
			ppContext=pp_get_context(w, h, ppCaps  );		
			ppMode=pp_get_mode_by_name_and_quality(
			stringMode, postProcStrength);;
			ADM_assert(ppMode);
			aprintf("Enabled type:%d strength:%d\n",postProcType,postProcStrength);
		}	   
	else    // if nothing is selected we may as well set back every thing to 0
		{
			postProcStrength=0;
			aprintf("Disabled\n");
		}
    return false;
}
/**
    \fn process
*/
bool        ADM_PP::process(class ADMImage *src, class ADMImage *dest)
{
int type;

uint32_t ww,hh;
uint32_t border;

   // return dest->duplicate(src);

    border=w&(7);
    ww=w-border;
    hh=h&(~1);
    
    ADM_assert(src);
    ADM_assert(dest);

    ADM_assert(ppMode);
    ADM_assert(ppContext);

	#warning FIXME should be FF_I_TYPE/B/P
	if(src->flags & AVI_KEY_FRAME) type=1;
		else if(src->flags & AVI_B_FRAME) type=3;
			else type=2;

    ADM_assert(src->_colorspace==ADM_COLOR_YV12);

	// we do postproc !
	// keep
	uint8_t       *oBuff[3];
    const uint8_t *xBuff[3];
	uint8_t       *iBuff[3];
	uint32_t	  strideTab[3];
	uint32_t	  strideTab2[3];
    int           iStrideTab2[3],iStrideTab[3];

        src->GetReadPlanes(iBuff);
        src->GetPitches(strideTab);
        dest->GetPitches(strideTab2);
        dest->GetWritePlanes(oBuff);
        if(swapuv)
        {
                uint8_t *s=oBuff[1];
                oBuff[1]=oBuff[2];
                oBuff[2]=s;
        }
        
        for(int i=0;i<3;i++) 
        {
            iStrideTab[i]=strideTab[i];
            iStrideTab2[i]=strideTab2[i];
            xBuff[i]=iBuff[i];
        }
        pp_postprocess(
            xBuff,
            iStrideTab,
            oBuff,
            iStrideTab2,
            ww,
            hh,
            (int8_t *)(src->quant),
            src->_qStride,
            ppMode,
            ppContext,
            type);			// img type

  /*
                If there is a chroma block that needs padding
                (width not multiple of 16) while postprocessing,
                we process up to the nearest 16 multiple and
                just copy luma & chroma info that was left over
        */
        if(border)
        {
              
                uint8_t *src,*dst;
                uint32_t stridein,strideout,right;

                right=ww;
                // Luma
                dst=oBuff[0]+right;
                src=(uint8_t *)(xBuff[0]+right);
              
                for(int y=h;y>0;y--)
                {
                        memcpy(dst,src,border);
                        dst+=strideTab2[0];
                        src+=strideTab[0];
                }
                // Chroma
                border>>=1;
                right>>=1;
                dst=oBuff[1]+right;
                src=(uint8_t *)(xBuff[1]+right);
                //
                for(int y=h>>1;y>0;y--)
                {
                        memcpy(dst,src,border);
                        dst+=strideTab2[1];
                        src+=strideTab[1];
                }
                //
                dst=oBuff[2]+right;
                src=(uint8_t *)(xBuff[2]+right);
                //
                for(int y=h>>1;y>0;y--)
                {
                        memcpy(dst,src,border);
                        dst+=strideTab2[2];
                        src+=strideTab[2];
                }
        }
        return true;
}
//EOF
