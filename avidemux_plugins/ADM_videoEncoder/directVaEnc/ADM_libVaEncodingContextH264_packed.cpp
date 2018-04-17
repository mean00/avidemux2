/***************************************************************************
                          \fn     libvaEnc_plugin
                          \brief  Plugin to use libva hw encoder (intel mostly)
                             -------------------

    copyright            : (C) 2018 by mean
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
#include "ADM_default.h"
#include "ADM_bitstream.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_videoInfoExtractor.h"


#include "va/va.h"
#include "va/va_enc_h264.h"
#include "ADM_coreLibVA_buffer.h"
#include "ADM_libVaEncodingContextH264.h"


/**
 * 
 * @param header_buffer
 * @return 
 */
bool ADM_vaEncodingContextH264AnnexB::build_packed_pic_buffer(vaBitstream *bs)
{
    bs->startCodePrefix();
    bs->nalHeader(NAL_REF_IDC_HIGH, NAL_PPS);
    pps_rbsp(bs);
    bs->startCodePrefix();
    return true; // Offset
}

/**
 * 
 * @param header_buffer
 * @return 
 */
bool ADM_vaEncodingContextH264AnnexB::build_packed_seq_buffer(vaBitstream *bs)
{
    bs->startCodePrefix();
    bs->nalHeader(NAL_REF_IDC_HIGH, NAL_SPS);
    sps_rbsp(bs);
    bs->stop();
    return true;
}

/**
 * 
 * @param init_cpb_removal_length
 * @param init_cpb_removal_delay
 * @param init_cpb_removal_delay_offset
 * @param cpb_removal_length
 * @param cpb_removal_delay
 * @param dpb_output_length
 * @param dpb_output_delay
 * @param sei_buffer
 * @return 
 */
bool ADM_vaEncodingContextH264AnnexB::build_packed_sei_buffer_timing(vaBitstream *bs,
                                                               unsigned int init_cpb_removal_length,
                                                               unsigned int init_cpb_removal_delay,
                                                               unsigned int init_cpb_removal_delay_offset,
                                                               unsigned int cpb_removal_length,
                                                               unsigned int cpb_removal_delay,
                                                               unsigned int dpb_output_length,
                                                               unsigned int dpb_output_delay)
{
    unsigned char *byte_buf;
    int bp_byte_size, i, pic_byte_size;


    // sei _bp 
    vaBitstream sei_bp;
    sei_bp.put_ue(0);
    sei_bp.put_ui(init_cpb_removal_delay, cpb_removal_length);
    sei_bp.put_ui(init_cpb_removal_delay_offset, cpb_removal_length);
    sei_bp.add1BitIfNotaligned();
    sei_bp.stop();
    bp_byte_size = (sei_bp.lengthInBits() + 7) / 8;

    // sei_pic
    vaBitstream sei_pic;

    sei_pic.put_ui(cpb_removal_delay, cpb_removal_length);
    sei_pic.put_ui(dpb_output_delay, dpb_output_length);
    sei_pic.add1BitIfNotaligned();
    sei_pic.stop();
    pic_byte_size = (sei_bp.lengthInBits() + 7) / 8;
    //--- nal
    vaBitstream nal;

    nal.startCodePrefix();
    nal.nalHeader(NAL_REF_IDC_NONE, NAL_SEI);

    /* Write the SEI buffer period data */
    nal.put_ui(0, 8);
    nal.put_ui(bp_byte_size, 8);


    //------------ Merge-------------

    byte_buf = sei_bp.getPointer();
    for (i = 0; i < bp_byte_size; i++)
    {
        nal.put_ui(byte_buf[i], 8);
    }

    /* write the SEI timing data */
    nal.put_ui(0x01, 8);
    nal.put_ui(pic_byte_size, 8);

    byte_buf = sei_pic.getPointer();
    for (i = 0; i < pic_byte_size; i++)
    {
        nal.put_ui(byte_buf[i], 8);
    }

    nal.rbspTrailingBits();
    nal.stop();
    return true;
}

/**
 * 
 * @param header_buffer
 * @return 
 */
