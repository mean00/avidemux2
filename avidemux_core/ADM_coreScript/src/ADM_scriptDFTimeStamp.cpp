/**
    \file   ADM_JSDFTimeStamp.cpp
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
#include "ADM_scriptDFTimeStamp.h"
/**
        \fn ctor
*/
ADM_scriptDFTimeStampHelper::ADM_scriptDFTimeStampHelper(const char *title,uint32_t mn,uint32_t mx)
{
    _title = ADM_strdup(title); 
    _min=mn;
    _max=mx;
    _value = 0;
}
/**
        \fn dtor
*/

ADM_scriptDFTimeStampHelper::~ADM_scriptDFTimeStampHelper(void)
{
	if (_title)
		delete _title;
	_title = NULL;
}

/**
        \fn getControl
*/
diaElem* ADM_scriptDFTimeStampHelper::getControl(void)
{
    return new diaElemTimeStamp(&_value,_title, _min,_max);
}
/**
        \fn value
*/
uint32_t ADM_scriptDFTimeStampHelper::value(void)
{
	return _value;
}
/**
        \fn setValue
*/
void ADM_scriptDFTimeStampHelper::setValue(uint32_t index)
{
	_value = index;
}
// EOF

