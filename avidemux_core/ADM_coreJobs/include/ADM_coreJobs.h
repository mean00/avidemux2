/**
    \file   ADM_coreJobs
    \brief  Handle low level access to jobs
    \author (C) 2010 by mean fixounet@free.fr
        
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_COREJOBS_H
#define ADM_COREJOBS_H

#include "ADM_coreJobs_export.h"
#include "ADM_cpp.h"
#include "ADM_default.h"

/**
    \enum ADM_JOB_STATUS
*/
typedef enum 
{
        ADM_JOB_UNKNOWN=0,
        ADM_JOB_IDLE,
        ADM_JOB_RUNNING,
        ADM_JOB_OK,
        ADM_JOB_KO,
        ADM_JOB_MAX
}ADM_JOB_STATUS;
/**
    \class ADMJob
*/
class ADM_COREJOBS_EXPORT ADMJob
{
public:
    uint32_t            id;
    string              jobName;
    string              scriptName;
    string              outputFileName;
    ADM_JOB_STATUS      status;
    uint64_t            startTime;  /// epoch
    uint64_t            endTime;
                        ADMJob(void) {id=0;jobName=string("");scriptName=string("");outputFileName=string("");
                                            status=ADM_JOB_UNKNOWN;startTime=endTime=0;}


    static bool    jobInit(void);
    static bool    jobShutDown(void);
    static bool    jobAdd(const ADMJob& job);
    static bool    jobDelete(const ADMJob& job);
    static bool    jobGet(vector <ADMJob> &jobs);
    static bool    jobUpdate(const ADMJob & job);

    static bool    jobDump(const ADMJob &job);
    static bool    jobDropAllJobs(void);
};
#endif
// EOF
