/***************************************************************************
                         
    copyright            : (C) 2006 by mean
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
#ifndef ENC_MPEG2ENC_H
#define ENC_MPEG2ENC_H
#include "ADM_libraries/ADM_libmpeg2enc/ADM_mpeg2Param.h"
#include "ADM_libraries/ADM_libmpeg2enc/ADM_mpeg2enc.h"
typedef enum 
{
  MPEG2ENC_INVALID=0,
  MPEG2ENC_VCD=1,
  MPEG2ENC_SVCD=2,
  MPEG2ENC_DVD=3
}MPEG2ENC_ID;

class EncoderMpeg2enc:public Encoder
{
  private:
    uint8_t setMatrix (void);
    uint8_t _lastQz;
    uint32_t _lastBitrate;
    MPEG2ENC_ID _id;
    Mpeg2encParam _settings;
    Mpeg2enc      *_codec;
    uint32_t      _fps1000;
    uint32_t      _delayed;
    uint32_t      _availableFrames;
  public:


    uint32_t _totalframe;
    uint32_t _pass1Done;

  public:
    EncoderMpeg2enc (MPEG2ENC_ID id, COMPRES_PARAMS * config);
    virtual ~ EncoderMpeg2enc ();	// can be called twice if needed ..
    virtual uint8_t isDualPass (void);
    virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
    virtual uint8_t encode (uint32_t frame, ADMBitstream *out);
    virtual uint8_t setLogFile (const char *p, uint32_t fr);
    virtual uint8_t stop (void);
    virtual uint8_t startPass2 (void);
    virtual uint8_t startPass1 (void);
    virtual const char *getCodecName (void) {return "MPEG";};
    virtual const char *getFCCHandler (void) {return "MPEG";};
    virtual const char *getDisplayName (void) {return QT_TR_NOOP("MPEG");}; // FIXME
            uint8_t verifyLog(const char *name,uint32_t nbFrame);

};

#endif
//EOF
