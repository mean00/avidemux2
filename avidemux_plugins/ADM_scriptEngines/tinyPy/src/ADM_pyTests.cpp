/***************************************************************************
   \file ADM_pyAvidemux.cpp
    \brief binding between tinyPy and avidemux
    \author mean/gruntster 2011/2012
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_pyAvidemux.h"
#include "ADM_audiodef.h"
#include "ADM_vidMisc.h"
#include "fourcc.h"
#include "DIA_fileSel.h"
#include "DIA_coreToolkit.h"
#include "ADM_coreSubtitles.h"

/**
    \fn pyTestCrash
*/

int pyTestCrash(void)
{
	int *foobar = NULL;
	*foobar = 0; // CRASH!
	return true;
}
/**
    \fn pyTestAssert
*/

int pyTestAssert(void)
{
	ADM_assert(0);
	return true;
}
int pyTestSub( char *subName)
{
    ADM_info("pyTestSub : %s\n",subName);
    ADM_subtitle sub;
    if(!sub.load(subName))
    {
        ADM_warning("Cannot load <%s>\n",subName);
        return -1;
    }
    ADM_info("Dumping : %s\n",subName);
    sub.dump();
    
    ADM_info("Converting to  ssa: %s\n",subName);
    sub.srt2ssa();

    ADM_info("Dumping : %s\n",subName);
    sub.dump();

    ADM_info("Done : %s\n",subName);
    return 1;
}

// EOF
