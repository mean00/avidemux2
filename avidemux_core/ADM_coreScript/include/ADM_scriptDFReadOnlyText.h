/**
    \file   ADM_JSDFReadOnly.h
    \brief  JS / DF binding
    \author gruntster/mean 2010


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ADM_scriptDFReadOnlyText_H
#define _ADM_scriptDFReadOnlyText_H

#include "ADM_coreScript_export.h"
#include "DIA_factory.h"
#include "ADM_scriptDF.h"
/**
    \class ADM_JSDFReadOnlyHelper
*/
class ADM_CORESCRIPT_EXPORT ADM_scriptDFReadOnlyTextHelper : public ADM_scriptDFBaseHelper
{
private:
	char                *_title;

public:
                     ADM_scriptDFReadOnlyTextHelper(const char *title);
                     ~ADM_scriptDFReadOnlyTextHelper(void);
	virtual diaElem* getControl(void);
};
#endif
