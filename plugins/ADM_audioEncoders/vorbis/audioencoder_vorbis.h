
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
#ifndef AUDMaudioVorbis
#define AUDMaudioVorbis

 //_____________________________________________
class AUDMEncoder_Vorbis : public AUDMEncoder
{
  protected:
   
    void              *_handle;
    uint64_t          _oldpos;

         
  public:
            uint8_t     initialize(void);
            virtual     ~AUDMEncoder_Vorbis();
                        AUDMEncoder_Vorbis(AUDMAudioFilter *instream);	
            
   virtual uint8_t	getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples);
};

#endif