bool ADM_vaEncodingContextH264AnnexB::build_packed_slice_buffer(vaBitstream *bs)
{
    int is_idr = !!pic_param.pic_fields.bits.idr_pic_flag;
    int is_ref = !!pic_param.pic_fields.bits.reference_pic_flag;

    bs->startCodePrefix();
    if (IS_I_SLICE(slice_param.slice_type))
    {
        bs->nalHeader(NAL_REF_IDC_HIGH, is_idr ? NAL_IDR : NAL_NON_IDR);
    }
    else if (IS_P_SLICE(slice_param.slice_type))
    {
        bs->nalHeader(NAL_REF_IDC_MEDIUM, NAL_NON_IDR);
    }
    else
    {
        assert(IS_B_SLICE(slice_param.slice_type));
        bs->nalHeader(is_ref ? NAL_REF_IDC_LOW : NAL_REF_IDC_NONE, NAL_NON_IDR);
    }
    slice_header(bs);
    bs->stop();
    return true;
}

/**
 * 
 */
bool ADM_vaEncodingContextH264AnnexB::render_packedslice()
{
    VAEncPackedHeaderParameterBuffer packedheader_param_buffer;
    VABufferID packedslice_para_bufid, packedslice_data_bufid, render_id[2];
    unsigned int length_in_bits;

    VAStatus va_status;
    vaBitstream bs;
    build_packed_slice_buffer(&bs);
    length_in_bits = bs.lengthInBits();

    packedheader_param_buffer.type = VAEncPackedHeaderSlice;
    packedheader_param_buffer.bit_length = length_in_bits;
    packedheader_param_buffer.has_emulation_bytes = 0;

    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(),
                                        context_id,
                                        VAEncPackedHeaderParameterBufferType,
                                        sizeof (packedheader_param_buffer), 1, &packedheader_param_buffer,
                                        &packedslice_para_bufid));


    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(),
                                        context_id,
                                        VAEncPackedHeaderDataBufferType,
                                        (length_in_bits + 7) / 8, 1, bs.getPointer(),
                                        &packedslice_data_bufid));


    render_id[0] = packedslice_para_bufid;
    render_id[1] = packedslice_data_bufid;
    CHECK_VA_STATUS_BOOL(vaRenderPicture(admLibVA::getDisplay(), context_id, render_id, 2));

    return true;
}


/**
 * 
 */
bool ADM_vaEncodingContextH264AnnexB::render_packedsei(int frameNumber)
{
    VAEncPackedHeaderParameterBuffer packed_header_param_buffer;
    VABufferID packed_sei_header_param_buf_id, packed_sei_buf_id, render_id[2];
    unsigned int length_in_bits /*offset_in_bytes*/;

    VAStatus va_status;
    vaBitstream bs;
    int init_cpb_size, target_bit_rate, i_initial_cpb_removal_delay_length, i_initial_cpb_removal_delay;
    int i_cpb_removal_delay, i_dpb_output_delay_length, i_cpb_removal_delay_length;

    /* it comes for the bps defined in SPS */
    target_bit_rate = VA_BITRATE;
    init_cpb_size = (target_bit_rate * 8) >> 10;
    i_initial_cpb_removal_delay = init_cpb_size * 0.5 * 1024 / target_bit_rate * 90000;

    i_cpb_removal_delay = 2;
    i_initial_cpb_removal_delay_length = 24;
    i_cpb_removal_delay_length = 24;
    i_dpb_output_delay_length = 24;


    build_packed_sei_buffer_timing(&bs,
                                   i_initial_cpb_removal_delay_length,
                                   i_initial_cpb_removal_delay,
                                   0,
                                   i_cpb_removal_delay_length,
                                   i_cpb_removal_delay * frameNumber,
                                   i_dpb_output_delay_length,
                                   0);
    length_in_bits = bs.lengthInBits();
    //offset_in_bytes = 0;
    packed_header_param_buffer.type = VAEncPackedHeaderH264_SEI;
    packed_header_param_buffer.bit_length = length_in_bits;
    packed_header_param_buffer.has_emulation_bytes = 0;

    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(),
                                        context_id,
                                        VAEncPackedHeaderParameterBufferType,
                                        sizeof (packed_header_param_buffer), 1, &packed_header_param_buffer,
                                        &packed_sei_header_param_buf_id));


    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(),
                                        context_id,
                                        VAEncPackedHeaderDataBufferType,
                                        (length_in_bits + 7) / 8, 1, bs.getPointer(),
                                        &packed_sei_buf_id));



    render_id[0] = packed_sei_header_param_buf_id;
    render_id[1] = packed_sei_buf_id;
    CHECK_VA_STATUS_BOOL(vaRenderPicture(admLibVA::getDisplay(), context_id, render_id, 2));

    return true;
}

/**
 * 
 * @return 
 */
