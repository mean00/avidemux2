/***************************************************************************
                          op_savesmart.hxx  -  description
                             -------------------
    begin                : Mon May 6 2002
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
 #ifndef __AVI_SAVESMT__
 #define   __AVI_SAVESMT__

 class GenericAviSaveSmart : public   GenericAviSave
 {
     protected :
                                uint32_t        _hasBframe;
     #warning HARDCODED MAX IMAGE SIZE
     				
				ADMImage	*aImage;
				uint32_t	_nextip;
				uint8_t 	initEncoder(uint32_t qz );
				uint8_t 	stopEncoder(void  );
       				uint32_t 	encoderReady;
              			uint32_t 	compEngaged;
	virtual 		uint8_t 	setupVideo( char *name  );
 	virtual 		uint8_t 	writeVideoChunk(uint32_t frame );
				uint8_t		 writeVideoChunk_recode (uint32_t frame);
				uint8_t		 writeVideoChunk_copy (uint32_t frame,uint32_t first=0);
              			encoder 	*_encoder;
              			uint8_t 	_cqReenc          ;
				
     public:
     							
     						GenericAviSaveSmart(uint32_t qfactor);		
                          virtual 		~GenericAviSaveSmart();
			  uint8_t 		seekNextRef(uint32_t frame,uint32_t *nextip);
   };


  #endif
