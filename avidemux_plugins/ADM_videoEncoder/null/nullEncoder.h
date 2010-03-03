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
#ifndef ADM_null_ENCODER_H
#define ADM_null_ENCODER_H
#include "ADM_coreVideoEncoder.h"
/**
        \class ADM_nullEncoder
        \brief Dummy encoder that does nothing

*/

class ADM_nullEncoder : public ADM_coreVideoEncoder
{
protected:
public:

                           ADM_nullEncoder(ADM_coreVideoFilter *src,bool globalHeader);
                           ~ADM_nullEncoder();
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void);
};


#endif
