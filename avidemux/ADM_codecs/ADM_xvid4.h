/***************************************************************************
                          ADM_xvid4.h.h  -  description
			  	DevAPI4 xvid encoder
                             -------------------
    begin                : Fri Jul 12 2002
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

#ifdef USE_XVID_4



#include "ADM_codecs/ADM_xvid4param.h"

class xvid4Encoder:public encoder
{
protected:uint8_t _init;
  void *_handle;
  uint32_t encode_flags;
  uint32_t motion_flags;
  uint32_t _fps1000;
  xvid4EncParam _param;
  void dumpConf (void);
  void checkFlags (xvid4EncParam * extend = NULL);
  void createUpdate (void);

  uint8_t preAmble (uint8_t * in);
  uint8_t postAmble (ADMBitstream * out);
  void dump (void);

public:  xvid4Encoder (uint32_t width, uint32_t height):encoder (width,
							    height)
  {
    _init = 0;
    _handle = NULL;
  };
  ~xvid4Encoder ();
  void *getXvidStat (void);
  uint8_t stopEncoder (void);
  virtual uint8_t init (uint32_t val, uint32_t fps1000,
			xvid4EncParam * param);

  uint8_t init (uint32_t a, uint32_t b)
  {
    return 0;
  }				// not used
  virtual uint8_t encode (ADMImage * in, ADMBitstream * out) ;
};

class xvid4EncoderCQ:public xvid4Encoder
{
protected:uint32_t _q;

public:xvid4EncoderCQ (uint32_t width, uint32_t height):xvid4Encoder (width,
								 height)
  {

  };

  virtual uint8_t init (uint32_t val, uint32_t fps1000,
			xvid4EncParam * param);
  virtual uint8_t encode (ADMImage * in, ADMBitstream * out);
};
class xvid4EncoderVBRExternal:public xvid4Encoder
{
protected:uint32_t _q;

public:xvid4EncoderVBRExternal (uint32_t width,
			   uint32_t height):xvid4Encoder (width,
							  height)
  {

  };

  virtual uint8_t init (uint32_t val, uint32_t fps1000,
			xvid4EncParam * param);
  virtual uint8_t encode (ADMImage * in, ADMBitstream * out);
  
};
class xvid4EncoderCBR:public xvid4Encoder
{
protected:uint32_t _bitrate;	// In kBits!         

public:xvid4EncoderCBR (uint32_t width,
		   uint32_t height):xvid4Encoder (width, height)
  {

  };

  virtual uint8_t init (uint32_t val, uint32_t fps1000,
			xvid4EncParam * param);
  virtual uint8_t encode (ADMImage * in, ADMBitstream * out);
};
class xvid4EncoderPass1:public xvid4Encoder
{
protected: public:xvid4EncoderPass1 (uint32_t width,
		     uint32_t height):xvid4Encoder (width,
						    height)
  {

  };

  virtual uint8_t init (uint32_t val, uint32_t fps1000,
			xvid4EncParam * param);
  
};
class xvid4EncoderPass2:public xvid4Encoder
{
protected:uint32_t _bitrate;

public:xvid4EncoderPass2 (uint32_t width,
		     uint32_t height):xvid4Encoder (width, height)
  {

  };

  virtual uint8_t init (uint32_t val, uint32_t fps1000,
			xvid4EncParam * param);
  
};


#endif
