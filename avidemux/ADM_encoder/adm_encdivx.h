/***************************************************************************
                          adm_encdivx.h  -  description
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
#ifndef __ADM_encoder_divx__
#define __ADM_encoder_divx__
typedef struct DIVXConfig
{
  COMPRES_PARAMS generic;
} DIVXConfig;



class EncoderDivx:public Encoder
{

protected:

  divxEncoder * _codec;
public:
  EncoderDivx (DIVXConfig * conf);
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
    return "DX50";
  }
  virtual const char *getFCCHandler (void)
  {
    return "divx";
  }
  virtual const char *getDisplayName (void)
  {
    return QT_TR_NOOP("DivX");
  }

};


#endif
