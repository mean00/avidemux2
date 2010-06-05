/**
    \file ADM_jsVideo.cpp
    \brief Video oriented functions
    \author mean (c) 2009 fixounet@free.fr


    jsapigen does not like much variable number of arguments
    In that case, we patch the generated file to go back to native spidermonkey api


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "ADM_scriptCommon.h"
#include "ADM_scriptIf.h"
#include "ADM_scriptGui.h"

#include "DIA_fileSel.h"
/**

*/
char *scriptFileSelRead(const char *title)
{
char *me=NULL;
    GUI_FileSelRead(title,&me);
    return me;
}
/**

*/
char *scriptFileSelWrite(const char *title)
{
    char *me=NULL;
    GUI_FileSelWrite(title,&me);
    return me;
}
/**

*/
char *scriptDirSelect(const char *title)
{
    char me[1024]={0};
    if(!FileSel_SelectDir(QT_TR_NOOP("Select a directory"),me,1023, NULL))
        return NULL;
    return ADM_strdup(me);
}
/**

*/
void scriptDisplayError(const char *one, const char *two)
{
    GUI_Error_HIG(one,two);
}
/**

*/
void scriptDisplayInfo(const char *one, const char *two)
{
    GUI_Info_HIG(ADM_LOG_INFO,one,two);
}

//EOF
