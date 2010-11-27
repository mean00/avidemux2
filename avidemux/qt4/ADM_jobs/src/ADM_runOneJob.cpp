/**
    \file   ADM_jobControl
    \author mean fixounet@free.Fr (c) 2010

*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "T_jobs.h"
#include "ADM_default.h"
#include "ADM_coreJobs.h"
#include "pthread.h"

/**
    \fn spawnerBoomerang
    \brief Allow to do class -> plain C -> class 
*/
static void *spawnerBoomerang(void *arg)
{
    spawnData    *data=(spawnData *)arg;
    data->me->runProcess(data);
    return NULL;
}

bool spawnProcess(const char *processName, int argc, const string argv[])
{
    ADM_info("Starting <%s>\n",processName);
    string command=string(processName);
    for(int i=0;i<argc;i++)
    {
        ADM_info("%d : %s\n",i,argv[i].c_str());
        command+=string(" ")+argv[i];
    }
    ADM_info("=>%s\n",command.c_str());
    ADM_info("==================== Start of spawner process job ================\n");
    system(command.c_str());
    ADM_info("==================== End of spawner process job ================\n");
    return true;
}

/**
    \fn runProcess
    \brief Thread can will run a process
*/
bool jobWindow::runProcess(spawnData *data)
{
    // 3 args in our case...
    string argv[5];
    char str[100];
    sprintf(str,"--slave %d",localPort);
    argv[0]=string("--nogui ");
    argv[1]=string(str);
    argv[2]=string("--runpy \"")+data->script+string("\" ");
    argv[3]=string("--save \"")+data->outputFile+string("\" ");
    argv[4]=string("--quit > /tmp/prout.log");
    return spawnProcess(data->exeName,5,argv);
}
/**
    \fn spawnChild
    \brief Spawn a child to execute a commande
*/
bool jobWindow::spawnChild(const char *exeName, const string &script, const string &outputFile)
{
    spawnData data;
            data.script=script;
            data.outputFile=outputFile;
            data.exeName=exeName;
            data.me=this;
            pthread_t threadId;
        // Have to spawn a thread that will handle the exec...
            if( pthread_create(&threadId,NULL,
                                spawnerBoomerang, &data))
            {
                ADM_error("Spawn failed\n");
                return false;
            }
            // Everything is fine, give process 1 second to use the stack allocated data....
            ADM_usleep(1000*1000LL);
            ADM_info("Spawning successfull\n");
            return true;
}
 
/**
    \fn runOneJob
*/
bool jobWindow::runOneJob( ADMJob &job)   
{
    bool r=false;
    uint32_t version;
    job.startTime=ADM_getSecondsSinceEpoch();
    job.status=ADM_JOB_RUNNING;
    ADM_commandSocket *runSocket=NULL;
    ADM_jobUpdate(job);
    
    // 1- Start listening to socket

    // 2- Spawn  child
    string ScriptFullPath;
    ScriptFullPath=string(ADM_getJobDir())+string("/")+string(job.scriptName);
    if(false==spawnChild("avidemux3_qt4",ScriptFullPath,job.outputFileName))
    {
        ADM_error("Cannot spawn child\n");
        r=false;
        goto done;
    }

    // 3- Wait for connect...
    runSocket=mySocket.waitForConnect(6*1000);
    if(!runSocket)
    {
        ADM_error("No connect\n");
        goto done;
    }
    if(!runSocket->handshake())
    {
        popup("Cannot handshake");
        goto done;
    }
    // 4- wait for complete and/or success message
    ADM_socketMessage msg;
    while(1)
    {
        if(!runSocket->isAlive())
        {
            ADM_info("Exiting loop\n");
            break;
        }
        if(runSocket->pollMessage(msg))
        {
            ADM_info("Got a new message %d\n",msg.command);
        }
        ADM_usleep(1000*1000); // Refresh once per sec
    }
    ADM_info("** End of slave process **\n");
    ADM_info("** End of slave process **\n");
    ADM_info("** End of slave process **\n");

    // 5-Done, do the cleanup
done:

    if(r) job.status=ADM_JOB_OK;
        else job.status=ADM_JOB_KO;
    job.endTime=ADM_getSecondsSinceEpoch();
    ADM_jobUpdate(job);
    if(runSocket) delete runSocket;
    refreshList();
    ADM_info("Running job id = %d\n",job.id);
    return r;
}
