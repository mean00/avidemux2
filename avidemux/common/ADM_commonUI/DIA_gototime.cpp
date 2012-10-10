/**
 * \file DIA_gototime.cpp
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"
#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_vidMisc.h"
/**
 * \fn DIA_gotoTime
 * \brief Popup a display to enter hour/minutes/seconds/ms
 * @param hh
 * @param mm
 * @param ss
 * @param ms
 * @return 
 */
uint8_t DIA_gotoTime(uint32_t *hh, uint32_t *mm, uint32_t *ss,uint32_t *ms)
{
uint32_t v=(*hh)*3600*1000+(*mm)*60*1000+(*ss)*1000+*ms;

diaElemTimeStamp   eh(&v,QT_TR_NOOP("TimeStamp:"),0,24);
diaElem *allWidgets[]={&eh};

  if(!diaFactoryRun(QT_TR_NOOP("Go to Time"),1,allWidgets)) return 0;

//
ms2time(v,hh,mm,ss,ms);
return 1;

}
// EOF
