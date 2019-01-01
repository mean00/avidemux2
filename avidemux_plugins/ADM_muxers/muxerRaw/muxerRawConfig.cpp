/***************************************************************************
    copyright            : (C) 2007 by mean
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
#include "ADM_default.h"
#include "raw_muxer.h"
#include "DIA_factory.h"

raw_muxer muxerConfig={0};

/**
    \fn rawConfigure
*/
bool rawConfigure(void)
{
    bool annexb=(bool)muxerConfig.requestAnnexB;

    diaElemToggle chkbox(&annexb,QT_TRANSLATE_NOOP("rawmuxer","Prefer Annex B type stream"));

    diaElem *tab[]={&chkbox};
    if(diaFactoryRun(QT_TRANSLATE_NOOP("rawmuxer","Video Only Muxer Settings"),1,tab))
    {
        muxerConfig.requestAnnexB=annexb;
        return true;
    }
    return false;
}
// EOF
