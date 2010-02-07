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
#ifndef ADM_ffMpeg2_ENCODER_H
#define ADM_ffMpeg2_ENCODER_H
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "mpeg2_encoder.h"

typedef enum
{
        MPEG2_MATRIX_DEFAULT,
        MPEG2_MATRIX_TMPGENC,
        MPEG2_MATRIX_ANIME,
        MPEG2_MATRIX_KVCD,
        MPEG2_MATRIX_LAST
} ;

/**
        \class ADM_ffMpeg2Encoder
        \brief Dummy encoder that does nothing

*/
class ADM_ffMpeg2Encoder : public ADM_coreVideoEncoderFFmpeg
{
protected:
               int             plane;
               mpeg2_encoder   settings;
public:

                           ADM_ffMpeg2Encoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~ADM_ffMpeg2Encoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "MPEG";}

virtual        bool         isDualPass(void) ;

};

#endif
