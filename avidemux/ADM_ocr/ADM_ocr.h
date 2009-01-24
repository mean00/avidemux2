/***************************************************************************
                         
     External Interface for OCR engine
     
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
#ifndef ADM_OCR_H
#define ADM_OCR_H
#include "ADM_videoFilter/ADM_vobsubinfo.h"
#include "ADM_videoFilter/ADM_vidVobSub.h"
#include "ADM_ocr/adm_glyph.h"
typedef enum
{
	ADM_OCR_TYPE_VOBSUB=1,
	ADM_OCR_TYPE_TS=2,
}ADM_OCR_SOURCE_TYPE;


typedef struct
{
	ADM_OCR_SOURCE_TYPE type;
	vobSubParam *subparam;
	char		*TsFile;
	uint32_t    TsPid;
}ADM_OCR_SOURCE;


uint8_t ADM_ocr_engine(   ADM_OCR_SOURCE & source,const char *labelSrt,admGlyph *head);


#endif 
