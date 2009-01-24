//
// C++ Implementation: op_ogsaveprocess
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
//
// C++ Implementation: op_ogsavecopy
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "config.h"
#include "fourcc.h"
#include "avi_vars.h"
#include "DIA_coreToolkit.h"

//#include "avilist.h"

#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_audio/aviaudio.hxx"
#include "ADM_audiofilter/audioprocess.hxx"

#include "ADM_default.h"
#include "op_ogsave.h"

#define aprintf printf

//________________________________________________
uint8_t	ADM_ogmWriteProcess::initVideo(const char *name)
{		
uint32_t w,h,fps1000,fcc;
        _prestore=0;
        _prestoring=1;
	_incoming = getLastVideoFilter (frameStart,frameEnd-frameStart);
 	_togo=_incoming->getInfo()->nb_frames;
  	_encode = getVideoEncoder (_incoming->getInfo()->width,_incoming->getInfo()->height);
	if (!_encode)
    		return 0;
 	
	TwoPassLogFile=new char[strlen(name)+6];
  	strcpy(TwoPassLogFile,name);
  	strcat(TwoPassLogFile,".stat"); 	  
   
 	_encode->setLogFile(TwoPassLogFile,_togo);
	
	int reuse = 0;

	if (_encode->isDualPass())
	{
		FILE *tmp;

		if ((tmp = fopen(TwoPassLogFile,"rt")))
		{
			fclose(tmp);

			if (GUI_Question(QT_TR_NOOP("Reuse the existing log file?")))
				reuse = 1;
		}
	}

  	if (!_encode->configure (_incoming, reuse))
    	{
      		delete 	_encode;
      		_encode = NULL;
                GUI_Error_HIG (QT_TR_NOOP("Filter init failed"), NULL);
      		return 0;
    	};
 	w= _incoming->getInfo ()->width;
	h=_incoming->getInfo ()->height;
	fps1000=_incoming->getInfo ()->fps1000;
	_fps1000=fps1000;
	fcc= fourCC::get((uint8_t *)_encode->getCodecName());
   	_videoBuffer=new uint8_t[w*h*3];
  	encoding_gui->setCodec(_encode->getDisplayName());
   
//-----------------------VBR--------------------------------------
	if (_encode->isDualPass ())
	{
		uint8_t *buffer;
		uint32_t len, flag;

		aprintf("\n** Dual pass encoding**\n");
	
		if(!reuse)
 		{
      			aprintf("**Pass 1:%lu\n",_togo);
      			_encode->startPass1 ();
			encoding_gui->setCodec(_encode->getCodecName());
			encoding_gui->setPhasis(QT_TR_NOOP("Pass one"));
                        ADMBitstream bitstream(w*h*3);
                        bitstream.data=_videoBuffer;
      			//__________________________________
      			//   now go to main loop.....
      			//__________________________________
      			for (uint32_t cf = 0; cf < _togo; cf++)
			{	
                                bitstream.cleanup(cf); 
	  			if (!_encode->encode (cf, &bitstream))
				{
					printf("\n Encoding of frame %lu failed !\n",cf);
	    				return 0;
				}
                                encoding_gui->setFrame(cf,bitstream.len,bitstream.out_quantizer,_togo);
				if(!encoding_gui->isAlive())
				{
					return 0;
				}
			}
		     	aprintf("**Pass 1:done\n");
    		}// End of reuse

      		if(!_encode->startPass2 ())
		{
      			printf("Pass2 ignition failed\n");
      			return 0;
		}
		encoding_gui->setPhasis(QT_TR_NOOP("Pass 2"));
	}   //-------------------------/VBR-----------------------------------
	else
	{
		encoding_gui->setPhasis(QT_TR_NOOP("Encoding"));
	}
  // init save avi

// now we build the new stream !
    	aprintf("**main pass:\n");


		stream_header header;
		int64_t dur64;
		uint32_t dur32;
		uint16_t dur16;
		
		memset(&header,0,sizeof(header));
		
		memcpy(&(header.streamtype),"video\0\0\0",8);
		MEMCPY(&(header.subtype),&fcc,4);
		
		//header.size=sizeof(header);
		dur32=sizeof(header);
		MEMCPY(&header.size,&dur32,4);
		MEMCPY(&(header.video.width),&w,4);
		MEMCPY(&(header.video.height),&h,4);
		// Timing ..
		double duration; // duration in 10us
		duration=fps1000;
		duration=1000./duration;
		duration*=1000*1000;
		duration*=10;
		
		dur64=(int64_t)duration;
		
		MEMCPY(&header.time_unit,&dur64,8);
		dur64=1;
		MEMCPY(&header.samples_per_unit,&dur64,8);
		
		dur32=0x10000;
		MEMCPY(&header.buffersize,&dur32,4);
		
		dur16=24;
		MEMCPY(&header.bits_per_sample,&dur16,2);
		
		
		//header.default_len=1;
		dur32=1;
		MEMCPY(&header.default_len,&dur32,4);
		
		return videoStream->writeHeaders(sizeof(header),(uint8_t *)&header); // +4 ?

}
//___________________________________________________
uint8_t	ADM_ogmWriteProcess::writeVideo(uint32_t frame)
{
uint32_t len,flags;
uint8_t ret;
uint32_t page=_incoming->getInfo ()->width*_incoming->getInfo ()->height;
ADMBitstream bitstream(page*3);
                 bitstream.data=_videoBuffer;
                 
                 ret= _encode->encode ( frame, &bitstream);
                 if(!bitstream.len && _prestoring) 
                 {
                   printf("Frame skipped\n");
                   _prestore++;
                   return 1;
                 }
                 _prestoring=0;
                 bitstream.dtsFrame=frame-_prestore;
		 if(!ret)
                 {
                        printf("OgmWrite: Error encoding frame %d\n",frame);
                        return 0;
                 }
                encoding_gui->setFrame(frame,bitstream.len,bitstream.out_quantizer,_togo);
                return videoStream->write(bitstream.len,_videoBuffer,bitstream.flags,bitstream.dtsFrame);
}
//___________________________________________________
ADM_ogmWriteProcess::ADM_ogmWriteProcess( void)
{
	_incoming=NULL;
	_encode=NULL;
}
//___________________________________________________
ADM_ogmWriteProcess::~ADM_ogmWriteProcess()
{
	if(_incoming) delete _incoming;
	if(_encode) delete   _encode;

}
//EOF

