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

#include "ADM_default.h"
#include "ADM_vidMisc.h"
#include "DIA_factory.h"
#include "avi_vars.h"
#include "config.h"
/**
 * \fn DIA_gotoTime
 * \brief Popup a display to enter hour/minutes/seconds/ms
 * @param hh
 * @param mm
 * @param ss
 * @param ms
 * @return
 */
uint8_t DIA_gotoTime(uint32_t *hh, uint32_t *mm, uint32_t *ss, uint32_t *ms)
{
    uint32_t v = (*hh) * 3600 * 1000 + (*mm) * 60 * 1000 + (*ss) * 1000 + *ms;
    uint32_t max = (uint32_t)(video_body->getVideoDuration() / 1000);

    diaElemTimeStamp eh(&v, QT_TRANSLATE_NOOP("adm", "TimeStamp:"), 0, max);
    diaElem *allWidgets[] = {&eh};

    if (!diaFactoryRun(QT_TRANSLATE_NOOP("adm", "Go to Time"), 1, allWidgets))
        return 0;

    //
    ms2time(v, hh, mm, ss, ms);
    return 1;
}
// EOF
