/**
    \file   ADM_JSDFFloat.cpp
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
#include "ADM_scriptDFFloat.h"

ADM_scriptDFFloatHelper::ADM_scriptDFFloatHelper(const char *title,double mn,double mx, int prec)
{
	_title = ADM_strdup(title); 
    _min=mn;
    _max=mx;
    _precision=prec;
	_value = 0;
}

ADM_scriptDFFloatHelper::~ADM_scriptDFFloatHelper(void)
{
	if (_title)
		ADM_dealloc( _title);

	_title = NULL;


}


diaElem* ADM_scriptDFFloatHelper::getControl(void)
{
    return new diaElemFloat(&_value,_title, _min,_max, NULL, _precision);
}

double ADM_scriptDFFloatHelper::value(void)
{
	return _value;
}

void ADM_scriptDFFloatHelper::setValue(double index)
{
	_value = index;
}
