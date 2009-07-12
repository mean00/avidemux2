/***************************************************************************
    copyright            : (C) 2001/2005 by mean
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
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>

#include "ADM_default.h"
#include "ADM_assert.h"
#include "DIA_fileSel.h"
#include "DIA_coreToolkit.h"

#define MAX_JOBS 120
static char *jobName[MAX_JOBS];
extern uint8_t DIA_job(uint32_t nb,char **name);
uint8_t GUI_jobs(void)
{
uint32_t nb;
char     *name;
        memset(jobName,0,sizeof(jobName));
char *jobDir=ADM_getJobDir();
        if(!buildDirectoryContent(&nb,ADM_getJobDir(),jobName,MAX_JOBS,".js"))
        {
          delete [] jobDir;
          GUI_Error_HIG(QT_TR_NOOP("Oops"),QT_TR_NOOP("Something very wrong happened when building joblist."));
                return 0;
        }
        delete [] jobDir;
        if(!nb)
        {
          GUI_Info_HIG(ADM_LOG_IMPORTANT,QT_TR_NOOP("There are no jobs stored"), NULL);
                return 1;
        }
        DIA_job(nb,jobName);
        for(int i=0;i<nb;i++)
        {
                //printf("List : %s\n",jobName[i]);
                ADM_dealloc(jobName[i]);
                jobName[i]=NULL;
        }
        return 1;
}

//EOF
