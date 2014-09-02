/** *************************************************************************
    \file  ADM_prettyPrint
    \brief Convert a duration in a human readable format
                      
    copyright            : (C) 2009 by mean
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#pragma once
#include <string>

#include "ADM_core6_export.h"
/// Convert a duration in ms into a human friendly string
ADM_CORE6_EXPORT bool ADM_durationToString(uint32_t durationInMs, std::string &outputString);