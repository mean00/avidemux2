/***************************************************************************
                          gui_action.hxx  -  description
                             -------------------
    begin                : Fri Jan 18 2002
    copyright            : (C) 2002 by mean
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

#ifndef __GUI_ACTION
#define __GUI_ACTION
#define ADM_MAC_CUSTOM_SCRIPT 10
enum Action
{
    ACT_INVALID = 0,

#define ACT(_name) ACT_ ## _name,
#include "gui_action.names"
#undef ACT

ACT_CUSTOM_BASE_JS,
ACT_CUSTOM_END_JS=ACT_CUSTOM_BASE_JS+ADM_MAC_CUSTOM_SCRIPT,
ACT_CUSTOM_BASE_PY,
ACT_CUSTOM_END_PY=ACT_CUSTOM_BASE_PY+ADM_MAC_CUSTOM_SCRIPT,
ACT_DUMMY
};

void HandleAction (Action action);
Action lookupActionByName (const char * name);
const char * getActionName (Action act);
void dumpActionNames (const char * filename);

#endif
