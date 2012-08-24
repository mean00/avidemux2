/**
    \file   ADM_scriptDialogFactory.h
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

#ifndef _ADM_scriptDialogFactory_H
#define _ADM_scriptDialogFactory_H

#include "ADM_coreScript_export.h"
#include "DIA_factory.h"
#include "ADM_scriptDFMenu.h"
#include <vector>
/**
    \class ADM_scriptDialogFactoryHelper
*/
class ADM_CORESCRIPT_EXPORT ADM_scriptDialogFactoryHelper
{
private:
	char        *_title;
	std::vector <ADM_scriptDFBaseHelper*> _controls;

public:
                ADM_scriptDialogFactoryHelper(const char *title);
                ~ADM_scriptDialogFactoryHelper(void);
	void        addControl(ADM_scriptDFBaseHelper* control);
    int         run(void);
	diaElem**   getControls(int *controlCount);
	const char* title(void);
};

#endif
