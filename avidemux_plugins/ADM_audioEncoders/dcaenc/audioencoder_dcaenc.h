
/***************************************************************************
    \file audioencoder_dcaenc.h
    copyright            : (C) 2011/2012 by mean
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
#ifndef AUDMaudioDCAENC_H
#define AUDMaudioDCAENC_H
 /**
        \class AUDMEncoder_DcaEnc
        \brief Front end for libDcaEnc
*/
class AUDMEncoder_DcaEnc : public ADM_AudioEncoder
{
  protected:
    dcaenc_context    context;

    int                 inputSize;
    int                 outputSize;

                      int send(uint32_t nbSample, uint8_t *dest);
  public:
    virtual             ~AUDMEncoder_DcaEnc();
                        AUDMEncoder_DcaEnc(AUDMAudioFilter *instream,bool globalHeader,CONFcouple *setup);
            bool	    isVBR(void );

   virtual bool         encode(uint8_t *dest, uint32_t *len, uint32_t *samples);
   virtual bool         initialize(void);
           bool         provideAccurateSample(void) {return false;}
};

#endif
