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
class ADMJob
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

};
/*
    Our tiny tiny interface
*/
bool    ADM_jobInit(void);
bool    ADM_jobShutDown(void);

int     ADM_jobCount(void);
bool    ADM_jobAdd(const ADMJob& job);
bool    ADM_jobDelete(const ADMJob& job);
bool    ADM_jobGet(vector <ADMJob> &jobs);
bool    ADM_jobUpdate(const ADMJob & job);

bool    ADM_jobDump(const ADMJob &job);
bool    ADM_jobDropAllJobs(void);

#endif
// EOF
