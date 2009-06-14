/***************************************************************************
                          op_avisave.cpp  -  description
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
 
 /*
* MODIFIED BY GMV 30.1.05: prepared for ODML
*/
 


#include "ADM_default.h"
#include "ADM_threads.h"

#include "fourcc.h"

#include "DIA_coreToolkit.h"

//#include "avilist.h"

#include "ADM_videoFilter.h"

#include "ADM_videoFilter.h"




#include "op_aviwrite.hxx"
#include "op_avisave.h"
#include "GUI_mux.h"
#include <math.h>
uint32_t muxSize=4090;
extern PARAM_MUX muxMode;
extern int muxParam;



#include "audioeng_buildfilters.h"
#include "ADM_coreUI/include/DIA_factory.h"
const char *getStrFromAudioCodec( uint32_t codec);
//_________________________
uint8_t ADM_aviUISetMuxer(  void )
{
  
//	return DIA_setUserMuxParam ((int *) &muxMode, (int *) &muxParam, (int *) &muxSize);
  uint32_t mux_n_frame=muxParam;
  uint32_t mux_size_block=muxParam;
  uint32_t mux_mode=(uint32_t)muxMode;
  
  
  diaMenuEntry muxingType[]={
  {MUX_REGULAR,QT_TR_NOOP("Normal")},
  {MUX_N_FRAMES,QT_TR_NOOP("Mux every N video frames")},
  {MUX_N_BYTES,QT_TR_NOOP("Mux by packet size")}
  };
  
    diaElemMenu      mux(&(mux_mode),QT_TR_NOOP("Muxing _type:"),3,muxingType);
    diaElemUInteger blockSize(&(muxSize),QT_TR_NOOP("_Split every MB:"),1,9000);
    
    diaElemUInteger n_frames(&(mux_n_frame),QT_TR_NOOP("Mux _every x video frames:"),1,100);
    diaElemUInteger n_block(&(mux_size_block),QT_TR_NOOP("Mux in _blocks of x bytes:"),1,50000);
    
    
     mux.link(&(muxingType[1]),1,&n_frames);
     mux.link(&(muxingType[2]),1,&n_block);
    
     diaElem *elems[4]={&mux,&n_frames,&n_block,&blockSize};
     if( diaFactoryRun(QT_TR_NOOP("AVI Muxer Options"),4,elems))
    {
      muxMode=(PARAM_MUX)mux_mode;
      switch(muxMode)
      {
        case MUX_REGULAR: muxParam=1;break;
        case MUX_N_FRAMES: muxParam=mux_n_frame;break;
        case MUX_N_BYTES: muxParam=mux_size_block;break;
        default: ADM_assert(0);
      }
      return 1;
    }
    return 0;
};


//_______ set the autosplit size
uint8_t ADM_aviSetSplitSize(uint32_t size)
{
	muxSize=size;
	return 1;
}

GenericAviSave::GenericAviSave ()
{

  has_audio_track = has_audio_vbr = 0;
 vbuffer = new uint8_t[MAXIMUM_SIZE * MAXIMUM_SIZE * 3];
//  vbuffer=new ADMImage(MAXIMUM_SIZE,MAXIMUM_SIZE);
  abuffer = new uint8_t[96000];

  ADM_assert (vbuffer);
  ADM_assert (abuffer);

  audio_filter=NULL;
  audio_filter2=NULL;
  _part=0;
  dialog_work=NULL;
  _lastIPFrameSent=0xffffff;
  _incoming=NULL;
  encoding_gui=NULL;
  _videoProcess=0;
  _audioCurrent=_audioTarget=0;
 _audioTotal=0;  
 _file=NULL;
 _pq=NULL;
 memset(&_context,0,sizeof(_context));
 _context.audioDone=1;
}

