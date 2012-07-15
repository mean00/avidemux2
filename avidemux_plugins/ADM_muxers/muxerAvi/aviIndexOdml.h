/***************************************************************************
                 \file         aviIndexOdml.h
                 \brief        Write odml / avi type2 index/indeces
                 \author       mean fixounet@free.Fr (c) 2012
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
#include "aviIndex.h"


/**
    \class aviIndexBase
*/
class aviIndexOdml : public aviIndexBase
{
protected:
public:
                        aviIndexOdml(aviWrite *father) ;
           virtual      ~aviIndexOdml();
           virtual bool  addVideoFrame( int len,uint32_t flags,const uint8_t *data);
           virtual bool  addAudioFrame(int trackNo, int len,uint32_t flags,const uint8_t *data);
           virtual bool  writeIndex();
};