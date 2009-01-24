/***************************************************************************
                          adm_encxvi4d.h  -  description
                             -------------------
			     structure for xvid api-4 encoder
    begin                : Sun Jul 14 2002
    copyright            : (C) 2002/2003 by mean
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
#ifndef __ADM_encoder_xvid4__
#define __ADM_encoder_xvid4__
#include "ADM_codecs/ADM_xvid4param.h"
typedef struct XVID4config
{
  COMPRES_PARAMS generic;
  xvid4EncParam specific;
} XVID4config;


class EncoderXvid4:public Encoder
{

protected:

  xvid4Encoder * _codec;
  xvid4EncParam encparam;


  uint32_t _totalframe;
  uint8_t _pass1Done;
  uint32_t _fps1000;

public:
    EncoderXvid4 (COMPRES_PARAMS * codecconfig);
   ~EncoderXvid4 ();		// can be called twice if needed ..
  virtual uint8_t isDualPass (void);
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
 
  virtual uint8_t encode (uint32_t frame, ADMBitstream *out);
  virtual uint8_t setLogFile (const char *p, uint32_t fr);
  virtual uint8_t stop (void);
  virtual uint8_t startPass2 (void);
  virtual uint8_t startPass1 (void);
  virtual const char *getCodecName (void)
  {
    return "XVID";
  }
  virtual const char *getFCCHandler (void)
  {
    return "xvid";
  }
  virtual const char *getDisplayName (void)
  {
    return QT_TR_NOOP("Xvid4");
  }

};


#endif
