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
#ifndef AUDMaudioPCM
#define AUDMaudioPCM

/*!
    This class is the float->PCM encoder.
    It is somehow special as it can alsa be a LPCM encoder and a bigendian/littleendian swapper
*/
class AUDMEncoder_PCM : public AUDMEncoder
{
  protected:
    uint32_t            revert;
         
  public:
            uint8_t     initialize(void);
            virtual     ~AUDMEncoder_PCM();
                        /*! \param reverted : Should the endianness be reverted compared to system  
                            \param fourCC   : FourCC to use (WAV_PCM/WAV_LPCM)
                        */
                         AUDMEncoder_PCM(AUDMAudioFilter * instream);
    virtual uint8_t	     getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples);
};

#endif
