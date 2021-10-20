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
    diaElemReadOnlyText applicability(NULL,QT_TRANSLATE_NOOP("adm","The options above are not immediately effective on cached and displayed frames"),NULL);
    
    diaMenuEntry toneMapEntries[]={
                          {0,       QT_TRANSLATE_NOOP("adm","disabled"),NULL}
                         ,{1,       QT_TRANSLATE_NOOP("adm","Fast YUV"),NULL}
                         ,{2,       QT_TRANSLATE_NOOP("adm","RGB clipping"),NULL}
                         ,{3,       QT_TRANSLATE_NOOP("adm","RGB Reinhard"),NULL}
                         ,{4,       QT_TRANSLATE_NOOP("adm","RGB Hable"),NULL}
                         //,{2,      QT_TRANSLATE_NOOP("adm","TODO"),NULL}
    };

    diaElemMenu menuToneMapHDR(toneMappingMethod,QT_TRANSLATE_NOOP("adm","_Tone mapping:"),sizeof(toneMapEntries)/sizeof(diaMenuEntry),toneMapEntries);
    diaElemFloat floatSaturationHDR(saturationAdjust,QT_TRANSLATE_NOOP("adm","_Saturation:"),0.,10.);
    diaElemFloat floatBoostHDR(boostAdjust,QT_TRANSLATE_NOOP("adm","_Boost (level multiplier):"),0.,10.);

    diaElem *elems[6]={ &menuToneMapHDR, &floatSaturationHDR, &floatBoostHDR, &applicability };

    if(diaFactoryRun("HDR tone mapping",4,elems))
    {
        return 1;
    }
    return 0;
}
