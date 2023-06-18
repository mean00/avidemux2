/**
    \file   ADM_JSDF.h
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

#ifndef _ADM_scriptDFText_H
#define _ADM_scriptDFText_H

#include "ADM_coreScript_export.h"
#include "DIA_factory.h"
#include "ADM_scriptDF.h"
/**
    \class ADM_JSDFHelper
*/
class ADM_CORESCRIPT_EXPORT ADM_scriptDFTextHelper : public ADM_scriptDFBaseHelper
{
private:
	char                *_title;
	char                *_text;

public:
                     ADM_scriptDFTextHelper(const char *title);
                     ~ADM_scriptDFTextHelper(void);
	virtual diaElem* getControl(void);
            const char *   value(void);
            void     setValue(const char * value);
};
#endif
