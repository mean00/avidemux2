/***************************************************************************
  Try to display interesting crash dump

    copyright            : (C) 2007 by mean, (C) 2007 Gruntster
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
#include "ADM_default.h"
#include "ADM_coreTranslator.h"

static const ADM_translator *myTranslator=NULL;

/**
 * \fn ADM_InitTranslator
 */
void ADM_InitTranslator(const ADM_translator &trans)
{
    myTranslator=&trans;
    
}
/**
 * \fn ADM_translate
 */
const char *ADM_translate(const char *domain, const char *stringToTranslate)
{
    if(!myTranslator)
    {
        return stringToTranslate;
    }
    return myTranslator(domain,stringToTranslate);
}
// EOF

