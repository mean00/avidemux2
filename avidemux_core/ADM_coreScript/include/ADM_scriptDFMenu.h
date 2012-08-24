/**
    \file   ADM_scriptDFMenu.h
    \brief  JS / DF binding
    \author gruntster 2010


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ADM_scriptDFMenu_H
#define _ADM_scriptDFMenu_H

#include "ADM_coreScript_export.h"
#include "DIA_factory.h"
#include "ADM_scriptDF.h"
#include <vector>
/**
    \class ADM_scriptDFMenuHelper
*/
class ADM_CORESCRIPT_EXPORT ADM_scriptDFMenuHelper : public ADM_scriptDFBaseHelper
{
private:
	char                *_title;
	uint32_t            _index;
	diaMenuEntry        *_menuEntries;
	std::vector <char*> _items;

public:
                     ADM_scriptDFMenuHelper(const char *title);
                     ~ADM_scriptDFMenuHelper(void);
            void     addItem(const char* item);
	virtual diaElem* getControl(void);
            int      index(void);
            void     setIndex(int index);
};

#endif
