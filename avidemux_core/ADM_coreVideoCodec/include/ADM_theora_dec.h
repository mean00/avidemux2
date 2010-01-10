/***************************************************************************
                          ADM_theora_dec.h  -  description
                             -------------------
    begin                : Thu Sep 26 2002
    copyright            : (C) 2002 by mean
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
#ifdef USE_THEORA
#ifndef __theora__
#define __theora__

extern "C"
{
#include "theora/theora.h"
}
class decoderTheora:public decoders
{
protected:


  theora_info _tinfo;
  theora_state _tstate;



public:
    decoderTheora (uint32_t w, uint32_t h);
    virtual ~ decoderTheora ();
  virtual uint8_t uncompress (uint8_t * in, uint8_t * out, uint32_t len,
			      uint32_t * flag = NULL);
  virtual void setParam (void);


};

#endif
#endif
