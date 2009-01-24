/***************************************************************************
                          ADM_deviceArts.h  -  description
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

 
     class artsAudioDevice : public audioDevice
	 {
		 protected :
		 		arts_stream_t _stream;
		  public:
		  			artsAudioDevice() {_stream=NULL;}
				virtual ~artsAudioDevice() {stop();}
		     		virtual uint8_t init(uint32_t channels, uint32_t fq);
	    			virtual uint8_t play(uint32_t len, float *data);
		      		virtual uint8_t stop(void);
				uint8_t setVolume(int volume);
		 };



