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
#ifndef ADM_jpeg_ENCODER_H
#define ADM_jpeg_ENCODER_H
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "jpeg_encoder.h"
/**
        \class ADM_jpegEncoder
        \brief Dummy encoder that does nothing

*/
extern jpeg_encoder jpegConf;
class ADM_jpegEncoder : public ADM_coreVideoEncoderFFmpeg
{
protected:
               int              plane;
public:

                           ADM_jpegEncoder(ADM_coreVideoFilter *src,bool globalHeader);
                           ~ADM_jpegEncoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "jpeg";}
};


#endif