bool ADM_vaEncodingContextH264AnnexB::render_packedpicture(void)
{
    VAEncPackedHeaderParameterBuffer packedheader_param_buffer;
    VABufferID packedpic_para_bufid, packedpic_data_bufid, render_id[2];
    unsigned int length_in_bits;

    VAStatus va_status;
    vaBitstream bs;


    build_packed_pic_buffer(&bs);
    length_in_bits = bs.lengthInBits();
    packedheader_param_buffer.type = VAEncPackedHeaderPicture;
    packedheader_param_buffer.bit_length = length_in_bits;
    packedheader_param_buffer.has_emulation_bytes = 0;

    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(),
                                        context_id,
                                        VAEncPackedHeaderParameterBufferType,
                                        sizeof (packedheader_param_buffer), 1, &packedheader_param_buffer,
                                        &packedpic_para_bufid));


    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(),
                                        context_id,
                                        VAEncPackedHeaderDataBufferType,
                                        (length_in_bits + 7) / 8, 1, bs.getPointer(),
                                        &packedpic_data_bufid));


    render_id[0] = packedpic_para_bufid;
    render_id[1] = packedpic_data_bufid;
    CHECK_VA_STATUS_BOOL(vaRenderPicture(admLibVA::getDisplay(), context_id, render_id, 2));


    return true;
}

/**
 * 
 * @return 
 */
bool ADM_vaEncodingContextH264AnnexB::render_packedsequence(void)
{
    VAEncPackedHeaderParameterBuffer packedheader_param_buffer;
    VABufferID packedseq_para_bufid, packedseq_data_bufid, render_id[2];
    unsigned int length_in_bits;

    VAStatus va_status;
    vaBitstream bs;

    build_packed_seq_buffer(&bs);
    length_in_bits = bs.lengthInBits();

    packedheader_param_buffer.type = VAEncPackedHeaderSequence;

    packedheader_param_buffer.bit_length = length_in_bits; /*length_in_bits*/
    packedheader_param_buffer.has_emulation_bytes = 0;
    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(),
                                        context_id,
                                        VAEncPackedHeaderParameterBufferType,
                                        sizeof (packedheader_param_buffer), 1, &packedheader_param_buffer,
                                        &packedseq_para_bufid));


    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(),
                                        context_id,
                                        VAEncPackedHeaderDataBufferType,
                                        (length_in_bits + 7) / 8, 1, bs.getPointer(),
                                        &packedseq_data_bufid));


    render_id[0] = packedseq_para_bufid;
    render_id[1] = packedseq_data_bufid;
    CHECK_VA_STATUS_BOOL(vaRenderPicture(admLibVA::getDisplay(), context_id, render_id, 2));

    return true;
}


/**
 * 
 * @return 
 */
bool ADM_vaEncodingContextH264AnnexB::render_hrd(void)
{
    VABufferID misc_parameter_hrd_buf_id;
    VAStatus va_status;
    VAEncMiscParameterBuffer *misc_param;
    VAEncMiscParameterHRD *misc_hrd_param;

    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(), context_id,
                                        VAEncMiscParameterBufferType,
                                        sizeof (VAEncMiscParameterBuffer) + sizeof (VAEncMiscParameterHRD),
                                        1,
                                        NULL,
                                        &misc_parameter_hrd_buf_id));


    vaMapBuffer(admLibVA::getDisplay(),
                misc_parameter_hrd_buf_id,
                (void **) &misc_param);
    misc_param->type = VAEncMiscParameterTypeHRD;
    misc_hrd_param = (VAEncMiscParameterHRD *) misc_param->data;

    if (VA_BITRATE > 0)
    {
        misc_hrd_param->initial_buffer_fullness = VA_BITRATE* 1024 * 4;
        misc_hrd_param->buffer_size = VA_BITRATE * 1024 * 8;
    }
    else
    {
        misc_hrd_param->initial_buffer_fullness = 0;
        misc_hrd_param->buffer_size = 0;
    }
    vaUnmapBuffer(admLibVA::getDisplay(), misc_parameter_hrd_buf_id);

    CHECK_VA_STATUS_BOOL(vaRenderPicture(admLibVA::getDisplay(), context_id, &misc_parameter_hrd_buf_id, 1));


    return true;
}
//--


/**
 * 
 * @return 
 */
