//
// C++ Interface: ADM_deviceAudioCore
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifdef __APPLE__

	        class coreAudioDevice : public audioDevice
	 {
		 protected :
					uint8_t				_inUse;
		  public:
		  				coreAudioDevice(void);
		     		virtual uint8_t init(uint8_t channels, uint32_t fq);
	    			virtual uint8_t play(uint32_t len, float *data);
		      		virtual uint8_t stop(void);
					virtual uint8_t setVolume(int volume);
		 }     ;

#endif
