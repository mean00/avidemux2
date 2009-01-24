/***************************************************************************
                          op_saveprocess.cpp  -  description
                             -------------------
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

#include "ADM_audio/aviaudio.hxx"
#include "ADM_audiofilter/audioprocess.hxx"

#include "ADM_videoFilter.h"
//#include "ADM_gui/GUI_encoder.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_encoder/ADM_vidEncode.hxx"

#include "op_aviwrite.hxx"
#include "op_avisave.h"

#include "ADM_encoder/adm_encoder.h"
#include "op_saveprocess.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_SAVE_AVI
#include "ADM_osSupport/ADM_debug.h"

GenericAviSaveProcess::GenericAviSaveProcess( void ) 
{
	TwoPassLogFile=NULL;
	_incoming=NULL;
	_encode=NULL;
	_videoProcess=1;
};

uint8_t
GenericAviSaveProcess::setupVideo (char *name)
{
	_notnull=0;
	_incoming = getLastVideoFilter (frameStart,frameEnd-frameStart);
 	frametogo=_incoming->getInfo()->nb_frames;
	encoding_gui->setFps(_incoming->getInfo()->fps1000);
	// anish
 	if(_incoming->getInfo()->width%8)
		{
                  if(!GUI_Question(QT_TR_NOOP("Width is not a multiple of 8\n continue anyway ?")))
			return 0;

		}

  _encode = getVideoEncoder (_incoming->getInfo()->width,_incoming->getInfo()->height);
  if (!_encode)
    return 0;

  // init compressor
  TwoPassLogFile=new char[strlen(name)+6];
  strcpy(TwoPassLogFile,name);
  strcat(TwoPassLogFile,".stat");
  _encode->setLogFile(TwoPassLogFile,frametogo);

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
 
  memcpy (&_bih, video_body->getBIH (), sizeof (_bih));
  _bih.biWidth = _incoming->getInfo ()->width;
  _bih.biHeight = _incoming->getInfo ()->height;
  _bih.biSize=sizeof(_bih);
  _bih.biXPelsPerMeter=_bih.biClrUsed=_bih.biYPelsPerMeter=0;

  _mainaviheader.dwTotalFrames= _incoming->getInfo ()->nb_frames;
_mainaviheader.dwMicroSecPerFrame=0;

  printf("\n Saved as %ld x %ld\n",_bih.biWidth,_bih.biHeight);
  _bih.biCompression=fourCC::get((uint8_t *)_encode->getCodecName());
   
  encoding_gui->setCodec(_encode->getDisplayName());
  
  // init save avi
//-----------------------2 Pass--------------------------------------
  if (_encode->isDualPass ())
    {
      uint8_t *buffer;
      uint32_t len, flag;

 	aprintf("\n** Dual pass encoding**\n");

	if(!reuse)
 	{
	
      	guiSetPhasis (QT_TR_NOOP("1st Pass"));
      	aprintf("**Pass 1:%lu\n",frametogo);
     	buffer = new uint8_t[_incoming->getInfo ()->width *
		    _incoming->getInfo ()->height * 3];

      	_encode->startPass1 ();
      //__________________________________
      //   now go to main loop.....
      //__________________________________
        bitstream.bufferSize=_incoming->getInfo ()->width *   _incoming->getInfo ()->height * 3;
        bitstream.data=buffer;
        for (uint32_t cf = 0; cf < frametogo; cf++)
        {
          if (guiUpdate (cf, frametogo))
            {
            abt:
                GUI_Error_HIG (QT_TR_NOOP("Aborting"), NULL);
              delete[]buffer;
              return 0;
            }
            
            bitstream.cleanup(cf);
            if (!_encode->encode (cf, &bitstream))
                {
                        printf("\n Encoding of frame %lu failed !\n",cf);
                        goto abt;
                }
           encoding_gui->setFrame(cf,bitstream.len,bitstream.out_quantizer,frametogo);
    
        }

	encoding_gui->reset();
      	delete[]buffer;	
     	aprintf("**Pass 1:done\n");
    }// End of reuse

      if(!_encode->startPass2 ())
      {
      	printf("Pass2 ignition failed\n");
      	return 0;
	}
   }   //-------------------------/VBR-----------------------------------
  // init save avi

// now we build the new stream !
    	aprintf("**main pass:\n");

		memcpy(&_videostreamheader,video_body->getVideoStreamHeader (),sizeof(_videostreamheader));
		memcpy(&_videostreamheader.fccHandler	,_encode->getFCCHandler(),4);
		_videostreamheader.fccType	=fourCC::get((uint8_t *)"vids");
		_videostreamheader.dwScale=1000;
		_videostreamheader.dwRate= _incoming->getInfo ()->fps1000;

    		memcpy(&_mainaviheader	,video_body->getMainHeader (),sizeof(_mainaviheader));


  		  _mainaviheader.dwWidth=_bih.biWidth;
    		_mainaviheader.dwHeight=_bih.biHeight;
    		_videostreamheader.dwQuality=10000;

    uint8_t *data;
    uint32_t dataLen=0;

    _encode->hasExtraHeaderData( &dataLen,&data);
#if 0
  	if (!writter->saveBegin (name,
			   &_mainaviheader,
			   frameEnd - frameStart + 1,
			   &_videostreamheader,
			   &_bih,
			   data,dataLen,
			   (AVDMGenericAudioStream *) audio_filter,
			   NULL))
    	{
      		return 0;
    	}
#endif
  aprintf("Setup video done\n");
  bitstream.data=vbuffer;
  bitstream.bufferSize=MAXIMUM_SIZE * MAXIMUM_SIZE * 3;
  return 1;
  //---------------------
}

//
//      Just to keep gcc happy....
//
GenericAviSaveProcess::~GenericAviSaveProcess ()
{
  cleanupAudio();
  if (_encode)
    delete      _encode;
  	_encode=NULL;
  if(TwoPassLogFile)
  {
  	delete [] TwoPassLogFile;
  	TwoPassLogFile=NULL;
  }
}

// copy mode
// Basically ask a video frame and send it to writter
uint8_t
GenericAviSaveProcess::writeVideoChunk (uint32_t frame)
{
  uint8_t    	ret1;
  // CBR or CQ
  if (frame == 0)
  {
    encoding_gui->setCodec(_encode->getDisplayName())  ;
  if (!_encode->isDualPass ())
  {
                          guiSetPhasis (QT_TR_NOOP("Encoding"));
        }
  else
          {
                          guiSetPhasis (QT_TR_NOOP("2nd Pass"));
        }
  }
  // first read
  bitstream.cleanup(frame);
  ret1 = _encode->encode ( frame,&bitstream);// &len1, vbuffer, &_videoFlag);
  _videoFlag=bitstream.flags;
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
                      uint8_t *data;
                      uint32_t dataLen=0;

                      _encode->hasExtraHeaderData( &dataLen,&data);	   
                      if(!reigniteChunk(dataLen,data)) return 0;
              }
        }
        encoding_gui->setFrame(frame,bitstream.len,bitstream.out_quantizer,frametogo);	
        // If we have several null B frames dont write them
        if(bitstream.len) _notnull=1;
	else	if( !_notnull)
	{
		printf("Frame : %lu dropped\n",frame);
	 	return 1;			 
	 }
         return writter->saveVideoFrame (bitstream.len, _videoFlag, vbuffer);

}


// EOF
