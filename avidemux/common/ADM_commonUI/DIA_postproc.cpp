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

//#include "xonfig.h"
#include "ADM_default.h"
#include "ADM_pp.h"

#include "DIA_factory.h"

int DIA_getMPParams( uint32_t *pplevel, uint32_t *ppstrength,bool *swap)
{
#define PX(x) x
    diaElemReadOnlyText applicability(NULL,QT_TRANSLATE_NOOP("adm","The options above are effective only for software decoding"),NULL);
    diaElemUInteger   postProcStrength(PX(ppstrength),QT_TRANSLATE_NOOP("adm","_Filter strength:"),0,5);
    //diaElemToggle     swapuv(PX(swap),QT_TRANSLATE_NOOP("adm","_Swap U and V"));

    bool hzd,vzd,dring,deint;

#define DOME(x,y) if(*pplevel & x) y=1;else y=0;

    DOME(ADM_POSTPROC_HORIZ_DEBLOCK,hzd)
    DOME(ADM_POSTPROC_VERT_DEBLOCK,vzd)
    DOME(ADM_POSTPROC_DERING,dring)
    DOME(ADM_POSTPROC_DEINT,deint)

    diaElemToggle fhzd(&hzd,QT_TRANSLATE_NOOP("adm","_Horizontal deblocking"));
    diaElemToggle fvzd(&vzd,QT_TRANSLATE_NOOP("adm","_Vertical deblocking"));
    diaElemToggle fdring(&dring,QT_TRANSLATE_NOOP("adm","_Deringing"));
    diaElemToggle fdeint(&deint,QT_TRANSLATE_NOOP("adm","De_interlacing (ffmpegdeint)"));

    diaElemFrame frameDeblock(QT_TRANSLATE_NOOP("adm","Deblocking"));
    frameDeblock.swallow(&postProcStrength);
    frameDeblock.swallow(&fhzd);
    frameDeblock.swallow(&fvzd);
    frameDeblock.swallow(&fdring);

    diaElem *elems[3]={&frameDeblock , &fdeint, &applicability };

    if(diaFactoryRun("Postprocessing",3,elems))
    {
#undef DOME
#define DOME(x,y) if(y) *pplevel |=x;
        *pplevel=0;
        DOME(ADM_POSTPROC_HORIZ_DEBLOCK,hzd)
        DOME(ADM_POSTPROC_VERT_DEBLOCK,vzd)
        DOME(ADM_POSTPROC_DERING,dring)
        DOME(ADM_POSTPROC_DEINT,deint)
        return 1;
    }
    return 0;
}
