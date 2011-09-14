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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <cxxabi.h>

#include "ADM_default.h"

// Our callback to give UI formatted informations....
static ADM_saveFunction *mysaveFunction=NULL;
static ADM_fatalFunction *myFatalFunction=NULL;
/**
        \fn ADM_setCrashHook
        \brief install crash handlers (save + display)
*/
void ADM_setCrashHook(ADM_saveFunction *save, ADM_fatalFunction *fatal)
{
        mysaveFunction=save;
        myFatalFunction=fatal;
}

void installSigHandler() {}

void ADM_backTrack(const char *info,int lineno,const char *file)
{
	char bfr[1024];

	if (mysaveFunction)
		mysaveFunction();

	snprintf(bfr,1024,"%s\n file %s, line %d\n", info, file, lineno);

	if(myFatalFunction)
		myFatalFunction("Crash",bfr);

	exit(-1);
}
