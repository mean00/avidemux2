/***************************************************************************
                          op_savecopy.h  -  description
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
 #ifndef __AVI_SAVECPY__
 #define   __AVI_SAVECPY__
#include "ADM_encoder/adm_encCopy.h"
 class GenericAviSaveCopy : public   GenericAviSave
 {
     protected :
		                                EncoderCopy          *copy;	
		          virtual uint8_t 	setupVideo( char *name  );
                          virtual uint8_t 	writeVideoChunk(uint32_t frame );
                                  uint8_t       newFile(void);

     public:
                                              GenericAviSaveCopy()  :     GenericAviSave()
                                                                      {
                                                                        copy=NULL;
                                                                      };
                           virtual ~GenericAviSaveCopy();
   };

 class GenericAviSaveCopyDualAudio : public   GenericAviSaveCopy
 {
     protected :

                        
                        char				*_trackname;
                        uint32_t			_audioCurrent2;

                        uint8_t    doOneTrack (uint32_t index,void *stream,uint32_t target,uint32_t *current);
                                    virtual uint8_t setupAudio( void);
                                  virtual uint8_t writeAudioChunk(uint32_t frame );

     public:
                                     GenericAviSaveCopyDualAudio(void	*track);

   };
/*            Pack /unpack */
class GenericAviSaveCopyUnpack : public   GenericAviSaveCopy
 {
     protected :
                          virtual uint8_t 	setupVideo( char *name  );
                          virtual uint8_t 	writeVideoChunk(uint32_t frame );
     public:
  
 };
class GenericAviSaveCopyPack : public   GenericAviSaveCopy
 {
     protected :
                          EncoderCopy          *copy;
                          ADMBitstream         *lookAhead[2];
                          uint32_t              curToggle;
                          uint32_t              time_inc;
                          virtual uint8_t 	setupVideo( char *name  );
                          virtual uint8_t 	writeVideoChunk(uint32_t frame );
                                  uint8_t       prefetch(uint32_t buffer,uint32_t frame);
     public:
                          virtual	~GenericAviSaveCopyPack();	
                                        GenericAviSaveCopyPack()  :     GenericAviSaveCopy()
                                        {
                                                lookAhead[0]=NULL;
                                                lookAhead[1]=NULL;
                                                curToggle=0;
                                                time_inc=0;
                                        };
  
 };
  #endif
