/***************************************************************************
                          \fn ADM_VideoEncoders
                          \brief Internal handling of video encoders
                             -------------------

    copyright            : (C) 2002/2009 by mean
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
#pragma once
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "ffVaEnc.h"




#define VAENC_CONF_DEFAULT \
{ \
		0, \
                10000, \
                20000,\
	}



/**
        \class ADM_ffVaEncEncoder
        \brief Dummy encoder that does nothing

*/
class ADM_ffVaEncEncoder : public ADM_coreVideoEncoderFFmpeg
{
protected:

               uint8_t      *nv12;
               int          nv12Stride;               
public:

                           ADM_ffVaEncEncoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~ADM_ffVaEncEncoder();
virtual        bool        configureContext(void);
virtual        bool        setup(void);
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "H264";}

virtual        bool         isDualPass(void) ;

};

