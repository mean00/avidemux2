/***************************************************************************
    \file             : ADM_coreDXVA2.cpp
    \brief            : Wrapper around DXVA2 functions
    \author           : (C) 2016 by mean fixounet@free.fr
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
#include "../include/ADM_coreDxva2.h"
#ifdef USE_DXVA2
#include "../include/ADM_coreDxva2Internal.h"
#include "ADM_dynamicLoading.h"
#include <map>
static bool                  coreDxva2Working=false;
bool admDxva2::init(GUI_WindowInfo *x)
{
          return false;
}
  
/**
    \fn isOperationnal
*/
bool admDxva2::isOperationnal(void)
{
    ADM_warning("This binary has no DXVA2 support\n");
    return coreDxva2Working;
}
bool admDxva2::cleanup(void)
{
    ADM_warning("This binary has no DXVA2 support\n");
    return true;
}
/**
 * \fn admLibVa_exitCleanup
 */
bool admDxva2_exitCleanup()
{
    ADM_info("Dxva2 cleanup begin\n");
    ADM_info("Dxva2 cleanup end\n");
    return true;
}




#endif
// EOF
