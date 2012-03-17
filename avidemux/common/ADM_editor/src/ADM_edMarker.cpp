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
#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"

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
        markerAPts=pts;;
        return true;
}
/**
        \fn setMarkerBPts
*/

bool        ADM_Composer::setMarkerBPts(uint64_t pts)
{
        markerBPts=pts;;
        return true;
}
//EOF
