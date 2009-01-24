/***************************************************************************
                          adm_encxvid.h  -  description
                             -------------------
    begin                : Sun Jul 14 2002
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
#ifndef __ADM_encoder_xvid__
#define __ADM_encoder_xvid__

typedef struct XVIDconfig
{
  COMPRES_PARAMS generic;
  xvidEncParam specific;
} XVIDconfig;


class EncoderXvid:public Encoder
{

protected:

  xvidEncoder * _codec;
  xvidEncParam encparam;
  uint8_t updateStats (uint32_t len);
  void setAdvancedOptions (void);
  uint32_t _totalframe;
  uint8_t _pass1Done;
//  char                                        *_logFile;

public:
    EncoderXvid (XVIDconfig * codecconfig);
   ~EncoderXvid ();		// can be called twice if needed ..
  virtual uint8_t isDualPass (void);
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
  virtual uint8_t encode (uint32_t frame, uint32_t * len, uint8_t * out,
			  uint32_t * flags);
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
    return QT_TR_NOOP("Xvid");
  }
};


#endif
