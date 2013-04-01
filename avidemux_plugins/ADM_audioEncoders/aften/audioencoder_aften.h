
/***************************************************************************
    copyright            : (C) 2002-6 by mean
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
#ifndef AUDMaudioAften
#define AUDMaudioAften
#include "aften_encoder.h"
/**
    \class AUDMEncoder_Aften
    \brief Class wrapping aften ac3 encoder
*/
class AUDMEncoder_Aften : public ADM_AudioEncoder
{
protected:
         void           *_handle;
         uint32_t       _chunk;
         bool           _globalHeader;
         aften_encoder  _config;
         bool           reorder(float *sample_in,float *sample_out,int samplePerChannel,CHANNEL_TYPE *mapIn,CHANNEL_TYPE *mapOut);
         float          *ordered;
public:
                bool    initialize(void);
    virtual     bool    encode(uint8_t *dest, uint32_t *len, uint32_t *samples);
    virtual             ~AUDMEncoder_Aften();
                        AUDMEncoder_Aften(AUDMAudioFilter *instream,bool globalHeader,CONFcouple *setup);
};
#endif

