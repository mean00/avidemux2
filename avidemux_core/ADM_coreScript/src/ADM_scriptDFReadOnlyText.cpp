/**
    \file   ADM_JSDFReadOnlyText.cpp
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
#include "ADM_scriptDFReadOnlyText.h"

ADM_scriptDFReadOnlyTextHelper::ADM_scriptDFReadOnlyTextHelper(const char *title)
{
	_title = ADM_strdup(title); 
}

ADM_scriptDFReadOnlyTextHelper::~ADM_scriptDFReadOnlyTextHelper(void)
{
	if (_title)
		ADM_dealloc( _title);

	_title = NULL;


}


diaElem* ADM_scriptDFReadOnlyTextHelper::getControl(void)
{
    return new diaElemReadOnlyText(NULL, _title, NULL);
}

