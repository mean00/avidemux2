/**
        \file ADM_paramList
        \brief Structure and stuff to handle autoconfiguration of configuration struct
        Mean,2009
        GPL V2
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_PARAM_LIST_H
#define ADM_PARAM_LIST_H

#include "ADM_confCouple.h"
/**
        \enum ADM_paramType
        \brief Type of parameter

*/
typedef enum
{
        ADM_param_uint32_t=1,
        ADM_param_int32_t=2,
        ADM_param_float=3,
       // ADM_param_bool=4,
        ADM_param_string=5
}ADM_paramType;


/**
        \struct ADM_paramList
        \brief  one parmeter desc

*/
typedef struct
{
        const char      *paramName;
        uint32_t        offset;
        const char      *typeAsString;
        ADM_paramType   type;
}ADM_paramList;

/// Couple -> structure
bool ADM_paramLoad(CONFcouple *couples, const ADM_paramList *params,void *s);
/// Structure -> couples
bool ADM_paramSave(CONFcouple **couples, const ADM_paramList *params,void *s);


#endif
//EOF