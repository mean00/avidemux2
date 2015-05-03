

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once

typedef const char *ADM_translator(const char *domain, const char *stringToTranslate);
ADM_CORE6_EXPORT void ADM_InitTranslator( ADM_translator &trans);

// EOF