bool ADM_vaEncodingContextH264AnnexB::render_slice(int frameNumber,vaFrameType frameType)
{
    VABufferID slice_param_buf;
    VAStatus va_status;
    int i;

    update_RefPicList(frameType);

    /* one frame, one slice */
    slice_param.macroblock_address = 0;
    slice_param.num_macroblocks = frame_width_mbaligned * frame_height_mbaligned / (16 * 16); /* Measured by MB */
    switch(frameType)      
    {
        case FRAME_IDR:
            slice_param.slice_type =SLICE_TYPE_I;
            if (frameNumber)  
                slice_param.idr_pic_id++;
            for (i = 0; i < 32; i++)
            {
                slice_param.RefPicList0[i].picture_id = VA_INVALID_SURFACE;
                slice_param.RefPicList0[i].flags      = VA_PICTURE_H264_INVALID;
                slice_param.RefPicList1[i].picture_id = VA_INVALID_SURFACE;
                slice_param.RefPicList1[i].flags      = VA_PICTURE_H264_INVALID;
                
            }            
            break;
        case FRAME_P:    
        {
            slice_param.slice_type=SLICE_TYPE_P;
            int refpiclist0_max = h264->h264_maxref_p0;
            memcpy(slice_param.RefPicList0, RefPicList0_P, refpiclist0_max * sizeof (VAPictureH264));
            for (i = refpiclist0_max; i < 32; i++)
            {
                slice_param.RefPicList0[i].picture_id = VA_INVALID_SURFACE;
                slice_param.RefPicList0[i].flags      = VA_PICTURE_H264_INVALID;
            }
        }
            break;
        case FRAME_B:
        {
            slice_param.slice_type=SLICE_TYPE_B;
            int refpiclist0_max = h264->h264_maxref_p0;
            int refpiclist1_max = h264->h264_maxref_p1;

            memcpy(slice_param.RefPicList0, RefPicList0_B, refpiclist0_max * sizeof (VAPictureH264));
            for (i = refpiclist0_max; i < 32; i++)
            {
                slice_param.RefPicList0[i].picture_id = VA_INVALID_SURFACE;
                slice_param.RefPicList0[i].flags      = VA_PICTURE_H264_INVALID;
            }

            memcpy(slice_param.RefPicList1, RefPicList1_B, refpiclist1_max * sizeof (VAPictureH264));
            for (i = refpiclist1_max; i < 32; i++)
            {
                slice_param.RefPicList1[i].picture_id = VA_INVALID_SURFACE;
                slice_param.RefPicList1[i].flags      = VA_PICTURE_H264_INVALID;
            }
        }
            break;
        default:
            ADM_assert(0);
            break;
    }
   
    slice_param.slice_alpha_c0_offset_div2 = 0;
    slice_param.slice_beta_offset_div2 = 0;
    slice_param.direct_spatial_mv_pred_flag = 1;
    slice_param.pic_order_cnt_lsb = (frameNumber - gop_start) % MaxPicOrderCntLsb;

    
    render_packedslice();

    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(), context_id, VAEncSliceParameterBufferType,
                                        sizeof (slice_param), 1, &slice_param, &slice_param_buf));
    CHECK_VA_STATUS_BOOL(vaRenderPicture(admLibVA::getDisplay(), context_id, &slice_param_buf, 1));
    return true;
}



/**
 * 
 * @param in
 * @param out
 * @return 
 */


bool ADM_vaEncodingContextH264AnnexB::generateExtraData(int *size, uint8_t **data)
{
    ADM_info("vaH264 extraData\n");

    *size=0;
    *data=NULL;    
    ADM_info("/vaH264 extraData\n");
    return true;
}
bool ADM_vaEncodingContextH264AnnexB::encode(ADMImage *in, ADMBitstream *out)
{
    vaFrameType current_frame_type;
    aprintf("Encoding frame %d, H264 AnnexB\n",current_frame_encoding);
    if(!vaSurface[current_frame_encoding%SURFACE_NUM]->fromAdmImage(in))
    {
        ADM_warning("Failed to upload image to vaSurface\n");
        return false;
    }

    encoding2display_order(current_frame_encoding, vaH264Settings.IntraPeriod,    &current_frame_type);
    aprintf("Encoding order = %d,  frame type=%d\n",(int)current_frame_encoding,current_frame_type);
    if (current_frame_type == FRAME_IDR) 
    {
        numShortTerm = 0;
    }
    int current_slot= (current_frame_encoding % SURFACE_NUM);

    CHECK_VA_STATUS_BOOL(vaBeginPicture(admLibVA::getDisplay(), context_id, vaSurface[current_slot]->surface));
    

    if (current_frame_type == FRAME_IDR) 
    {
        render_sequence();
        render_picture(current_frame_encoding,current_frame_type);            
        render_packedsequence();
        render_packedpicture();
        out->flags = AVI_KEY_FRAME;
    }
    else 
    {
        out->flags = AVI_P_FRAME;
        render_picture(current_frame_encoding,current_frame_type);
    }
    render_slice(current_frame_encoding,current_frame_type);
    CHECK_VA_STATUS_BOOL( vaEndPicture(admLibVA::getDisplay(),context_id));
    //--    
    
    CHECK_VA_STATUS_BOOL( vaSyncSurface(admLibVA::getDisplay(), vaSurface[current_frame_encoding % SURFACE_NUM]->surface));
    
    out->len=vaEncodingBuffers[current_frame_encoding % SURFACE_NUM]->read(out->data, out->bufferSize);
    ADM_assert(out->len>=0);

    /* reload a new frame data */

    update_ReferenceFrames(current_frame_type);        
    current_frame_encoding++;
    out->pts=in->Pts;
    out->dts=out->pts;
    return true;
}

