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
#include "config.h"


#include "fourcc.h"
#include "avi_vars.h"
#include "ADM_default.h"
#include "ADM_assert.h"

//#include "avilist.h"

#include "ADM_videoFilter.h"

#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_audio/aviaudio.hxx"
#include "ADM_audiofilter/audioprocess.hxx"


#include "op_ogsave.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME 0
#include "ADM_osSupport/ADM_debug.h"


uint8_t	ADM_ogmWriteCopy::initVideo(const char *name)
{
uint32_t w,h;	
int64_t dur64;
uint32_t dur32;
uint16_t dur16;
		_togo=frameEnd-frameStart;
		stream_header header;
		
		_videoBuffer=new uint8_t[avifileinfo->width*avifileinfo->height*3];

		

		
		memset(&header,0,sizeof(header));
		
		memcpy(&(header.streamtype),"video\0\0\0",8);
		MEMCPY(&(header.subtype),&(avifileinfo->fcc),4);
		
		encoding_gui->setCodec(QT_TR_NOOP("Copy"));
		encoding_gui->setPhasis(QT_TR_NOOP("Saving..."));
		
		//header.size=sizeof(header);
		dur32=sizeof(header);
		MEMCPY(&header.size,&dur32,4);
		
		w=avifileinfo->width;
		h=avifileinfo->height;
		MEMCPY(&(header.video.width),&w,4);
		MEMCPY(&(header.video.height),&h,4);
				
		// Timing ..
		double duration; // duration in 10us
		_fps1000=avifileinfo->fps1000;
		duration=avifileinfo->fps1000;
		duration=1000./duration;
		duration*=1000*1000;
		duration*=10;
		
		
		dur64=(int64_t)duration;
		
		MEMCPY(&header.time_unit,&dur64,8);
		
		dur64=1;
		MEMCPY(&header.samples_per_unit,&dur64,8);
		
		dur32=0x10000;
		MEMCPY(&header.buffersize,&dur32,4);
		//header.buffersize=0x10000;
		
		//header.bits_per_sample=24;
		dur16=24;
		MEMCPY(&header.bits_per_sample,&dur16,2);
		
		dur16=1;
		//header.default_len=1;
		dur32=1;
		MEMCPY(&header.default_len,&dur32,4);
		
		_lastIPFrameSent=0xfffffff;
		
		return videoStream->writeHeaders(sizeof(header),(uint8_t *)&header); // +4 ?

}
//___________________________________________________
uint8_t	ADM_ogmWriteCopy::writeVideo(uint32_t frame)
{
uint32_t len,flags;
uint32_t forward;
uint8_t ret1=0;
ADMCompressedImage img;

              img.data=_videoBuffer;

		// Check for B_frames
		if(!video_body->isReordered(frameStart+frame))
		{

			if(!  video_body->getFrameNoAlloc (frameStart+frame,&img))// _videoBuffer, &len,     &flags)) 
				return 0;		
			encoding_gui->setFrame(frame,img.dataLength,0,_togo);
			return videoStream->write(img.dataLength,img.data,img.flags,frame);
		}
		
		// we DO have b frame
		// 
		video_body->getFlags (frameStart + frame, &flags);
		if(flags & AVI_B_FRAME)
			{ 	// search if we have to send a I/P frame in adance
				
				uint32_t forward;
				forward=searchForward(frameStart+frame);
				// if we did not sent it, do it now
				if(forward!=_lastIPFrameSent)
				{
					aprintf("\tP Frame not sent, sending it :%lu\n",forward);
					ret1 = video_body->getFrameNoAlloc (forward,&img);// _videoBuffer, &len,&flags);
					_lastIPFrameSent=forward;

				}
				else
				{
					// we already sent it :)
					// send n-1
					aprintf("\tP Frame already, sending  :%lu\n",frameStart+frame-1);
					ret1 = video_body->getFrameNoAlloc (frameStart+frame-1,&img);// _videoBuffer, &len,&flags);

				}

			}
			else // it is not a B frame and we have nothing on hold, sent it..
			{
				// send n-1 if we reach the fwd reference frame
				if((frame+frameStart)==_lastIPFrameSent)
				{
					aprintf("\tSending Last B-frame :(%lu)\n",frameStart + frame-1);
					ret1 = video_body->getFrameNoAlloc (frameStart + frame-1, &img);//_videoBuffer, &len,		&flags);

				}
				else
				{
					aprintf("\tJust sending it :(%lu)-(%lu)\n",frameStart + frame,_lastIPFrameSent);
					ret1 = video_body->getFrameNoAlloc (frameStart + frame,&img);// _videoBuffer, &len, 		&flags);

				}
			}
                encoding_gui->setFrame(frame,img.dataLength,0,_togo);
		return videoStream->write(img.dataLength,img.data,img.flags,frame);
		return ret1;

}
// Return the next non B frame
// 0 if not found
//___________________________________________________
uint32_t ADM_ogmWriteCopy::searchForward(uint32_t startframe)
{
		uint32_t fw=startframe;
		uint32_t flags;
		uint8_t r;

			while(1)
			{
				fw++;
				r=video_body->getFlags (fw, &flags);
				if(!(flags & AVI_B_FRAME))
				{
					return fw;

				}
				ADM_assert(r);
				if(!r)
				{
					printf("\n Could not locate last non B frame \n");
					return 0;
				}

			}
}
//___________________________________________________
ADM_ogmWriteCopy::ADM_ogmWriteCopy( void)
{
	_lastIPFrameSent=0;

}
//___________________________________________________
ADM_ogmWriteCopy::~ADM_ogmWriteCopy()
{


}

