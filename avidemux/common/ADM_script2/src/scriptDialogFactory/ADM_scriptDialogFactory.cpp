/**
    \file   ADM_scriptDialogFactory.cpp
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
#include "scriptDialogFactory/ADM_scriptDialogFactory.h"

ADM_scriptDialogFactoryHelper::ADM_scriptDialogFactoryHelper(const char *title)
{
	_title = ADM_strdup(title);
}

ADM_scriptDialogFactoryHelper::~ADM_scriptDialogFactoryHelper(void)
{
	if (_title)
		delete _title;

	_title = NULL;
}

void ADM_scriptDialogFactoryHelper::addControl(ADM_scriptDFBaseHelper* control)
{
	_controls.push_back(control);
}

diaElem** ADM_scriptDialogFactoryHelper::getControls(int *controlCount)
{
	*controlCount = (int)_controls.size();

	std::vector<ADM_scriptDFBaseHelper*>::iterator it;
	int i = 0;
	diaElem **elems = new diaElem*[*controlCount];

	for (it = _controls.begin(); it != _controls.end(); it++)
	{
		elems[i] = (*it)->getControl();
		i++;
	}

	return elems;
}

const char* ADM_scriptDialogFactoryHelper::title(void)
{
	return _title;
}
// EOF