ADM_vaEncodingContextH264AnnexB::ADM_vaEncodingContextH264AnnexB()
{
    
}
ADM_vaEncodingContextH264AnnexB::~ADM_vaEncodingContextH264AnnexB()
{
    
}

/**
 * 
 * @param width
 * @param height
 * @param knownSurfaces
 * @return 
 */
bool ADM_vaEncodingContextH264AnnexB::setup( int width, int height, int frameInc,std::vector<ADM_vaSurface *>knownSurfaces)
{
        ADM_info("vaH264 setup\n");
        
        h264=vaGetH264EncoderProfile();
        if(h264->profile==VAProfileNone)
        {
            ADM_error("No H264 encoding support\n");
            return false;
        }
        
        
        VAStatus va_status;
        frame_width=width;
        frame_height=height;
        frame_width_mbaligned=(width+15)&~15;
        frame_height_mbaligned=(height+15)&~15;
        int  i;
        usSecondsToFrac(frameInc,&frameNum,&frameDen);        
        ADM_info("xFps : %d : %d\n",frameNum,frameDen);
        // marshall new config...
        
        // copy common part
        int nAttrib=h264->newAttributes.count();
        VAConfigAttrib *ttrib=new VAConfigAttrib[nAttrib+1];
        const VAConfigAttrib *old=h264->newAttributes.getPointer();
        memcpy(ttrib,old,nAttrib*sizeof(VAConfigAttrib));
        
        // add rate control, it is per instance
        ttrib[nAttrib].type=VAConfigAttribRateControl;
        ttrib[nAttrib].value=VA_RC_CBR;
                
        
        CHECK_VA_STATUS_BOOL( vaCreateConfig(admLibVA::getDisplay(), h264->profile, VAEntrypointEncSlice, ttrib, nAttrib+1, &config_id));
        

        int n=knownSurfaces.size();                    
        VASurfaceID *tmp_surfaceId = new VASurfaceID[n];
        for(int i=0;i<n;i++)
        {
            tmp_surfaceId[i]=knownSurfaces[i]->surface;
        }

        /* Create a context for this encode pipe */
        CHECK_VA_STATUS_BOOL( vaCreateContext(admLibVA::getDisplay(), config_id,
                                    frame_width_mbaligned, frame_height_mbaligned,
                                    VA_PROGRESSIVE,
                                    tmp_surfaceId, n,
                                    &context_id));
        
        delete [] ttrib;
        delete [] tmp_surfaceId;
        tmp_surfaceId=NULL;

        int codedbuf_size = (frame_width_mbaligned * frame_height_mbaligned * 400) / (16*16);

        for (i = 0; i < SURFACE_NUM; i++) 
        {
            vaEncodingBuffers[i]= ADM_vaEncodingBuffers::allocate(context_id,codedbuf_size);
            if(!vaEncodingBuffers[i])
            {
                ADM_warning("Cannot create encoding buffer %d\n",i);
                return false;;
            }
        }

        // Allocate VAImage

        for(int i=0;i<VA_ENC_NB_SURFACE;i++)
        {
            vaSurface[i]=ADM_vaSurface::allocateWithSurface(width,height);
            if(!vaSurface[i]) 
            {
                ADM_warning("Cannot allocate surface\n");
                return false;
            }

            vaRefSurface[i]=ADM_vaSurface::allocateWithSurface(width,height);
            if(!vaRefSurface[i]) 
            {
                ADM_warning("Cannot allocate ref surface\n");
                return false;
            }
        }
        tmpBuffer=new uint8_t[codedbuf_size];
        render_sequence();
        ADM_info("/vaH264 setup\n");
        return true;                
}

// EOF