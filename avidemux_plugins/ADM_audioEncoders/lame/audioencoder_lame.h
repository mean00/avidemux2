
/***************************************************************************
    \file audioencoder_lame.h
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
#ifndef AUDMaudioLame_H
#define AUDMaudioLame_H
#include "lame_encoder.h"
 /**
        \class AUDMEncoder_Lame
        \brief Front end for libmp3lame
*/
class AUDMEncoder_Lame : public ADM_AudioEncoder
{
  protected:
   
    void              *lameFlags;
    uint32_t          _chunk; // Nb of float we encode each time
         
  public:
    virtual             ~AUDMEncoder_Lame();
                        AUDMEncoder_Lame(AUDMAudioFilter *instream,bool globalHeader);	
            bool	    isVBR(void );
            
   virtual bool         encode(uint8_t *dest, uint32_t *len, uint32_t *samples);
   virtual bool         initialize(void);
};

#endif
