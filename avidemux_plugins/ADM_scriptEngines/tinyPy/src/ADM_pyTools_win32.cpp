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

#include "ADM_pyTools.h"
#include "ADM_default.h"

double pyTool_time(IEditor *editor)
{
        return 0.0;
}

char * pyTool_date(IEditor *editor)
{
        return ADM_strdup("none");
}


int pyTool_randint(IEditor *editor, int start, int stop)
{
        return start;
}

int pyTool_isalnum(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!isalnum(str[i])) return 0; }
    return 1;
}

int pyTool_isalpha(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!isalpha(str[i])) return 0; }
    return 1;
}

int pyTool_isdigit(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!isdigit(str[i])) return 0; }
    return 1;
}

int pyTool_islower(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!islower(str[i])) return 0; }
    return 1;
}

int pyTool_isspace(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!isspace(str[i])) return 0; }
    return 1;
}

int pyTool_isupper(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!isupper(str[i])) return 0; }
    return 1;
}

char* pyTool_upper(IEditor *editor, const char * str)
{
    char * t = ADM_strdup(str);
    for (int i=0; i<strlen(str); i++) { t[i] = toupper(t[i]); }
    return t;
}

char* pyTool_lower(IEditor *editor, const char * str)
{
    char * t = ADM_strdup(str);
    for (int i=0; i<strlen(str); i++) { t[i] = tolower(t[i]); }
    return t;
}

// EOF
