/**
    \file   ADM_JSDFText.cpp
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
#include "ADM_scriptDFText.h"

ADM_scriptDFTextHelper::ADM_scriptDFTextHelper(const char *title)
{
	_title = ADM_strdup(title);
	_text = ADM_strdup("");
}

ADM_scriptDFTextHelper::~ADM_scriptDFTextHelper(void)
{
	if (_title)
		ADM_dealloc( _title);
	if (_text)
		ADM_dealloc( _text);

	_title = NULL;
	_text = NULL;

}


diaElem* ADM_scriptDFTextHelper::getControl(void)
{
    return new diaElemText(&_text, _title, NULL);
}

const char * ADM_scriptDFTextHelper::value(void)
{
    return _text;
}

void ADM_scriptDFTextHelper::setValue(const char * value)
{
	if (_text)
		ADM_dealloc( _text);
	_text = ADM_strdup(value);
}
