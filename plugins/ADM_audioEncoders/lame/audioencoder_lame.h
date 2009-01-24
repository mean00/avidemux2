
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
#ifndef AUDMaudioLame
#define AUDMaudioLame

 //_____________________________________________
class AUDMEncoder_Lame : public AUDMEncoder
{
  protected:
   
    void              *lameFlags;
    
         
  public:
//            uint8_t     init(ADM_audioEncoderDescriptor *config);
    virtual             ~AUDMEncoder_Lame();
                        AUDMEncoder_Lame(AUDMAudioFilter *instream);	
            uint8_t	isVBR(void );
            
   virtual uint8_t	getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples);
   virtual uint8_t  initialize(void);
};

#endif
