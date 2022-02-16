//
// Author: szlldm, (C) 2021
//
// Copyright: See COPYING file that comes with this distribution
//
//

//#include "xonfig.h"
#include "ADM_default.h"


#include "DIA_factory.h"

int DIA_getHDRParams( uint32_t * toneMappingMethod, float * saturationAdjust, float * boostAdjust, bool * adaptiveRGB, uint32_t * gamutMethod)
{
#ifdef EXPLAIN
#   define NB_ELEMS 6
    diaElemReadOnlyText applicability(NULL,QT_TRANSLATE_NOOP("adm","Changing the options above will results the editor jumping to the nearest prior key frame."),NULL);
#else
#   define NB_ELEMS 5
#endif
    diaMenuEntry toneMapEntries[]={
                          {0,       QT_TRANSLATE_NOOP("adm","Disabled"),NULL}
                         ,{1,       QT_TRANSLATE_NOOP("adm","Fast YUV"),NULL}
                         ,{2,       QT_TRANSLATE_NOOP("adm","RGB clipping"),NULL}
                         ,{3,       QT_TRANSLATE_NOOP("adm","RGB Reinhard"),NULL}
                         ,{4,       QT_TRANSLATE_NOOP("adm","RGB Hable"),NULL}
                         //,{2,      QT_TRANSLATE_NOOP("adm","TODO"),NULL}
    };

    diaElemMenu menuToneMapHDR(toneMappingMethod,QT_TRANSLATE_NOOP("adm","_Tone mapping:"),sizeof(toneMapEntries)/sizeof(diaMenuEntry),toneMapEntries);
    diaElemFloatResettable floatSaturationHDR(saturationAdjust,QT_TRANSLATE_NOOP("adm","_Saturation:"),0.,10.,1.);
    diaElemFloatResettable floatBoostHDR(boostAdjust,QT_TRANSLATE_NOOP("adm","_Boost (level multiplier):"),0.,10.,1.);
    diaElemToggle adaptive(adaptiveRGB,QT_TRANSLATE_NOOP("adm","_Adaptive RGB tonemappers"));

    diaMenuEntry gamutMapEntries[]={
                          {0,       QT_TRANSLATE_NOOP("adm","Clipping"),NULL}
                         ,{1,       QT_TRANSLATE_NOOP("adm","Compression"),NULL}
    };
    diaElemMenu menuGamutMapHDR(gamutMethod,QT_TRANSLATE_NOOP("adm","_RGB out of gamut handling:"),sizeof(gamutMapEntries)/sizeof(diaMenuEntry),gamutMapEntries);
    

    for(int i=0; i < sizeof(toneMapEntries)/sizeof(diaMenuEntry); i++)
    {
        menuToneMapHDR.link(&(toneMapEntries[i]), i, &floatSaturationHDR);
        menuToneMapHDR.link(&(toneMapEntries[i]), i, &floatBoostHDR);
        menuToneMapHDR.link(&(toneMapEntries[i]), i, &adaptive);
        menuToneMapHDR.link(&(toneMapEntries[i]), i, &menuGamutMapHDR);
    }

    diaElem *elems[NB_ELEMS]={
         &menuToneMapHDR
        ,&floatSaturationHDR
        ,&floatBoostHDR
        ,&adaptive
        ,&menuGamutMapHDR
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
