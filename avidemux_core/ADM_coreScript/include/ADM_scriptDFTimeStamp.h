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

#ifndef _ADM_scriptDFTimeStamp
#define _ADM_scriptDFTimeStamp

#include "ADM_coreScript_export.h"
#include "DIA_factory.h"
#include "ADM_scriptDF.h"
/**
    \class ADM_JSDFIntegerHelper
*/
class ADM_CORESCRIPT_EXPORT ADM_scriptDFTimeStampHelper : public ADM_scriptDFBaseHelper
{
private:
	char                *_title;
	uint32_t             _value;
        uint32_t             _min,_max;

public:
                     ADM_scriptDFTimeStampHelper(const char *title,uint32_t mn, uint32_t mx);
                     ~ADM_scriptDFTimeStampHelper(void);
	virtual diaElem* getControl(void);
            uint32_t  value(void);
            void      setValue(uint32_t value);
};
#endif
