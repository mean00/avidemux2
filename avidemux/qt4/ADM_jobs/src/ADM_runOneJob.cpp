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
#include "ADM_cpp.h"

#define UNICODE
#include <windows.h>

#endif

#include "T_jobs.h"
#include "T_progress.h"
#include "ADM_default.h"
#include "ADM_coreJobs.h"
#include "pthread.h"

#ifdef _WIN32
    int utf8StringToWideChar(const char *utf8String, int utf8StringLength, wchar_t *wideCharString);
    #define MKCLI() "avidemux_cli.exe"
    #define MKQT()  "avidemux.exe"
    const string slash=string("\\");
#else
    #ifdef __APPLE__
        #define STR(x) #x
        #define MKSTRING(x) STR(x)
        #define MKQT()  admExecutable("Avidemux" MKSTRING(AVIDEMUX_MAJOR_MINOR))
        #define MKCLI() admExecutable( "avidemux_cli")
    #else
        #define MKCLI() admExecutable("avidemux3_cli")
        #if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            #define MKQT() admExecutable("avidemux3_qt6")
        #elif QT_VERSION >= QT_VERSION_CHECK(5,0,0)
            #define MKQT() admExecutable("avidemux3_qt5")
        #else
            #define MKQT() admExecutable("avidemux3_qt4")
        #endif
    #endif
    const string slash=string("/");
#endif
/**
 * 
 * @param exeName
 * @return 
 */
const char *admExecutable(const char *exeName)    
{
   static char fullName[2048]; // only use once per job, so "safe" to use static
   sprintf(fullName,"%s%s%s",QCoreApplication::applicationDirPath().toStdString().c_str(),slash.c_str(),exeName);
   return fullName;
}
    
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

static bool spawnProcess(const char *processName, const vector <string> args)
{
    ADM_info("Starting <%s>\n",processName);
    string command=string(processName);
    for(int i=0;i<args.size();i++)
    {
        ADM_info("%d : %s\n",i,args.at(i).c_str());
        command+=string(" ")+args.at(i);
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
    wchar_t* w = new wchar_t[size+1];

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
        delete [] w;
        ADM_error("Cannot spawn process! (%s)\n",c);
        return false;
    }

    delete [] w;
    
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
    vector <string> args;
    if(portable)
        args.push_back(string("--portable"));
    args.push_back(string("--nogui "));
    char str[100];
    sprintf(str,"--slave %d",localPort);
    args.push_back(string(str));
    string s=string("--run \"")+data->script+string("\" ");
    args.push_back(s);
    s=string("--save \"")+data->outputFile+string("\" ");
    args.push_back(s);
#ifndef _WIN32
    args.push_back(string("--quit > /tmp/prout.log"));
#else
    args.push_back(string("--quit"));
#endif
    return spawnProcess(data->exeName,args);
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
    job.startTime=ADM_getSecondsSinceEpoch();
    job.status=ADM_JOB_RUNNING;
    ADM_commandSocket *runSocket=NULL;
    ADM_socketMessage msg;
    uint32_t v, version, wait=500; // by default, give application 500 ms to exit

    ADMJob::jobUpdate(job);
    refreshList();

    // 1- Start listening to socket

    // 2- Spawn  child
    string ScriptFullPath=ADM_getJobDir()+slash+string(job.scriptName);
    const char *avidemuxVersion=MKCLI();
    if(ui.checkBoxUseQt4->isChecked())
    {
        avidemuxVersion=MKQT();
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
                    if(!msg.getPayloadAsUint32_t(&v))
                    {
                        ADM_error("Cannot read End payload\n");
                        break;
                    }
                    r=(bool)v;
                    ADM_info("Result is %d\n",r);
                    msg.command = ADM_socketCommand_Readback;
                    msg.setPayloadAsUint32_t(v);
                    if(!runSocket->sendMessage(msg))
                        wait+=5000; // avoid starting next job while the application is still running
                    goto done;
                    break;
                case ADM_socketCommand_Progress:
                    if(!msg.getPayloadAsUint32_t(&v))
                    {
                        ADM_error("Cannot read Progress payload\n");
                        break;
                    }
                    printf("Progress %d %%\n",(int)v);
                    dialog->setPercent(v);
                    break;
                default:
                    ADM_error("Unknown command\n");
                    break;
            }
        }
        else
            ADM_usleep(1000*1000); // Refresh once per sec
        QApplication::processEvents();
    }
    ADM_info("** End of slave process **\n");
    ADM_info("** End of slave process **\n");
    ADM_info("** End of slave process **\n");

    // 5-Done, do the cleanup
done:

    if(r) job.status=ADM_JOB_OK;
        else job.status=ADM_JOB_KO;
    job.endTime=ADM_getSecondsSinceEpoch();
    ADMJob::jobUpdate(job);
    if(runSocket) delete runSocket;
    refreshList();
    ADM_info("Running job id = %d\n",job.id);
    ADM_usleep(wait*1000); // Give application some time to exit
    return r;
}
