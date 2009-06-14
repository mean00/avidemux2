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
#ifndef ADM_PNG_ENCODER_H
#define ADM_PNG_ENCODER_H
#include "ADM_coreVideoEncoder.h"


/**
        \class ADM_pngEncoder
        \brief Dummy encoder that does nothing

*/
class ADM_pngEncoder : public ADM_coreVideoEncoder
{
protected:
               int plane;
public:

                            ADM_pngEncoder(ADM_coreVideoFilter *src);
                            ~ADM_pngEncoder();
virtual        bool         encode (ADMBitstream * out);
virtual const  char         *getFourcc(void) {return "png ";}
};


#endif