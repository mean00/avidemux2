/***************************************************************************
                          avidemutils.cpp  -  description
                             -------------------
    begin                : Sun Nov 11 2001
    copyright            : (C) 2001 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <vector>
#include <string>
using std::string;
using std::vector;

#include "ADM_default.h"
#include "ADM_string.h"

/**
        \fn ADM_splitString
        \brief convert a string in a list using a separator
*/
bool        ADM_splitString(const string &separator, const string &source, vector<string>& result)
{
string splitted=source;
result.clear();

    string::size_type next = splitted.find(separator);
    while(next != string::npos)
    {
        string chunk=splitted.substr(0, next);
        if(chunk.length())
                    result.push_back(chunk);
        splitted = splitted.substr(next + 1);
        next = splitted.find(separator);
    }
    if(splitted.length())
            result.push_back(splitted);
    return true;
}

//EOF
