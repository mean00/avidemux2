/**
    \file   ADM_JSDFInteger.cpp
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
#include "scriptDialogFactory/ADM_scriptDFInteger.h"

ADM_scriptDFIntegerHelper::ADM_scriptDFIntegerHelper(const char *title,int32_t mn,int32_t mx)
{
	_title = ADM_strdup(title); 
    _min=mn;
    _max=mx;
	_value = 0;
}

ADM_scriptDFIntegerHelper::~ADM_scriptDFIntegerHelper(void)
{
	if (_title)
		delete _title;

	_title = NULL;


}


diaElem* ADM_scriptDFIntegerHelper::getControl(void)
{
    return new diaElemInteger(&_value,_title, _min,_max);
}

int32_t ADM_scriptDFIntegerHelper::value(void)
{
	return _value;
}

void ADM_scriptDFIntegerHelper::setValue(int32_t index)
{
	_value = index;
}
