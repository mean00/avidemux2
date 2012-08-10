/** *************************************************************************
    \file  ADM_string
    \brief String related utilities
                      
    copyright            : (C) 2009/2010 by mean
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef ADM_STRING_H
#define ADM_STRING_H

#include "ADM_coreUtils6_export.h"
#include "ADM_image.h"

ADM_COREUTILS6_EXPORT bool        ADM_splitString(const string &separator, const string &source, vector<string>& result);
#endif
//EOF
