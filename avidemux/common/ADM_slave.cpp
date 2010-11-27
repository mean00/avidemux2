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
#include "DIA_coreToolkit.h"
static ADM_commandSocket *mySocket=NULL;
/**
    \fn ADM_slaveConnect
    \brief connect to port given as arg
*/
bool ADM_slaveConnect(uint32_t port)
{
    uint32_t version;
    mySocket=new ADM_commandSocket();
    if(!mySocket->connectTo(port))
    {
        ADM_error("Failed to connect to port %d\n",(int)port);
        delete mySocket;
        mySocket=NULL;
    }
    ADM_info("Connected to port %d\n",(int)port);
 // 3b- handshake
    if(!mySocket->handshake())
    {
        ADM_error("Cannot handshake\n");
        goto done;
    }
    return true;
done:
    ADM_assert(0);
    return false;
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
/**
    \fn ADM_slaveReportProgress
*/
bool ADM_slaveSendResult(bool result)
{
    if(!mySocket)    return true;
    ADM_socketMessage msg;
    msg.setPayloadAsUint32_t(result);
    msg.command=ADM_socketCommand_End;
    mySocket->sendMessage(msg);
    return true;
}



//EOF
