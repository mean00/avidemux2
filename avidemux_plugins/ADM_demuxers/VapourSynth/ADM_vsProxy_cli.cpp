/***************************************************************************
    \file ADM_vs.cpp
    \author (C) 2015 by mean    email                : fixounet@free.fr
    \brief VapourSynth demuxer

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "fourcc.h"
#ifndef __APPLE__
#include "ADM_memsupport.h"
#endif

#include "ADM_vsProxy.h"

static void printUsageAndExit()
{
    printf("vsProxy [--port PORT_TO_USE] scriptFile\n");
    exit(-1);
}


#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif
/**
 */
int main(int ac, char **av)
{
    int port = -1;
    int vsScriptPos = -1;
    if (ac < 2)
    {
        printUsageAndExit();
    }
    for (int avPos = 1; avPos < ac; ++avPos)
    {
        if (std::string("--port") == av[avPos])
        {
            if (++avPos == ac)
            {
                printf("Missing port number argument!");
                exit(-1);
            }
            try
            {
                size_t resPos;
                std::string strPort(av[avPos]);
                port = std::stoi(strPort, &resPos, 0);
                if (resPos != strPort.size())
                {
                    throw 123;
                }
            }
            catch(...)
            {
                printf("Invalid port number given '%s'\n", av[avPos]);
                exit(-1);
            }
            if (port < 1 || port > 65535)
            {
                printf("Invalid port number. Must be within range [1024;65535]\n");
                exit (-1);
            }
            if (port < 1024 && port > 1)
            {
                printf("Given port needs privilege access. This is not supported!\n");
                exit (-1);
            }
        }
        else
        {
            if (vsScriptPos == -1) {
                vsScriptPos = avPos;
                continue;
            }
            printf("Invalid argument '%s' found!\n", av[avPos]);
            exit(-1);
        }
    }

    // Missing script argument?
    if (vsScriptPos == -1)
    {
        printUsageAndExit();
    }
    // Use the default parameter
    if (port == -1) {
        port = 9999;
    }
#ifndef __APPLE__
    ADM_InitMemcpy();
#endif
#ifdef _WIN32
    WSADATA wsaData;
    int iResult;
            ADM_info("Initializing WinSock\n");
            fflush(stdout);
            iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
            if (iResult != NO_ERROR)
            {
                    printf("Error at WSAStartup()\n");
                    fflush(stdout);
                    exit(-1);
            }
            ADM_info("WinSock ok\n");

#endif


    vapourSynthProxy proxy;
    bool r=proxy.run(port,av[vsScriptPos]);
    if(r)
    {
        printf("Success\n");
        exit(0);
    }else
    {
        printf("Failure\n");
        exit(-1);
    }
    return 0;
}
//EOF
