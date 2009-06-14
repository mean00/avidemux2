
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
#ifndef AUDMaudioLavcodec
#define AUDMaudioLavcodec
 //_____________________________________________
class AUDMEncoder_Lavcodec : public ADM_AudioEncoder
{
  protected:
   
    void              *_context;
    uint32_t            _chunk;
    
         
  public:
            bool        initialize(void);
   virtual             ~AUDMEncoder_Lavcodec();
                        AUDMEncoder_Lavcodec(AUDMAudioFilter *instream);	
   virtual bool  	    encode(uint8_t *dest, uint32_t *len, uint32_t *samples);
};

#endif
