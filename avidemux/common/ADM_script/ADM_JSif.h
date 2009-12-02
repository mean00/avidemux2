/**
    \file ADM_JSif.cpp
    \brief interface to js
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_JS_IF_H
#define ADM_JS_IF_H
/**
    \fn parseECMAScript
    \brief Compile & execute ecma script
*/
bool parseECMAScript(const char *name);

/**
    \fn    interactiveECMAScript
    \brief interprete & execute ecma script (interactive)
*/
bool interactiveECMAScript(const char *name);

#endif
