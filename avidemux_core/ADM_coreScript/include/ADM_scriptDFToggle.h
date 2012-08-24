/**
    \file   ADM_JSDFToggle.h
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

#ifndef _ADM_scriptDFToggle_H
#define _ADM_scriptDFToggle_H

#include "ADM_coreScript_export.h"
#include "DIA_factory.h"
#include "ADM_scriptDF.h"
/**
    \class ADM_JSDFToggleHelper
*/
class ADM_CORESCRIPT_EXPORT ADM_scriptDFToggleHelper : public ADM_scriptDFBaseHelper
{
private:
	char                *_title;
	bool                 _value;

public:
                     ADM_scriptDFToggleHelper(const char *title);
                     ~ADM_scriptDFToggleHelper(void);
	virtual diaElem* getControl(void);
            uint32_t value(void);
            void     setValue(uint32_t value);
};
#endif
