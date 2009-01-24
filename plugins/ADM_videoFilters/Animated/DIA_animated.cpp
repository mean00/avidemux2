//
//
// C++ Implementation: DIA_SVCD
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//




# include <math.h>

#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "ADM_videoFilterDynamic.h"

#include "DIA_factory.h"

#include "ADM_vidAnimated_param.h"
 
#include "DIA_factory.h"

/**
      \fn DIA_animated
      \brief Dialog to set params for the animated filter
*/
uint8_t DIA_animated(ANIMATED_PARAM *param,uint32_t w, uint32_t h,uint32_t n)
{	
  uint8_t r=0;
#define PX(x) &(param->x)
   diaElemFile      jpeg(0,(char **)PX(backgroundImg),QT_TR_NOOP("_Background Image:"), NULL, QT_TR_NOOP("Select background image"));
   diaElemToggle    isNtsc(PX(isNTSC),QT_TR_NOOP("_NTSC(default is Pal):"));
   diaElemUInteger   vignetteW(PX(vignetteW),QT_TR_NOOP("Vignette _Width:"),16,w/3);
   diaElemUInteger   vignetteH(PX(vignetteH),QT_TR_NOOP("Vignette _Height:"),16,h/2);
   
   diaElemUInteger *timecode[MAX_VIGNETTE];
   diaElemFrame timecodes(QT_TR_NOOP("Vignette frame number"));

   for(int i=0;i<MAX_VIGNETTE;i++)
   {
     timecode[i]=new diaElemUInteger(&(param->timecode[i]),QT_TR_NOOP("Timecode:"),0,n);
     timecodes.swallow(timecode[i]);
   }

 
      diaElem *elems[5]={&jpeg,&isNtsc,&vignetteW,&vignetteH,&timecodes};
    
  if( diaFactoryRun(QT_TR_NOOP("MPEG-2 Configuration"),5,elems))
  {
    r=1;
  }
  for(int i=0;i<MAX_VIGNETTE;i++)
  {
    delete timecode[i]; 
  }
  
  return r;
}	
// EOF
