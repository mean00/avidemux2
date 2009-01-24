/***************************************************************************
                         Fake encoder used for copy mode

        We have to reorder !
        TODO FIXME

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
#ifndef __ADM_encoder_yv12__
#define __ADM_encoder_yv12__


class EncoderYV12:public Encoder
{

protected:

public:
  EncoderYV12 (void);
   ~EncoderYV12 ();		// can be called twice if needed ..
  virtual uint8_t isDualPass (void)
  {
    return 0;
  };
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
  virtual uint8_t encode (uint32_t frame, ADMBitstream *out);

  virtual uint8_t stop (void);

  uint8_t startPass1 (void)
  {
    return 0;
  }
  uint8_t startPass2 (void)
  {
    return 0;
  }
  uint8_t setLogFile (const char *, uint32_t)
  {
    return 0;
  }
  uint8_t hasExtraHeaderData (uint32_t * l, uint8_t ** v)
  {
    *l = 0;
    *v = NULL;
    return 0;
  };
  const char *getCodecName (void)
  {
    return "YV12";
  }
  const char *getFCCHandler (void)
  {
    return "YV12";
  }
  const char *getDisplayName (void)
  {
    return QT_TR_NOOP("YV12");
  }

};


#endif
