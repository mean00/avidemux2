/**
    \file   ADM_JSDF.h
    \brief  JS / DF binding
    \author gruntster/Mean 2010


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ADM_JSDF_H
#define _ADM_JSDF_H

/**
    \class ADM_JSBaseHelper
*/
class ADM_JSDFBaseHelper
{
public:
                        ADM_JSDFBaseHelper() {};
    virtual             ~ADM_JSDFBaseHelper() {};
    virtual diaElem*    getControl(void)=0;
};

#endif
