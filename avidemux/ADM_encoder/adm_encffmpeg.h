/***************************************************************************
                          adm_encffmpeg.h  -  description
                             -------------------
    begin                : Tue Sep 10 2002
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
#ifndef __ADM_encoder_ff__
#define __ADM_encoder_ff__



class EncoderFFMPEG:public Encoder
{

protected:

  ffmpegEncoder * _codec;
  uint8_t _internal;
  FF_CODEC_ID _id;
  uint32_t _fps;
  FFcodecSetting _settings;
public:
    EncoderFFMPEG (FF_CODEC_ID id, COMPRES_PARAMS * codecParam);
   ~EncoderFFMPEG ()
  {
    stop ();
  };				// can be called twice if needed ..
  virtual uint8_t isDualPass (void);
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
  virtual uint8_t encode (uint32_t frame,ADMBitstream *out);
  virtual uint8_t setLogFile (const char *p, uint32_t fr);
  virtual uint8_t stop (void);
  virtual uint8_t startPass2 (void);
  virtual uint8_t startPass1 (void);
  virtual const char *getDisplayName (void)
  {
    return QT_TR_NOOP("LavCodec");
  }
  virtual const char *getCodecName (void);	//{return "DX50";}
  virtual const char *getFCCHandler (void)
  {
    return "divx";
  }


};

class EncoderFFMPEGHuff:public EncoderFFMPEG
{

protected:


public:
  EncoderFFMPEGHuff (COMPRES_PARAMS * codecParam);
  ~EncoderFFMPEGHuff ()
  {
    stop ();
  };				// can be called twice if needed ..
  virtual uint8_t isDualPass (void)
  {
    return 0;
  };
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
  virtual uint8_t setLogFile (const char *p, uint32_t fr)
  {
    UNUSED_ARG (p);
    UNUSED_ARG (fr);
    return 1;
  };
  virtual uint8_t startPass2 (void)
  {
    return 1;
  };
  virtual uint8_t startPass1 (void)
  {
    return 1;
  };
  virtual const char *getDisplayName (void)
  {
    return "LavCodec HUFFYUV";
  }
  virtual const char *getFCCHandler (void)
  {
    return "HFYU";
  }
  virtual uint8_t hasExtraHeaderData (uint32_t * l, uint8_t ** data);

};
class EncoderFFMPEGFFHuff:public EncoderFFMPEG
{

protected:


public:
  EncoderFFMPEGFFHuff (COMPRES_PARAMS * config);
  ~EncoderFFMPEGFFHuff ()
  {
    stop ();
  };				// can be called twice if needed ..
  virtual uint8_t isDualPass (void)
  {
    return 0;
  };
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
  virtual uint8_t setLogFile (const char *p, uint32_t fr)
  {
    UNUSED_ARG (p);
    UNUSED_ARG (fr);
    return 1;
  };
  virtual uint8_t startPass2 (void)
  {
    return 1;
  };
  virtual uint8_t startPass1 (void)
  {
    return 1;
  };
  virtual const char *getDisplayName (void)
  {
    return "LavCodec HUFFYUV";
  }
  virtual const char *getFCCHandler (void)
  {
    return "HFYU";
  }
  virtual uint8_t hasExtraHeaderData (uint32_t * l, uint8_t ** data);

};

class EncoderFFMPEGFFV1:public EncoderFFMPEG
{

protected:


public:
  EncoderFFMPEGFFV1 (COMPRES_PARAMS * config);
  ~EncoderFFMPEGFFV1 ()
  {
    stop ();
  };				// can be called twice if needed ..
  virtual uint8_t isDualPass (void)
  {
    return 0;
  };
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
  virtual uint8_t setLogFile (const char *p, uint32_t fr)
  {
    UNUSED_ARG (p);
    UNUSED_ARG (fr);
    return 1;
  };
  virtual uint8_t startPass2 (void)
  {
    return 1;
  };
  virtual uint8_t startPass1 (void)
  {
    return 1;
  };
  virtual const char *getDisplayName (void)
  {
    return "LavCodec FFV1";
  }
  virtual const char *getFCCHandler (void)
  {
    return "FFV1";
  }
  virtual uint8_t hasExtraHeaderData (uint32_t * l, uint8_t ** data);

};


class EncoderFFMPEGMpeg1:public EncoderFFMPEG
{
private:
  uint8_t setMatrix (void);
  uint8_t _lastQz;
  uint32_t _lastBitrate;

public:


    uint32_t _totalframe;
  uint32_t _pass1Done;
  uint8_t _use_xvid_ratecontrol;

public:
    EncoderFFMPEGMpeg1 (FF_CODEC_ID id, COMPRES_PARAMS * config);
    virtual ~ EncoderFFMPEGMpeg1 ();	// can be called twice if needed ..
  virtual uint8_t isDualPass (void);
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
  virtual uint8_t encode (uint32_t frame, ADMBitstream *out);
  virtual uint8_t setLogFile (const char *p, uint32_t fr);
  virtual uint8_t stop (void);
  virtual uint8_t startPass2 (void);
  virtual uint8_t startPass1 (void);
          uint8_t verifyLog(const char *name,uint32_t nbFrame);
};

class EncodeFFMPEGSNow:public EncoderFFMPEG
{

protected:


public:
  EncodeFFMPEGSNow (COMPRES_PARAMS * config);
  ~EncodeFFMPEGSNow ()
  {
    stop ();
  };				// can be called twice if needed ..
  virtual uint8_t isDualPass (void)
  {
    return 0;
  };
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
  virtual uint8_t setLogFile (const char *p, uint32_t fr)
  {
    UNUSED_ARG (p);
    UNUSED_ARG (fr);
    return 1;
  };
  virtual uint8_t startPass2 (void)
  {
    return 1;
  };
  virtual uint8_t startPass1 (void)
  {
    return 1;
  };
  virtual const char *getDisplayName (void)
  {
    return "LavCodec Snow";
  }
  virtual const char *getFCCHandler (void)
  {
    return "SNOW";
  }
  virtual uint8_t hasExtraHeaderData (uint32_t * l, uint8_t ** data);

};


    
class EncoderFFMPEGDV:public EncoderFFMPEG
{

protected:


public:
  EncoderFFMPEGDV (COMPRES_PARAMS * config);
  ~EncoderFFMPEGDV ()
  {
    stop ();
  };				// can be called twice if needed ..
  virtual uint8_t isDualPass (void)
  {
    return 0;
  };
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
  virtual uint8_t setLogFile (const char *p, uint32_t fr)
  {
    UNUSED_ARG (p);
    UNUSED_ARG (fr);
    return 1;
  };
  virtual uint8_t startPass2 (void)
  {
    return 1;
  };
  virtual uint8_t startPass1 (void)
  {
    return 1;
  };
  virtual const char *getDisplayName (void)
  {
    return "FFMPEG DV";
  }
  virtual const char *getFCCHandler (void)
  {
    return "dvsd";
  }
  virtual uint8_t hasExtraHeaderData (uint32_t * l, uint8_t ** data);

};
class EncoderFFMPEGFLV1:public EncoderFFMPEG
{

protected:


public:
	EncoderFFMPEGFLV1 (COMPRES_PARAMS * config);
  ~EncoderFFMPEGFLV1 ()
  {
    stop ();
  };				// can be called twice if needed ..
  virtual uint8_t isDualPass (void)
  {
    return 0;
  };
  virtual uint8_t configure (AVDMGenericVideoStream * instream, int useExistingLogFile);
  virtual uint8_t setLogFile (const char *p, uint32_t fr)
  {
    UNUSED_ARG (p);
    UNUSED_ARG (fr);
    return 1;
  };
  virtual uint8_t startPass2 (void)
  {
    return 1;
  };
  virtual uint8_t startPass1 (void)
  {
    return 1;
  };
  virtual const char *getDisplayName (void)
  {
    return "FFMPEG FLV1";
  }
  virtual const char *getFCCHandler (void)
  {
    return "FLV1";
  }
  virtual uint8_t hasExtraHeaderData (uint32_t * l, uint8_t ** data);

};
#endif
