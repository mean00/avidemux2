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
#ifndef ADM_huff_ENCODER_H
#define ADM_huff_ENCODER_H
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "huff_encoder.h"
/**
        \class ADM_huffEncoder
        \brief Dummy encoder that does nothing

*/
extern huff_encoder huffType;
class ADM_huffEncoder : public ADM_coreVideoEncoderFFmpeg
{
protected:
               int              plane;
public:

                           ADM_huffEncoder(ADM_coreVideoFilter *src,bool globalHeader);
                           ~ADM_huffEncoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void);
virtual        bool         getExtraData(uint32_t *l,uint8_t **d);
};


#endif
