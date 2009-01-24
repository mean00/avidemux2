/***************************************************************************
                          op_avisavecopy.cpp  -  description
                             -------------------

	We bypass the use of _incoming to have easy access to furure
	frame.
	In fact only the getflags is necessary, other stuff will be done throught
	incoming.

    begin                : Fri May 3 2002
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
#include "config.h"
#include "ADM_default.h"
#include "ADM_threads.h"

#include "fourcc.h"
#include "avi_vars.h"
#include "DIA_coreToolkit.h"

//#include "avilist.h"

#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"

#include "ADM_encoder/ADM_vidEncode.hxx"

#include "ADM_audio/aviaudio.hxx"
#include "ADM_audiofilter/audioprocess.hxx"
#include "op_aviwrite.hxx"
#include "op_avisave.h"
#include "op_savecopy.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_SAVE_AVI
#include "ADM_osSupport/ADM_debug.h"


/**
      \fn  ~GenericAviSaveCopyUnpack
      \brief destructor
*/
GenericAviSaveCopy::~GenericAviSaveCopy ()
{
      if(copy) delete copy;
      copy=NULL; 
}

uint8_t GenericAviSaveCopy::setupVideo (char *name)
{
  //  Setup avi file output, all is coming from original avi
  // since we are inc copy mode
  memcpy(&_bih,video_body->getBIH (),sizeof(_bih));
  _bih.biSize=sizeof(_bih);  //fix old version of avidemux
  _bih.biXPelsPerMeter=_bih.biClrUsed=_bih.biYPelsPerMeter=0;
  //
  memcpy(&_videostreamheader,video_body->getVideoStreamHeader (),sizeof( _videostreamheader));
  memcpy(&_mainaviheader,video_body->getMainHeader (),sizeof(_mainaviheader));
  

  /* update to fix earlier bug */
   _mainaviheader.dwWidth=_bih.biWidth;
   _mainaviheader.dwHeight=_bih.biHeight;

   uint8_t *extraData;
   uint32_t extraLen;
  _lastIPFrameSent=0xfffffff;
   video_body->getExtraHeaderData(&extraLen,&extraData);

  	if (!writter->saveBegin (name,
			   &_mainaviheader,
			   frameEnd - frameStart + 1,
			   &_videostreamheader,
			   &_bih,
			   extraData,extraLen,
			   audio_filter,
			   audio_filter2
		))
    	{
          GUI_Error_HIG (QT_TR_NOOP("Cannot initiate save"), NULL);
      		return 0;
    	}
	if(audio_filter2)
	{
		printf("Second audio track present\n");
	}
	else
	{
		printf("Second audio track absent\n");
	}
 _incoming = getFirstVideoFilter (frameStart,frameEnd-frameStart);
 encoding_gui->setFps(_incoming->getInfo()->fps1000);
 encoding_gui->setPhasis(QT_TR_NOOP("Saving"));
 // Set up our copy codec ...
  copy=new EncoderCopy(NULL);
  if(!copy->configure(_incoming, 0))
  {
      printf("Copy cannot [configure] \n");
      return 0;
  }
  return 1;
}


// copy mode
// Basically ask a video frame and send it to writter
// If it contains b frame and frames have been re-ordered
// reorder them back ...
uint8_t
GenericAviSaveCopy::writeVideoChunk (uint32_t frame)
{
  
  uint8_t    ret1;
 ADMCompressedImage img;
 ADMBitstream bitstream;
 uint8_t seq;
      img.data=vbuffer;
      bitstream.bufferSize=_incoming->getInfo ()->width *   _incoming->getInfo ()->height * 3;
      bitstream.data=vbuffer;
      
      
       if(!video_body->isReordered(frameStart+frame))
      {
          ret1 = video_body->getFrame (frameStart + frame,&img,&seq);// vbuffer, &len,      &_videoFlag);
          _videoFlag=img.flags;
      }
      else
      {
           ret1=copy->encode(frame,&bitstream);
           img.dataLength=bitstream.len;
           _videoFlag=img.flags=bitstream.flags;
      }

  if (!ret1)
    return 0;

    // check for split
     // check for auto split
      // if so, we re-write the last I frame
      if(muxSize)
      	{
                        // we overshot the limit and it is a key frame
                // start a new chunk
                if(handleMuxSize() && (_videoFlag & AVI_KEY_FRAME))
                {		
                    uint8_t *extraData;
                    uint32_t extraLen;

                  video_body->getExtraHeaderData(&extraLen,&extraData);
      
                  if(!reigniteChunk(extraLen,extraData)) return 0;
                }
          }
  
  if(_videoFlag==AVI_KEY_FRAME)
          newFile();
  encoding_gui->setFrame(frame,img.dataLength,0,frametogo);
  return writter->saveVideoFrame (img.dataLength, img.flags, img.data);


}
/**
      \fn newFile
      \brief start a new file, for example if the muxer was setup to split every 700 Meg. Call only on intra!


*/
uint8_t          GenericAviSaveCopy::newFile(void)
{
        if(muxSize)
      	{
                        // we overshot the limit and it is a key frame
                // start a new chunk
                if(handleMuxSize() )
                {		
                    uint8_t *extraData;
                    uint32_t extraLen;

                  video_body->getExtraHeaderData(&extraLen,&extraData);
      
                  if(!reigniteChunk(extraLen,extraData)) return 0;
                }
          }
    return 1;
}
// EOF
