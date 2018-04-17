/***************************************************************************
                          \fn     libvaEnc_plugin
                          \brief  Plugin to use libva hw encoder (intel mostly)
                             -------------------

    copyright            : (C) 2018 by mean
    email                : fixounet@free.fr
 * 
 * 

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /***************************************************************************/
/* Derived from libva sample code */
/*
 * Copyright (c) 2007-2013 Intel Corporation. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once
/**
 * 
 */
#define VA_ENC_NB_SURFACE 16
#include "ADM_coreLibVA_encodingContext.h"
#include "ADM_coreLibVA_bitstream.h"
#include "vaDefines.h"
#include "vaenc_settings.h"
#include "ADM_coreLibVA_h264Encoding.h"
#include "vaenc_settings.h"
#include "ADM_libvaEncoder_debug.h"
extern vaconf_settings vaH264Settings;


#define VA_BITRATE (vaH264Settings.BitrateKbps*1000)

/**
 * \class ADM_vaEncodingContextH264Base
 * \brief This one is AVC format
 */
class ADM_vaEncodingContextH264Base : public ADM_vaEncodingContext
{
public:
                    ADM_vaEncodingContextH264Base()   ;             
virtual             ~ADM_vaEncodingContextH264Base();
virtual            bool    setup( int width, int height, int frameInc, std::vector<ADM_vaSurface *>knownSurfaces);
virtual            bool    encode(ADMImage *in, ADMBitstream *out);
virtual            bool    generateExtraData(int *size, uint8_t **data);
virtual            int     getPackedAttributes()
                            {
                                    return 0;
                            }
protected:    
//-- Per instance configuration --
          VAConfigID config_id;
          VAContextID context_id;
          
          
          VAEncSequenceParameterBufferH264 seq_param;
          VAEncPictureParameterBufferH264  pic_param;
          VAEncSliceParameterBufferH264    slice_param;
          VAPictureH264 CurrentCurrPic;
          VAPictureH264 ReferenceFrames[16], RefPicList0_P[32], RefPicList0_B[32], RefPicList1_B[32];

          unsigned int num_ref_frames;
          unsigned int numShortTerm;

          unsigned int MaxPicOrderCntLsb;
          unsigned int Log2MaxFrameNum;
          unsigned int Log2MaxPicOrderCntLsb;


          
          int frame_width;
          int frame_height;
          int frame_width_mbaligned;
          int frame_height_mbaligned;
          int gop_start;
          uint64_t current_frame_encoding;
          
          // -- RC --
          
          int initial_qp;
          int minimal_qp;
          int rc_mode;
          
          int frameDen,frameNum;
          //--
          ADM_vaEncodingBuffers *vaEncodingBuffers[VA_ENC_NB_SURFACE];
          ADM_vaSurface         *vaSurface[VA_ENC_NB_SURFACE];
          ADM_vaSurface         *vaRefSurface[VA_ENC_NB_SURFACE];
          uint8_t               *tmpBuffer;

         void sps_rbsp(vaBitstream *bs);
         void pps_rbsp(vaBitstream *bs);
         
      
               
        void encoding2display_order(    uint64_t encoding_order, int intra_idr_period,  vaFrameType *frame_type);
        bool update_ReferenceFrames(vaFrameType frameType);
        bool update_RefPicList(vaFrameType frameType);        
        int  calc_poc(int pic_order_cnt_lsb,vaFrameType frameType);
        // MP4
        bool render_sequence(void);
        bool render_picture(int frameNumber,vaFrameType frameType);
        
        void slice_header(vaBitstream *bs);
        
        
        bool render_hrd(void);

        //
        void fillSeqParam();
        void fillPPS(int frameNumber, vaFrameType frameType);
virtual bool render_slice(int frameNumber,vaFrameType frameType);   
        
        const ADM_VA_GlobalH264  *h264;
        
};


/**
 * \class ADM_vaEncodingContextH264AnnexB
 * \brief This one is AnnexB format (mpeg TS)
 */

class ADM_vaEncodingContextH264AnnexB: public ADM_vaEncodingContextH264Base
{
public:
                    ADM_vaEncodingContextH264AnnexB()   ;             
    virtual         ~ADM_vaEncodingContextH264AnnexB();
    virtual bool    setup( int width, int height, int frameInc, std::vector<ADM_vaSurface *>knownSurfaces);
    virtual bool    encode(ADMImage *in, ADMBitstream *out);
    virtual bool    generateExtraData(int *size, uint8_t **data);
    
protected:    

         
         bool build_packed_pic_buffer(vaBitstream *bs);
         bool build_packed_seq_buffer(vaBitstream *bs);    
         bool build_packed_sei_buffer_timing(vaBitstream *bs,
                                unsigned int init_cpb_removal_length,
				unsigned int init_cpb_removal_delay,
				unsigned int init_cpb_removal_delay_offset,
				unsigned int cpb_removal_length,
				unsigned int cpb_removal_delay,
				unsigned int dpb_output_length,
				unsigned int dpb_output_delay);
               
      
        
        // Annex B
        bool render_packedsequence(void);
        bool render_packedpicture(void);
        bool render_packedsei(int frameNumber);
        bool render_packedslice(void);
        bool build_packed_slice_buffer(vaBitstream *bs);
        
        bool render_hrd(void);
        bool render_slice(int frameNumber,vaFrameType frameType);   
        int     getPackedAttributes()
        {
          return h264->packedHeaderCapabilities;
        }
        
};

// EOF 
 
