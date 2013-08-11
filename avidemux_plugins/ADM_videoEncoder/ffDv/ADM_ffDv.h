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


/**
        \class ADM_ffDvEncoder
        \brief Dummy encoder that does nothing

*/
class ADM_ffDvEncoder : public ADM_coreVideoEncoderFFmpeg
{
protected:
               
           
               int             plane;
               
public:

                           ADM_ffDvEncoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~ADM_ffDvEncoder();
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "DVSD";}
               bool         setup(void);
virtual        bool         isDualPass(void)  {return false;}

};

