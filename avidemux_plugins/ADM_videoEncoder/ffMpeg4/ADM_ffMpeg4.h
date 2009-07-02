/***************************************************************************
                          \fn ADM_VideoEncoders
                          \brief Internal handling of video encoders
                             -------------------
    
    copyright            : (C) 2002/2009 by mean
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
#ifndef ADM_ffMpeg4_ENCODER_H
#define ADM_ffMpeg4_ENCODER_H
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "ADM_encoderConf.h"

/**
    FFcodecSetting
*/
typedef struct 
{
  COMPRES_PARAMS params;
//

  Motion_Est_ID me_method;
  uint32_t _GMC;
  uint32_t _4MV;
  uint32_t _QPEL;
  uint32_t _TRELLIS_QUANT;
  uint32_t qmin;			// 2-31
  uint32_t qmax;			// 2-31
  uint32_t max_qdiff;		// 1-31
  uint32_t max_b_frames;		// 0-1
  uint32_t mpeg_quant;		// 0-1
  uint32_t is_luma_elim_threshold;
  uint32_t luma_elim_threshold;	// -99--99
  uint32_t is_chroma_elim_threshold;	// -99--99           
  uint32_t chroma_elim_threshold;	// -99--99      

  float lumi_masking;		// -1--1        
  int32_t is_lumi_masking;		// -1--1
  float dark_masking;		// -1--1        
  int32_t is_dark_masking;		// -1--1
  float qcompress;		// 0.0--1.0
  float qblur;			// 0.0--1.0
  uint32_t minBitrate;          // In kBits/s
  uint32_t maxBitrate;          // In kBits/s
  uint32_t user_matrix;		// 0 normal / 1 tmpgenc / 2 anime / 3 kvcd / 4 hr-tmpgenc
  uint32_t gop_size;			// For mpeg1/2 , 12 is good
  uint16_t *intra_matrix;
  uint16_t *inter_matrix;
  uint32_t interlaced;
  uint32_t bff;			// WLA: bottom field first flag
  uint32_t widescreen;          //0 4/3  1 16/9

  // new stuff from jakub ui
  uint32_t mb_eval;			// Replace hq 0..2
  uint32_t vratetol;			// filesize tolerance in kb

  uint32_t is_temporal_cplx_masking;	// temporal masking 0--1        
  float temporal_cplx_masking;	// temporal masking 0--1

  uint32_t is_spatial_cplx_masking;	// spatial masking 0--1
  float spatial_cplx_masking;	// spatial masking 0--1
  uint32_t _NORMALIZE_AQP;		// normalize adap quantiz

  //
  uint32_t use_xvid_ratecontrol;
  uint32_t bufferSize;		// in KBYTES !!!!
  uint32_t override_ratecontrol;
  uint32_t dummy;

} FFcodecSetting;


/**
        \class ADM_ffMpeg4Encoder
        \brief Dummy encoder that does nothing

*/
class ADM_ffMpeg4Encoder : public ADM_coreVideoEncoderFFmpeg
{
protected:
               int              plane;
               bool            presetContext(FFcodecSetting *set);
               bool            postEncode(ADMBitstream *out, uint32_t size);
public:

                           ADM_ffMpeg4Encoder(ADM_coreVideoFilter *src);
                           ~ADM_ffMpeg4Encoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "DIVX";}

virtual        bool         isDualPass(void) ;
virtual        bool         startPass2(void) ;
};

#endif
