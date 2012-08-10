/***************************************************************************
                          ADM_ffmp43.h  -  description
                             -------------------

	Mpeg4 ****decoder******** using ffmpeg

    begin                : Wed Sep 25 2002
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
#ifndef ADM_FFMP43_H
#define ADM_FFMP43_H

#include "ADM_coreVideoCodec6_export.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "ADM_codec.h"
#include "ADM_paramList.h"

/**
    \class decoderFF
    \brief Base class for lavcodec based decoder
*/
class ADM_COREVIDEOCODEC6_EXPORT decoderFF:public decoders
{
protected:
  bool  hurryUp;
  int codecId;
  uint8_t _refCopy;
  AVCodecContext *_context;
  AVFrame _frame;
  uint8_t *_internalBuffer;
  uint8_t _allowNull;
  uint32_t frameType (void);
  uint8_t clonePic (AVFrame * src, ADMImage * out);
  void decoderMultiThread ();
  uint32_t _gmc;
  uint32_t _usingMT;

	typedef struct {
		bool swapUv;
		bool showMv;
	} decoderFF_param_t;

	decoderFF_param_t decoderFF_params;
	static const decoderFF_param_t defaultConfig;
	static const ADM_paramList decoderFF_param_template[];

public:

    decoderFF (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
    virtual ~ decoderFF ();
  virtual bool dontcopy (void)
  {
    return true;
  }
  virtual bool uncompress (ADMCompressedImage * in, ADMImage * out);
  virtual bool getConfiguration(CONFcouple **conf);
  virtual bool resetConfiguration();
  virtual bool setConfiguration(CONFcouple * conf);
  virtual bool setParam (void);
  virtual bool bFramePossible (void)
  {
    return false;
  }
  virtual bool decodeHeaderOnly (void);
  virtual bool decodeFull (void);
//  virtual uint32_t getSpecificMpeg4Info (void);
  virtual uint8_t getPARWidth (void);
  virtual uint8_t getPARHeight (void);
  virtual bool    flush(void);
  virtual const char *getDecoderName(void) {return "Lavcodec";}
};

class decoderFFDiv3:public decoderFF
{
protected:
public:
  decoderFFDiv3 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
};

class decoderFFMpeg4:public decoderFF
{
protected:
public:
        decoderFFMpeg4 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
  bool uncompress (ADMCompressedImage * in, ADMImage * out);
  // mpeg4 can have B-frame
  virtual bool bFramePossible (void)
  {
    return 1;
  }
};
class decoderFFMpeg12:public decoderFF
{
protected:
public:
  decoderFFMpeg12 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
  // mpeg1/2 can have B-frame
  virtual bool bFramePossible (void)
  {
    return 1;
  }

};

class decoderFFDV:public decoderFF
{
protected:
public:
  decoderFFDV (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
};

class decoderFFH264:public decoderFF
{
protected:
  uint32_t _lowDelay;
public:
  decoderFFH264 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
  virtual bool bFramePossible (void)
  {
      return true;
  }
  bool   uncompress (ADMCompressedImage * in, ADMImage * out);
};

class decoderFFhuff:public decoderFF
{
protected:
public:
  decoderFFhuff (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
};

class decoderFF_ffhuff:public decoderFF
{
protected:
public:
  decoderFF_ffhuff (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
};

class decoderFFPng : public decoderFF
{
public:
	decoderFFPng(uint32_t w, uint32_t h, uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData, uint32_t bpp);
};

ADM_COREVIDEOCODEC6_EXPORT void ADM_lavInit(void);
ADM_COREVIDEOCODEC6_EXPORT void ADM_lavDestroy(void);

#endif

// EOF

