/***************************************************************************
                          adm_encoder.h  -  description
                             -------------------

	This class defines the encoder.
	Encoder is the middle man between the actual codec and the user

	It is up to the encoder to deal with dual pass.
	That way it can be used for any codec.


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
#ifndef __ADM_encoder__
#define __ADM_encoder__
#include "ADM_image.h"
#include "ADM_bitstream.h"
#include "ADM_videoFilter.h"
typedef enum
{

  enc_CQ = 1,
  enc_CBR,
  enc_Pass1,
  enc_Pass2,
  enc_Same,
  enc_Invalid
} encoderState;




typedef struct entry_s
	/* max 28 bytes/frame or 5 Mb for 2-hour movie */
{
  int quant;
  int text_bits;
  int motion_bits;
  int total_bits;
  float mult;
  int is_key_frame;
  int drop;
} entry;


class Encoder
{

protected:
  encoderState _state;
  uint32_t _w, _h;
  ADMImage *_vbuffer;
  AVDMGenericVideoStream *_in;
  char _logname[500];
  COMPRES_PARAMS _param;

  // VBR
  FILE *fd;
  uint32_t _frametogo;
  entry *entries;
  uint8_t computeParameters (void);
// -- vbr

public:
    Encoder (void)
  {
    _w = _h = 0;
    _vbuffer = NULL;
    entries = NULL;
  };
  virtual ~ Encoder (void);
  virtual uint8_t isDualPass (void) = 0;
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile) = 0;
  virtual uint8_t encode (uint32_t frame, ADMBitstream * out) = 0;
  virtual uint8_t stop (void) = 0;
  virtual uint8_t setLogFile (const char *o, uint32_t frames) = 0;
  virtual uint8_t startPass1 (void) = 0;
  virtual uint8_t startPass2 (void) = 0;
  virtual uint8_t hasExtraHeaderData (uint32_t * l, uint8_t ** data)
  {
    *data = NULL;
    *l = 0;
    return 1;
  }
  virtual const char *getCodecName (void) = 0;
  virtual const char *getFCCHandler (void) = 0;
  virtual const char *getDisplayName (void) = 0;
  virtual uint8_t verifyLog(const char *,uint32_t nb) { return 1;} // Verify 1st pass log file..


};
Encoder *getVideoEncoder (uint32_t w, uint32_t h, uint32_t globalHeader = 0);


typedef enum
{
  CodecFamilyAVI,
  CodecFamilyMpeg,
  CodecFamilyXVCD
} CodecFamilty;

extern CodecFamilty videoCodecGetFamily (void);
extern void videoCodecConfigureUI(int codecIndex = -1);
extern int videoCodecConfigure (char *p, uint32_t i, uint8_t * c);
extern void videoCodecSelect (void);
#endif
