/***************************************************************************
                          ADM_deviceAlsa.h  -  description
                             -------------------
    begin                : Sat Sep 28 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_deviceALSA_H
#define ADM_deviceALSA_H

 class alsaAudioDevice : public audioDeviceThreaded
 {
     protected :
            //	0-> no init done
            //	1-> device opened but init failed
            //	2->fully initialized
            uint32_t _init;
      public:
                                alsaAudioDevice(void);
            virtual     bool    localInit(void);
            virtual     bool    localStop(void);
            virtual     void    sendData(void); 
            virtual     uint8_t setVolume(int volume);
     }     ;
#endif
//EOF