GenericAviSave::~GenericAviSave ()
{
  cleanupAudio();
  delete[] vbuffer;
  delete[] abuffer;
  _incoming=NULL;
  ADM_assert(!_file);
}
uint8_t GenericAviSave::cleanupAudio (void)
{
  printf("[AVI] Cleaning audio\n");
  if(_pq)
  {
    _pq->Abort();
    while(!_context.audioDone)
    {
      printf("Waiting Audio thread\n");
      ADM_usleep(500000); 
    }
    if(_pq) delete _pq;
    _pq=NULL;
  }
  if(audio_filter)
  {
//    deleteAudioFilter (audio_filter);
    audio_filter=NULL;
  }
  return 1;
}
//___________________________________________________________
//      Generic Save Avi loop
//
//___________________________________________________________
//
uint8_t  GenericAviSave::saveAvi (const char *name)
{
uint32_t size;
uint8_t ret=0;
  strcpy(_name,name);
  //frametogo = frameEnd - frameStart + 1;
  frametogo=0;
  
  writter = new aviWrite ();
    // 1- setup audio
  guiStart();
  if (!setupAudio ())
    {
      guiStop();
      GUI_Error_HIG (QT_TR_NOOP("Error initalizing audio filters"), NULL);
      delete writter;
      writter = NULL;
      return 0;
    }
   
   if (!setupVideo (_name))
    {
      guiStop();
      GUI_Error_HIG (QT_TR_NOOP("Error initalizing video filters"), NULL);
      delete   	writter;
      writter = NULL;
     // guiStop();
      return 0;
    }
  
  // 3- setup video
  frametogo=_incoming->getInfo()->nb_frames;
  fps1000=_incoming->getInfo()->fps1000;
  printf ("\n writing %lu frames\n", frametogo);

  //__________________________________
  //   now go to main loop.....
  //__________________________________
  for (uint32_t cf = 0; cf < frametogo; cf++) 
    {
			
			
			
      			if (guiUpdate (cf, frametogo))
					goto abortme;
      			//   printf("\n %lu / %lu",cf,frametogo);
      			writeVideoChunk (cf);
      			writeAudioChunk (cf);
			//writter->sync();
     
     
    };				// end for
    ret=1;
abortme:
  guiStop ();
  //__________________________________
  // and end save
  //__________________________________
  writter->setEnd ();
  delete       writter;
  writter = NULL;
  // resync GUI
  printf ("\n Saving AVI (v_engine)... done\n");
  return ret;
}

//_________________________________________________________________
//
//                                                              Set up audio system
//_________________________________________________________________
uint8_t
GenericAviSave::setupAudio (void)
{
// 1- Prepare audio filter
//__________________________

  _audioInBuffer = 0;
  _audioTarget=_audioCurrent=0;
  _audioTotal=0;
  audio_filter=NULL;
   if(!currentaudiostream) 
   {
   	encoding_gui->setAudioCodec(QT_TR_NOOP("None"));
	return 1;
   }
  printf (" mux mode : %d mux param %d\n", muxMode, muxParam);

  if (audioProcessMode())	// else Raw copy mode
    {
//      audio_filter = buildAudioFilter (currentaudiostream,video_body->getTime (frameStart));
      if(!audio_filter) return 0;
      encoding_gui->setAudioCodec(getStrFromAudioCodec(audio_filter->getInfo()->encoding));
    }
  else // copymode
    {
      // else prepare the incoming raw stream
      // audio copy mode here
      encoding_gui->setAudioCodec(QT_TR_NOOP("Copy"));
//      audio_filter=buildAudioFilter( currentaudiostream,video_body->getTime (frameStart));
      if(!audio_filter) return 0;
    }
    /* Setup audioQ */
    pthread_t     audioThread;
    _pq=new PacketQueue("AVI audioQ",5000,2*1024*1024);
    memset(&_context,0,sizeof(_context));
   // _context.audioEncoder=audio_filter;
    _context.audioTargetSample=0xFFFF0000; ; //FIXME
    _context.packetQueue=_pq;
    // start audio thread
    ADM_assert(!pthread_create(&audioThread,NULL,(THRINP)defaultAudioQueueSlave,&_context)); 
    ADM_usleep(4000);
  return 1;
}
//---------------------------------------------------------------------------
uint8_t
GenericAviSave::writeAudioChunk (uint32_t frame)
{
  uint32_t    len;
  // if there is no audio, we do nothing
  if (!audio_filter)
    return 1;
    
  double t;
  
  t=frame+1;
  t=t/fps1000;
  t=t*1000*audio_filter->getInfo()->frequency;
  _audioTarget=(uint32_t )floor(t);

        uint32_t sample,packetLen,packets=0;


       
        sample=0;
        // _audioTarget is the # of sample we want
        while(_audioCurrent<_audioTarget)
        {
                if(!_pq->Pop(abuffer+_audioInBuffer,&packetLen,&sample))
                  {
                    printf("AVIWR:Could not read packet\n");
                    break;
                  }
                _audioInBuffer+=packetLen;
                _audioTotal+=packetLen;
                _audioCurrent+=sample;		
                packets++;
        }
      switch (muxMode)
        {
        case MUX_N_FRAMES:
          stored_audio_frame++;
          if (stored_audio_frame < muxParam)
            return 1;
          stored_audio_frame = 0;
        case MUX_REGULAR:
          break;
        case MUX_N_BYTES:
                if(_audioInBuffer<muxParam) return 1;
                break;
          break;
        default:
          ADM_assert (0);
        }
      if (_audioInBuffer)
        {
          writter->saveAudioFrame (_audioInBuffer, abuffer);
          encoding_gui->setAudioSize(_audioTotal);
          _audioInBuffer=0;
        }
      return 1;
  
}

