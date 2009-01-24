/***************************************************************************
                          ADM_vidDeinterlace.h  -  description
                             -------------------
    begin                : Sat Apr 20 2002
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
 #include "ADM_vidField.h"

 class  ADMVideoDeinterlace:public ADMVideoFields
 {

 protected:
    		     							

 public:
 		

						ADMVideoDeinterlace(  AVDMGenericVideoStream *in,CONFcouple *setup);

  			virtual 		~ADMVideoDeinterlace();
		        virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          										ADMImage *data,uint32_t *flags);

			virtual char 	*printConf(void);
	
 }     ;



