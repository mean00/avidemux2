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
#ifndef ADM_xvid4_H
#define ADM_xvid4_H
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"
#include "xvid4_encoder.h"
/**
        \class ADM_ffMpeg4Encoder
        \brief Dummy encoder that does nothing

*/
class xvid4Encoder : public ADM_coreVideoEncoder
{
protected:
               
           
               int             plane;
               
public:

                           xvid4Encoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~xvid4Encoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "DIVX";}

virtual        bool         isDualPass(void) ;

};

#endif
