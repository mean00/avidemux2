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
#include "ADM_coreVideoEncoder.h"
#include "ADM_coreLibVA/ADM_coreLibVA.h"
#include "ADM_coreLibVA_encodingContext.h"

/**
        \class ADM_libvaEncoder
        \brief Dummy encoder that does nothing

*/
class ADM_libvaEncoder : public ADM_coreVideoEncoder
{
protected:
               ADM_vaEncodingContext     *vaContext;
public:

                            ADM_libvaEncoder(ADM_coreVideoFilter *src,bool globalHeader);
                            ~ADM_libvaEncoder();
virtual        bool         encode (ADMBitstream * out);
virtual const  char         *getFourcc(void) {return "H264";}
virtual        bool         setup(void);
virtual        bool         getExtraData(uint32_t *l,uint8_t **d);

protected:
                int         extraDataSize;
                uint8_t     *extraData;
                bool        globalHeader;



};


