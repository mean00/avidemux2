/**
    \file   ADM_JSDFInteger.h
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

#ifndef _ADM_scriptDFInteger_H
#define _ADM_scriptDFInteger_H

#include "ADM_coreScript_export.h"
#include "DIA_factory.h"
#include "ADM_scriptDF.h"
/**
    \class ADM_JSDFIntegerHelper
*/
class ADM_CORESCRIPT_EXPORT ADM_scriptDFIntegerHelper : public ADM_scriptDFBaseHelper
{
private:
	char                *_title;
	int32_t             _value;
        int32_t             _min,_max;

public:
                     ADM_scriptDFIntegerHelper(const char *title,int32_t mn, int32_t mx);
                     ~ADM_scriptDFIntegerHelper(void);
	virtual diaElem* getControl(void);
            int32_t value(void);
            void     setValue(int32_t value);
};
#endif
