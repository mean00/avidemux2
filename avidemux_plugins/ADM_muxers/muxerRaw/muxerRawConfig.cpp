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
#include "muxerRaw.h"

raw_muxer muxerConfig = {0, 0, 4, 0};

/**
    \fn rawConfigure
*/
bool rawConfigure(void)
{
    diaMenuEntry extensions[] = {
        {EXT_DEFAULT, "Default (raw)", NULL},
        {EXT_BIN,     "bin",           NULL},
        {EXT_JPEG,    "jpg",           NULL}
    };

    bool annexb=(bool)muxerConfig.requestAnnexB;
    bool sep = (bool)muxerConfig.separateFiles;

#define PX(x) &(muxerConfig.x)
#define MZ(x) (sizeof(x) / sizeof(diaMenuEntry))

    diaElemToggle chkboxAnnexB(&annexb, QT_TRANSLATE_NOOP("rawmuxer", "Prefer Annex B type stream"));
    diaElemToggle chkboxSepFiles(&sep, QT_TRANSLATE_NOOP("rawmuxer", "Save frames to separate files"));
    diaElemUInteger maxDigDial(PX(maxDigits), QT_TRANSLATE_NOOP("rawmuxer", "Maximum number of digits"), 2, 6);
    diaElemMenu menuExtension(PX(extIdx), QT_TRANSLATE_NOOP("rawmuxer", "Override filename extension"), MZ(extensions), extensions);

    chkboxSepFiles.link(1, &maxDigDial);
    chkboxSepFiles.link(1, &menuExtension);

    diaElem *tab[] = {&chkboxAnnexB, &chkboxSepFiles, &maxDigDial, &menuExtension};

#undef MZ
#define MZ(x) sizeof(x)/sizeof(diaElem *)

    if(diaFactoryRun(QT_TRANSLATE_NOOP("rawmuxer","Video Only Muxer Settings"), MZ(tab), tab))
    {
        muxerConfig.requestAnnexB=annexb;
        muxerConfig.separateFiles=sep;
        return true;
    }
    return false;
}
// EOF
