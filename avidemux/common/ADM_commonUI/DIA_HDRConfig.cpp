//
// Author: szlldm, (C) 2021
//
// Copyright: See COPYING file that comes with this distribution
//
//

//#include "xonfig.h"
#include "ADM_default.h"


#include "DIA_factory.h"

int DIA_getHDRParams( uint32_t * toneMappingMethod, float * saturationAdjust, float * boostAdjust)
{
#ifdef EXPLAIN
#   define NB_ELEMS 4
    diaElemReadOnlyText applicability(NULL,QT_TRANSLATE_NOOP("adm","Changing the options above will results the editor jumping to the nearest prior key frame."),NULL);
#else
#   define NB_ELEMS 3
#endif
    diaMenuEntry toneMapEntries[]={
                          {0,       QT_TRANSLATE_NOOP("adm","Disabled"),NULL}
                         ,{1,       QT_TRANSLATE_NOOP("adm","Fast YUV"),NULL}
                         ,{2,       QT_TRANSLATE_NOOP("adm","RGB clipping"),NULL}
                         ,{3,       QT_TRANSLATE_NOOP("adm","RGB Reinhard"),NULL}
                         ,{4,       QT_TRANSLATE_NOOP("adm","RGB Hable"),NULL}
                         ,{5,       QT_TRANSLATE_NOOP("adm","RGB clipping - adaptive"),NULL}
                         ,{6,       QT_TRANSLATE_NOOP("adm","RGB Reinhard - adaptive"),NULL}
                         ,{7,       QT_TRANSLATE_NOOP("adm","RGB Hable - adaptive"),NULL}
                         //,{2,      QT_TRANSLATE_NOOP("adm","TODO"),NULL}
    };

    diaElemMenu menuToneMapHDR(toneMappingMethod,QT_TRANSLATE_NOOP("adm","_Tone mapping:"),sizeof(toneMapEntries)/sizeof(diaMenuEntry),toneMapEntries);
    diaElemFloatResettable floatSaturationHDR(saturationAdjust,QT_TRANSLATE_NOOP("adm","_Saturation:"),0.,10.,1.);
    diaElemFloatResettable floatBoostHDR(boostAdjust,QT_TRANSLATE_NOOP("adm","_Boost (level multiplier):"),0.,10.,1.);

    for(int i=0; i < sizeof(toneMapEntries)/sizeof(diaMenuEntry); i++)
    {
        menuToneMapHDR.link(&(toneMapEntries[i]), i, &floatSaturationHDR);
        menuToneMapHDR.link(&(toneMapEntries[i]), i, &floatBoostHDR);
    }

    diaElem *elems[NB_ELEMS]={
         &menuToneMapHDR
        ,&floatSaturationHDR
        ,&floatBoostHDR
#ifdef EXPLAIN
        ,&applicability
#endif
    };

    if(diaFactoryRun("HDR tone mapping",NB_ELEMS,elems))
    {
        return 1;
    }
    return 0;
}
