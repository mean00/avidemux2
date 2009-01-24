//
// C++ Implementation: ADM_vidForcedPP
//
// Description: 
//
//	Force postprocessing assuming constant quant & image type
//	Uselefull on some badly authored DVD for example
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <math.h>

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "DIA_factory.h"
#include "ADM_vidTDeint_param.h"
/**
      \fn DIA_tdeint
      \brief Dialog for tdeint filter
*/

uint8_t  DIA_tdeint(TDEINT_PARAM *param)
{
         
         
diaMenuEntry meField[]={
  {0,QT_TR_NOOP("Auto (might not work)")},
  {2,QT_TR_NOOP("Top field first")},
  {1,QT_TR_NOOP("Bottom field first")}
};       

diaMenuEntry meInterpolate[]={
  {0,QT_TR_NOOP("Auto (might not work)")},
  {2,QT_TR_NOOP("Bottom field (keep top)")},
  {1,QT_TR_NOOP("Top field (keep bottom)")}
};       


diaMenuEntry meType[]={
  {0,QT_TR_NOOP("Cubic interpolation")},
  {1,QT_TR_NOOP("Modified ELA")},
  {2,QT_TR_NOOP("Kernel interpolation")},
  {3,QT_TR_NOOP("Modified ELA-2")},
  
};       

diaMenuEntry meMnt[]={
  {0,QT_TR_NOOP("4 fields check")},
  {1,QT_TR_NOOP("5 fields check")},
  {2,QT_TR_NOOP("4 fields check (no avg)")},
  {3,QT_TR_NOOP("5 fields check (no avg)")},
  
};       


diaMenuEntry meLink[]={
  {0,QT_TR_NOOP("No link")},
  {1,QT_TR_NOOP("Full link")},
  {2,QT_TR_NOOP("Y to UV")},
  {3,QT_TR_NOOP("UV to Y")}
};     

diaMenuEntry meAP[]={
  {0,QT_TR_NOOP("0")},
  {1,QT_TR_NOOP("1")},
  {2,QT_TR_NOOP("2")}
};     
#define PX(x) &(param->x)
      uint32_t order=param->order+1;
      uint32_t field=param->field+1;
      
      
      diaElemMenu     menuFieldOrder(&(order),QT_TR_NOOP("_Field order:"),3,meField);
      diaElemMenu     menuInterpolaye(&(field),QT_TR_NOOP("_Interpolate:"),3,meInterpolate);
      
      diaElemMenu     menuType(PX(type),QT_TR_NOOP("_Type:"),4,meType);
      diaElemMenu     menuMnt(PX(mtnmode),QT_TR_NOOP("_MntMode:"),4,meMnt);
      diaElemMenu     menuLink(PX(link),QT_TR_NOOP("_Link:"),4,meLink);
      diaElemMenu     menuAP(PX(APType),QT_TR_NOOP("_AP type:"),3,meAP);

      // Toggle
      diaElemToggle    toggleUseChroma(PX(chroma),QT_TR_NOOP("Use ch_roma to evalute"));
      diaElemToggle    toggleTryWeave(PX(tryWeave),QT_TR_NOOP("Try _weave"));
      diaElemToggle    toggleDenoise(PX(denoise),QT_TR_NOOP("_Denoise"));
      diaElemToggle    toggleSharp(PX(sharp),QT_TR_NOOP("_Sharp"));
      diaElemToggle    toggleEvaluteAll(PX(full),QT_TR_NOOP("_Evalute all frames"));
      
      // int
      diaElemUInteger  intMotionLuma(PX(mthreshL),QT_TR_NOOP("Motion threshold, l_uma:"),0,255);
      diaElemUInteger  intMotionChroma(PX(mthreshC),QT_TR_NOOP("Motion threshold, c_hroma:"),0,255);
      diaElemUInteger  intAreaCombing(PX(cthresh),QT_TR_NOOP("Area com_bing threshold:"),0,255);
      diaElemUInteger  intCombed(PX(MI),QT_TR_NOOP("Combe_d threshold:"),0,255);
      diaElemInteger   intArtefact(PX(AP),QT_TR_NOOP("Artefact _protection threshold:"),-1,255);
      diaElemToggle    intDebug(PX(debug),QT_TR_NOOP("Debug:"));

         diaElem *diaRC[]={&menuFieldOrder,&menuInterpolaye,&menuType,&menuMnt,&menuLink,&menuAP,
                            &toggleUseChroma,&toggleTryWeave,&toggleDenoise,&toggleSharp,&toggleEvaluteAll,
                            &intMotionLuma,&intMotionChroma,&intAreaCombing,&intCombed,&intArtefact,&intDebug
                  };

        
         
        if( diaFactoryRun(QT_TR_NOOP("TDeint"),17,diaRC))
	{
           param->order=(int)order-1;
           param->field=(int)field-1;
          return 1;
        }
         return 0;
}
// EOF
