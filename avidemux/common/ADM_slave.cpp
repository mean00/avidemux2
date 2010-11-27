/***************************************************************************
    \file ADM_slave
    \brief handle slave (i.e. started from a controlling daemon)
   \author (C) 2010  mean fixounet@free.fr
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
#include "ADM_coreSocket/include/ADM_coreCommandSocket.h"
#include "ADM_slave.h"
static ADM_commandSocket *mySocket=NULL;
/**
    \fn ADM_slaveConnect
    \brief connect to port given as arg
*/
bool ADM_slaveConnect(uint32_t port)
{
    mySocket=new ADM_commandSocket();
    if(!mySocket->connectTo(port))
    {
        ADM_error("Failed to connect to port %d\n",(int)port);
        delete mySocket;
        mySocket=NULL;
    }
    ADM_info("Connected to port %d\n",(int)port);
    return true;
}
/**
    \fn ADM_slaveShutdown
    \brief kill socket
*/
bool ADM_slaveShutdown(void)
{
    if(mySocket)
    {
        ADM_info("Closing slave socket\n");
        delete mySocket;
        mySocket=NULL;
        
    }
    return true;
}
/**
    \fn ADM_slaveReportProgress
*/
bool ADM_slaveReportProgress(uint32_t percent)
{
    if(!mySocket)    return true;
    ADM_info("Report : %d %%\n",percent);
    return true;
}



//EOF
