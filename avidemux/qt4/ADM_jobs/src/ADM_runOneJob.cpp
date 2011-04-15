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
#ifdef _WIN32
#define UNICODE
#include <winbase.h>
#include <windows.h>

#endif

#include "T_jobs.h"
#include "T_progress.h"
#include "ADM_default.h"
#include "ADM_coreJobs.h"
#include "pthread.h"

#ifdef _WIN32
int utf8StringToWideChar(const char *utf8String, int utf8StringLength, wchar_t *wideCharString);
#endif

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

#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    // utf8 -> utf16
    const char *c=command.c_str();
    int size=utf8StringToWideChar(c,strlen(c),NULL);
    wchar_t w[size+1];

    utf8StringToWideChar(c,strlen(c),w);
    // Start the child process. 
    if( !CreateProcessW( 
        NULL,   // No module name (use command line)
        w,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {
        ADM_error("Cannot spawn process! (%s)\n",c);
        return false;
    }
    
#else
    system(command.c_str());
#endif
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
#ifndef __WIN32
    argv[4]=string("--quit > /tmp/prout.log");
#else
    argv[4]=string("--quit ");
#endif
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
            QApplication::processEvents();
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
    ADM_socketMessage msg;
    uint32_t v;
    
    // 1- Start listening to socket

    // 2- Spawn  child
    string ScriptFullPath;
    
#ifdef _WIN32
    #define MKEXE(x) "avidemux3_"#x".exe"
    string slash=string("\\");
#else
    #define MKEXE(x) "avidemux3_"#x
    string slash=string("/");
#endif
    ScriptFullPath=string(ADM_getJobDir())+slash+string(job.scriptName);
    const char *avidemuxVersion=MKEXE(cli);
    if(ui.checkBoxUseQt4->isChecked())
    {
        avidemuxVersion=MKEXE(qt4);
    }
    if(false==spawnChild(avidemuxVersion,ScriptFullPath,job.outputFileName))
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
            switch(msg.command)
            {
                case ADM_socketCommand_End:
                            if(msg.getPayloadAsUint32_t(&v))
                            {
                                    r=(bool)v;
                                    ADM_info("Result is %d\n",r);
                                    goto done;
                            }else
                            {
                                    ADM_error("Can read End payload   \n");
                            }
                            break;
                case ADM_socketCommand_Progress:
                            if(msg.getPayloadAsUint32_t(&v))
                            {
                                    printf("Progress %d %%\n",(int)v);
                                    dialog->setPercent(v);
                            }else
                            {
                                    ADM_error("Can read End payload   \n");
                            }
                            break;
                default:        ADM_error("Unknown command\n");
                                break;
            }
        }
        ADM_assert(dialog);
        QApplication::processEvents();
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
