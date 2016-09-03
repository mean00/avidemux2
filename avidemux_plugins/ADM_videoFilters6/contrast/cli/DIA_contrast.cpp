/***************************************************************************
                          ADM_guiChromaShift.cpp  -  description
                             -------------------
    begin                : Sun Aug 24 2003
    copyright            : (C) 2002-2003 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_image.h"
#include "DIA_flyDialog.h"
#include "../contrast.h"
#include "DIA_factory.h"
#include "../DIA_flyContrast.h"

uint8_t DIA_getContrast( ADM_coreVideoFilter *instream,contrast    *param )
{
    diaElemToggle  u(&(param->doChromaU),QT_TRANSLATE_NOOP("contrast","U :"));
    diaElemToggle  v(&(param->doChromaV),QT_TRANSLATE_NOOP("contrast","V :"));
    diaElemToggle  l(&(param->doLuma),QT_TRANSLATE_NOOP("contrast","L :"));

    diaElemInteger  o(&(param->offset),QT_TRANSLATE_NOOP("contrast","Offset :"),-127,127);
    diaElemFloat    c(&(param->coef),QT_TRANSLATE_NOOP("contrast","Coef :"),-10,10);
    diaElem *elems[]={&c,&o,&l,&u,&v};
    return diaFactoryRun("Contrast",sizeof(elems)/sizeof(diaElem *),elems);
}
uint8_t    flyContrast::download(void) {return 1;}
uint8_t    flyContrast::upload(void) {return 1;}
