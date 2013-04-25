/**
    \file   ADM_scriptDFMenu.cpp
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
#include "ADM_scriptDFMenu.h"

ADM_scriptDFMenuHelper::ADM_scriptDFMenuHelper(const char *title)
{
	_title = ADM_strdup(title);
	_menuEntries = NULL;
	_index = 0;
}

ADM_scriptDFMenuHelper::~ADM_scriptDFMenuHelper(void)
{
	if (_title)
		ADM_dealloc( _title);

	_title = NULL;

	std::vector<char*>::iterator it;

	for (it = _items.begin(); it != _items.end(); it++)
		delete *it;

	_items.clear();

	if (_menuEntries)
		delete _menuEntries;

	_menuEntries = NULL;
}

void ADM_scriptDFMenuHelper::addItem(const char* item)
{
	_items.push_back(ADM_strdup(item));
}

diaElem* ADM_scriptDFMenuHelper::getControl(void)
{
	if (_menuEntries)
		delete _menuEntries;

	std::vector<char*>::iterator it;
	int i = 0;

	_menuEntries = new diaMenuEntry[_items.size()];	

	for (it = _items.begin(); it != _items.end(); it++)
	{
		_menuEntries[i].val = i;
		_menuEntries[i].text = *it;
		_menuEntries[i].desc = NULL;

		i++;
	}

	return new diaElemMenu(&_index, _title, _items.size(), _menuEntries);
}

int ADM_scriptDFMenuHelper::index(void)
{
	return _index;
}

void ADM_scriptDFMenuHelper::setIndex(int index)
{
	_index = index;
}

