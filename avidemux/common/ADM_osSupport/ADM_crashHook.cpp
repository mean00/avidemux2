/***************************************************************************
  Try to display interesting crash dump

    copyright            : (C) 2007 by mean, (C) 2007 Gruntster
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


#include <string>
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "ADM_edit.hxx"
#include "A_functions.h"
#include "../ADM_script2/include/ADM_script.h"

extern ADM_Composer *video_body;

#define CRASH_FILE "crash.py"

void saveCrashProject(void);
extern char *ADM_getBaseDir(void);

/**
    \fn saveCrashProject
    \brief Try to save the current project, useful in case of crash
*/
void saveCrashProject(void)
{
  char *baseDir=ADM_getBaseDir();
  const char *name=CRASH_FILE;
  static int crashCount=0;
  if(crashCount) return ; // avoid endless looping
  crashCount++;
  char *where=new char[strlen(baseDir)+strlen(name)+2];
  strcpy(where,baseDir);
  strcat(where,name);
  printf("Saving crash file to %s\n",where);

  A_saveScript(getScriptEngines()[0], where);

  delete[] where;
}
/**
    \fn checkCrashFile
    \brief Check if there i a crash file
*/

void checkCrashFile(void)
{
#ifdef USE_TINYPY
  char *baseDir=ADM_getBaseDir();
  const char *name=CRASH_FILE;
  static int crashCount=0;
  char *where=new char[strlen(baseDir)+strlen(name)+2];
  strcpy(where,baseDir);
  strcat(where,name);
  if(ADM_fileExist(where))
  {
    if(GUI_Confirmation_HIG(QT_TR_NOOP("Load it"),QT_TR_NOOP("Crash file"),
       QT_TR_NOOP("I have detected a crash file. \nDo you want to load it  ?\n(It will be deleted in all cases, you should save it if you want to keep it)")))
    {
       A_parseTinyPyScript(where);
    }
    unlink(where);
  }else
  {
    printf("No crash file (%s)\n",where);
  }
  delete [] where;
#endif
}
//EOF
