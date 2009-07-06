/***************************************************************************
                          ADM_vidEncode.hxx  -  description
                             -------------------
    begin                : Sun May 5 2002
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
#ifndef ADM_VIDENCODE
#define ADM_VIDENCODE

typedef enum
{
  COMPRESS_CQ,
  COMPRESS_CBR,
  COMPRESS_2PASS,
  COMPRESS_SAME,
  COMPRESS_2PASS_BITRATE,
  COMPRESS_AQ,
  COMPRESS_MAX
} COMPRESSION_MODE;

#define ADM_ENC_CAP_CBR    		0x001
#define ADM_ENC_CAP_CQ     		0x002
#define ADM_ENC_CAP_AQ     		0x080
#define ADM_ENC_CAP_2PASS  		0x004
#define ADM_ENC_CAP_GLOBAL 		0x008
#define ADM_ENC_CAP_2PASS_BR  	0x010
#define ADM_ENC_CAP_SAME  		0x020

#define ADM_EXTRA_PARAM_JS 0x100
#define ADM_EXTRA_PARAM    0x200
/**
    \struct COMPRES_PARAMS
    \brief Simple declaration of an encoder setting

*/
typedef struct 
{
  COMPRESSION_MODE  mode;
  uint32_t          qz;           /// Quantizer
  uint32_t          bitrate;      /// In kb/s ?
  uint32_t          finalsize;    /// In MB ?
  uint32_t          avg_bitrate;  /// avg_bitrate is in kb/s!!
  uint32_t          capabilities;
}COMPRES_PARAMS;



#endif
