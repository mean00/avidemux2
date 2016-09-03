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
#include "ADM_toolkitGtk.h"
#include "ADM_image.h"
#include "DIA_flyDialog.h"
#include "../chromashift.h"
#include "DIA_factory.h"
#include "../DIA_flyChromaShift.h"

uint8_t DIA_getChromaShift( ADM_coreVideoFilter *instream,chromashift    *param )
{
    int w=instream->getInfo()->width/2;
    diaElemInteger  u(&(param->u),QT_TR_NOOP("U :"),-w,w);
    diaElemInteger  v(&(param->v),QT_TR_NOOP("V :"),-w,w);
    diaElem *elems[]={&u,&v};
    return diaFactoryRun("ChromaShift",sizeof(elems)/sizeof(diaElem *),elems);
}
uint8_t    flyChromaShift::download(void) {return 1;}
uint8_t    flyChromaShift::upload(void) {return 1;}