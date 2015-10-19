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
#include "DIA_coreToolkit.h"
#include "ADM_videoInfoExtractor.h"


#include "ADM_vsProxy.h"
#include <math.h>

extern uint8_t ADM_InitMemcpy(void);

#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif
/**
 */
int main(int ac, char **av)
{
    int port=9999;
    if(ac!=2)
    {
        printf("vsProxy scriptFile\n");
        exit(-1); 
    }
    ADM_InitMemcpy();
    
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
    bool r=proxy.run(port,av[1]);
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
