/***************************************************************************
    \file audioencoder_twolame.h
    copyright            : (C) 2002-9 by mean
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
#ifndef AUDMaudioTwoLame
#define AUDMaudioTwoLame
#include "twolame_encoder.h"
/**
    \class AUDMEncoder_Twolame
*/
class AUDMEncoder_Twolame : public ADM_AudioEncoder
{
  protected:
    void           *_twolameOptions;
    uint32_t        _chunk;
    lame_encoder    _config;     
  public:
            bool     initialize(void);
    virtual             ~AUDMEncoder_Twolame();
                        AUDMEncoder_Twolame(AUDMAudioFilter *instream,bool globalHeader,CONFcouple *setup);
    virtual bool    encode(uint8_t *dest, uint32_t *len, uint32_t *samples);
    virtual bool    isVBR(void) {return false;}
};

#endif