void
GenericAviSave::guiStart (void)
{
      encoding_gui=new DIA_encoding(25000);
      encoding_gui->setCodec(QT_TR_NOOP("Copy"));
      encoding_gui->setFrame (0,0,2 ,100); // FXMe
      encoding_gui->setContainer(QT_TR_NOOP("AVI"));

}

void
GenericAviSave::guiStop (void)
{
      ADM_assert(encoding_gui);
      delete encoding_gui;
      encoding_gui=NULL;

}
void GenericAviSave::guiSetPhasis(const char *str)
{
      ADM_assert(encoding_gui);
      encoding_gui->setPhasis(str);
	
}
uint8_t
GenericAviSave::guiUpdate (uint32_t nb, uint32_t total)
{
  ADM_assert(encoding_gui);
  //encoding_gui->setFrame (nb, 0,0, total); //FXMe
  if ( encoding_gui->isAlive () == 1)
    return 0;
  return 1;


}
//	Return 1 if we exceed the chunk limit
//
uint8_t  GenericAviSave::handleMuxSize ( void )
{
  uint32_t pos;
  
        pos=writter->getPos();
        if(pos>=muxSize*1024*1024)
                {
                          return 1  ;
                        
                }
                return 0;
  
      
}
//
//	Finish the current avi and start a new one
//
uint8_t   GenericAviSave::reigniteChunk( uint32_t dataLen, uint8_t *data )
{
	// MOD Feb 2005 by GMV: ODML exit
	if(writter->doODML!=aviWrite::NO)return 1;	// exit if odml has to be used
	// END MOD Feb 2005 by GMV
	
	    // first end up the current chunk
	     	writter->setEnd ();
  			delete       writter;
  			writter = NULL;
     	// then create a new one
         writter = new aviWrite ();
    
				_part++;
				char n[500];
				
				sprintf(n,"%s%02d",_name,_part);
								         
      	 printf("\n *** writing a new avi part :%s\n",n);
          
	        if (!writter->saveBegin (n,
			   &_mainaviheader,
			   frameEnd - frameStart + 1, 
			   &_videostreamheader,
			   &_bih,
			   data,dataLen,
			   audio_filter,
			   audio_filter2))
    {
      GUI_Error_HIG (QT_TR_NOOP("Cannot initiate save"), NULL);

      return 0;
    }
    return 1;
}

/**
	Search Forward Reference frame from the current B frame
*/
uint32_t GenericAviSave::searchForward(uint32_t startframe)
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

//---------------------------------------

// EOF
