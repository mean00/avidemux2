/** *************************************************************************
     \file                     ADM_edMarker.cpp  
     \brief  Handle Marker

    copyright            : (C) 2002/2009 by mean
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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_vidMisc.h"
#include "fourcc.h"
#include "ADM_edit.hxx"

/**
        \fn getMarkerAPts
*/
uint64_t    ADM_Composer::getMarkerAPts()
{
    return markerAPts;
}
/**
        \fn getMarkerBPts
*/

uint64_t    ADM_Composer::getMarkerBPts()
{
    return markerBPts;
}
/**
        \fn setMarkerAPts
*/

bool        ADM_Composer::setMarkerAPts(uint64_t pts)
{
    if(pts!=markerAPts)
    {
        uint64_t max = getVideoDuration();
        if(pts > max)
        {
            ADM_warning("Marker A value %" PRIu64" out of bounds, correcting.\n",pts);
            pts = max;
        }
        markerAPts=pts;
        ADM_info("Selection's start point set to %s (%" PRIu64" us)\n",ADM_us2plain(markerAPts),markerAPts);
    }
    return true;
}
/**
        \fn setMarkerBPts
*/

bool        ADM_Composer::setMarkerBPts(uint64_t pts)
{
    if(pts!=markerBPts)
    {
        uint64_t max = getVideoDuration();
        if(pts > max)
        {
            ADM_warning("Marker B value %" PRIu64" out of bounds, correcting.\n",pts);
            pts = max;
        }
        markerBPts=pts;
        ADM_info("Selection's end point set to %s (%" PRIu64" us)\n",ADM_us2plain(markerBPts),markerBPts);
    }
    return true;
}
//EOF
