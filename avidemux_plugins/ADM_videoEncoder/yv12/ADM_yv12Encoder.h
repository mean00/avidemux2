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
#ifndef ADM_YV12_ENCODER_H
#define ADM_YV12_ENCODER_H
#include "ADM_coreVideoEncoder.h"


/**
        \class ADM_yv12Encoder
        \brief Dummy encoder that does nothing

*/
class ADM_yv12Encoder : public ADM_coreVideoEncoder
{
protected:
               int plane;
public:

                            ADM_yv12Encoder(ADM_coreVideoFilter *src,bool globalHeader);
                            ~ADM_yv12Encoder();
virtual        bool         encode (ADMBitstream * out);
virtual const  char         *getFourcc(void) {return "YV12";}
};


#endif
