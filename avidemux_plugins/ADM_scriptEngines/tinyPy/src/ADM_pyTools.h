/***************************************************************************
   \file ADM_pyTools.cpp
    \brief misc. helpers, not strictly related to ADM
    \author szlldm 2023
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_PYTOOLS_H
#define ADM_PYTOOLS_H

#include "IEditor.h"

double pyTool_time(IEditor *editor);
char * pyTool_date(IEditor *editor);
int pyTool_randint(IEditor *editor, int start, int stop);
int pyTool_isalnum(IEditor *editor, const char * str);
int pyTool_isalpha(IEditor *editor, const char * str);
int pyTool_isdigit(IEditor *editor, const char * str);
int pyTool_islower(IEditor *editor, const char * str);
int pyTool_isspace(IEditor *editor, const char * str);
int pyTool_isupper(IEditor *editor, const char * str);
char* pyTool_upper(IEditor *editor, const char * str);
char* pyTool_lower(IEditor *editor, const char * str);
#endif
// EOF
