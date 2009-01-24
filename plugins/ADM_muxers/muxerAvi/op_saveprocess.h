/***************************************************************************
                          op_saveprocess.h  -  description
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
 #ifndef __AVI_SAVEPRC__
 #define   __AVI_SAVEPRC__




 class GenericAviSaveProcess : public   GenericAviSave
 {
     protected :
					    
			Encoder 	*_encode;
			uint8_t		_notnull;
                       	char 		*TwoPassLogFile;
                        ADMBitstream    bitstream;

			virtual uint8_t	setupVideo( char *name  );
			virtual uint8_t	writeVideoChunk(uint32_t frame );
                         
     public:
     					GenericAviSaveProcess( void ) ;
                          virtual 	~GenericAviSaveProcess();
   };


  #endif
