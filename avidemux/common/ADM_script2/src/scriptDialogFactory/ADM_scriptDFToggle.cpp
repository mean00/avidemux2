/**
    \file   ADM_scriptDFToggle.cpp
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
#include "scriptDialogFactory/ADM_scriptDFToggle.h"

ADM_scriptDFToggleHelper::ADM_scriptDFToggleHelper(const char *title)
{
	_title = ADM_strdup(title);
	_value = 0;
}

ADM_scriptDFToggleHelper::~ADM_scriptDFToggleHelper(void)
{
	if (_title)
		delete _title;

	_title = NULL;


}


diaElem* ADM_scriptDFToggleHelper::getControl(void)
{
    return new   diaElemToggle(&_value,_title,NULL);
}

uint32_t ADM_scriptDFToggleHelper::value(void)
{
	return _value;
}

void ADM_scriptDFToggleHelper::setValue(uint32_t index)
{
	_value = index;
}
/**/
