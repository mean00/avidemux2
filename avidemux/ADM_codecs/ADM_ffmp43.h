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

#include "ADM_lavcodec.h"
class decoderFFSubs
{
  protected:
     int      codecId;
     uint32_t subId;
     AVCodecContext *_context;
  public:

    decoderFFSubs (uint32_t subId);
    virtual ~ decoderFFSubs ();
    virtual uint8_t uncompress (ADMCompressedImage * in, AVSubtitle * out); 
};

/****************************/
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

    decoderFF (uint32_t w, uint32_t h);
    virtual ~ decoderFF ();
  virtual uint8_t dontcopy (void)
  {
    return 1;
  }
  virtual uint8_t uncompress (ADMCompressedImage * in, ADMImage * out);
  virtual void setParam (void);
  virtual uint8_t bFramePossible (void)
  {
    return 0;
  }
  virtual uint8_t decodeHeaderOnly (void);
  virtual uint8_t decodeFull (void);
  virtual uint8_t isDivxPacked (void);
  virtual uint32_t getSpecificMpeg4Info (void);
  virtual uint8_t getPARWidth (void);
  virtual uint8_t getPARHeight (void);
  
};

class decoderFFDiv3:public decoderFF
{
protected:


public:

  decoderFFDiv3 (uint32_t w, uint32_t h);

};
class decoderFFMpeg4VopPacked:public decoderFF
{
protected:


public:
  decoderFFMpeg4VopPacked (uint32_t w, uint32_t h);
  uint8_t uncompress (ADMCompressedImage * in, ADMImage * out);
  // mpeg4 can have B-frame
  virtual uint8_t bFramePossible (void)
  {
    return 0;
  }
  // Vop packed are not indexable
  virtual uint8_t isIndexable (void)
  {
    return 0;
  };
};
class decoderFFMpeg4:public decoderFF
{
protected:


public:
  decoderFFMpeg4 (uint32_t w, uint32_t h, uint32_t fcc,uint32_t l, uint8_t * d);
  uint8_t uncompress (ADMCompressedImage * in, ADMImage * out);
  // mpeg4 can have B-frame
  virtual uint8_t bFramePossible (void)
  {
    return 1;
  }

};
class decoderFFMpeg12:public decoderFF
{
protected:


public:
  decoderFFMpeg12 (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);
  // mpeg1/2 can have B-frame
  virtual uint8_t bFramePossible (void)
  {
    return 1;
  }

};
class decoderFFSVQ3:public decoderFF
{
protected:


public:
  decoderFFSVQ3 (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);
  virtual uint8_t bFramePossible (void)
  {
    return 0;
  }

};

class decoderFFDV:public decoderFF
{
protected:


public:
  decoderFFDV (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);

};
class decoderFFMP42:public decoderFF
{
protected:


public:
  decoderFFMP42 (uint32_t w, uint32_t h);

};
class decoderFFH263:public decoderFF
{
protected:


public:
  decoderFFH263 (uint32_t w, uint32_t h);

};
class decoderFFH264:public decoderFF
{
protected:
  uint32_t _lowDelay;

public:
  decoderFFH264 (uint32_t w, uint32_t h, uint32_t l, uint8_t * d,uint32_t lowdelay);
  virtual uint8_t bFramePossible (void)
  {
      return _lowDelay;
  }
  uint8_t   uncompress (ADMCompressedImage * in, ADMImage * out);

};
class decoderFFhuff:public decoderFF
{
protected:


public:
  decoderFFhuff (uint32_t w, uint32_t h, uint32_t l, uint8_t * d,uint32_t bpp);

};
class decoderFF_ffhuff:public decoderFF
{
protected:


public:
  decoderFF_ffhuff (uint32_t w, uint32_t h, uint32_t l, uint8_t * d,uint32_t bpp);

};
class decoderFFWMV2:public decoderFF
{
protected:


public:
  decoderFFWMV2 (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);

};
class decoderFFWMV1:public decoderFF
{
  protected:


  public:
    decoderFFWMV1 (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);

};

class decoderFFWMV3:public decoderFF
{
protected:


public:
  decoderFFWMV3 (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);

};

class decoderFFVC1:public decoderFF
{
protected:


public:
  decoderFFVC1 (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);

};

class decoderFFV1:public decoderFF
{
protected:


public:
  decoderFFV1 (uint32_t w, uint32_t h);

};
class decoderFFMJPEG:public decoderFF
{
protected:


public:
  decoderFFMJPEG (uint32_t w, uint32_t h);

};
class decoderSnow:public decoderFF
{
protected:


public:
  decoderSnow (uint32_t w, uint32_t h);

};
class decoderFFcyuv:public decoderFF
{
protected:


public:
  decoderFFcyuv (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);

};
class decoderCamtasia:public decoderFF
{
protected:


public:
  decoderCamtasia (uint32_t w, uint32_t h,uint32_t bpp);

};
class decoderFFTheora:public decoderFF
{
protected:


public:
  decoderFFTheora (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);

};

class decoderFFCinepak:public decoderFF
{
protected:


public:
  decoderFFCinepak (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);

};
class decoderFFCRAM:public decoderFF
{
protected:


public:
  decoderFFCRAM (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);

};
class decoderFFVP6F:public decoderFF
{
protected:
public:
  decoderFFVP6F (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);
};
class decoderFFFLV1:public decoderFF
{
protected:
public:
  decoderFFFLV1 (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);
};
class decoderFFDVBSub:public decoderFF
{
protected:
public:
  decoderFFDVBSub (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);
};
class decoderFFAMV:public decoderFF
{
protected:
public:
  decoderFFAMV (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);
};
class decoderFFMjpegB:public decoderFF
{
protected:
public:
  decoderFFMjpegB (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);
};

#ifdef USE_VDPAU
class decoderFFVDPAU:public decoderFF
{
protected:
        
       
                    ADMImage *scratch;
public:
                    decoderFFVDPAU (uint32_t w, uint32_t h, uint32_t l, uint8_t * d);
    virtual uint8_t uncompress (ADMCompressedImage * in, ADMImage * out);
    virtual uint8_t dontcopy (void)
                      {
                        return 0;
                      }
};

#endif
// EOF

