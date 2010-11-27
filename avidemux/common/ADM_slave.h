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
#ifndef ADM_SLAVE_H
#define ADM_SLAVE_H

bool ADM_slaveConnect(uint32_t port);
bool ADM_slaveShutdown(void );
bool ADM_slaveReportProgress(uint32_t percent);

#endif

//EOF
