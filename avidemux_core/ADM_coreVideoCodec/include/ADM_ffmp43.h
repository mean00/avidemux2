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

#include "ADM_lavcodec.h"

/**
    \class decoderFF
    \brief Base class for lavcodec based decoder
*/
class decoderFF:public decoders
{
protected:

  int codecId;
  uint8_t _refCopy;
  AVCodecContext *_context;
  AVFrame _frame;
  uint8_t *_internalBuffer;
  uint8_t _allowNull;
  uint32_t _swapUV;
  uint32_t frameType (void);
  uint8_t clonePic (AVFrame * src, ADMImage * out);
  void decoderMultiThread ();
  uint32_t _showMv;
  uint32_t _gmc;
  uint32_t _usingMT;
public:

    decoderFF (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
    virtual ~ decoderFF ();
  virtual bool dontcopy (void)
  {
    return true;
  }
  virtual bool uncompress (ADMCompressedImage * in, ADMImage * out);
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
      return _lowDelay;
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


#ifdef USE_VDPAU
class decoderFFVDPAU:public decoderFF
{
protected:
                    int b_age;
                    int ip_age[2];

                    void     *vdpau;
                    ADMImage *scratch;
                    ADMImage *vdpau_copy;
                    uint64_t vdpau_pts;
                    bool     decode_status;
                    bool     destroying;
public:     // Callbacks
                    int     getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    void    releaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);
                    void    goOn( const AVFrame *d,int type);            
public:
            // public API
                    decoderFFVDPAU (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);
                    ~decoderFFVDPAU();
    virtual bool uncompress (ADMCompressedImage * in, ADMImage * out);
    virtual bool dontcopy (void)
                      {
                        return 0;
                      }
    virtual bool bFramePossible (void)
      {
        return 1;
      }
};

#endif
#endif
// EOF

