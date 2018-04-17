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
#include "va/va.h"
#include "va/va_enc_h264.h"
#include "ADM_coreLibVA_buffer.h"
#include "ADM_libVaEncodingContextH264.h"

#include "vaenc_settings.h"
extern vaconf_settings vaH264Settings;
/**
 * 
 * @param bs
 */
void ADM_vaEncodingContextH264Base::sps_rbsp(vaBitstream *bs)
{
    int profile_idc = PROFILE_IDC_BASELINE;
    int constraint_set_flag=0;    
    
    switch(h264->profile)
    {
    case VAProfileH264High:
        profile_idc = PROFILE_IDC_HIGH;
        constraint_set_flag=(1 << 3);
        break;
    case VAProfileH264Main:
        profile_idc = PROFILE_IDC_MAIN;
         constraint_set_flag=(1 << 1);
         break;
    default:
        ADM_assert(0);
        break;
    }   
    bs->put_ui(profile_idc, 8); /* profile_idc */
    
    bs->put_ui(!!(constraint_set_flag & 1), 1); /* constraint_set0_flag */
    bs->put_ui(!!(constraint_set_flag & 2), 1); /* constraint_set1_flag */
    bs->put_ui(!!(constraint_set_flag & 4), 1); /* constraint_set2_flag */
    bs->put_ui(!!(constraint_set_flag & 8), 1); /* constraint_set3_flag */
    bs->put_ui(0, 4); /* reserved_zero_4bits */
    bs->put_ui(seq_param.level_idc, 8); /* level_idc */
    bs->put_ue(seq_param.seq_parameter_set_id); /* seq_parameter_set_id */

    if (profile_idc == PROFILE_IDC_HIGH)
    {
        bs->put_ue(1); /* chroma_format_idc = 1, 4:2:0 */
        bs->put_ue(0); /* bit_depth_luma_minus8 */
        bs->put_ue(0); /* bit_depth_chroma_minus8 */
        bs->put_ui(0, 1); /* qpprime_y_zero_transform_bypass_flag */
        bs->put_ui(0, 1); /* seq_scaling_matrix_present_flag */
    }

    bs->put_ue(seq_param.seq_fields.bits.log2_max_frame_num_minus4); /* log2_max_frame_num_minus4 */
    bs->put_ue(seq_param.seq_fields.bits.pic_order_cnt_type); /* pic_order_cnt_type */

    if (seq_param.seq_fields.bits.pic_order_cnt_type == 0)
        bs->put_ue(seq_param.seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4); /* log2_max_pic_order_cnt_lsb_minus4 */
    else
    {
        assert(0);
    }

    bs->put_ue(seq_param.max_num_ref_frames); /* num_ref_frames */
    bs->put_ui(0, 1); /* gaps_in_frame_num_value_allowed_flag */

    bs->put_ue(seq_param.picture_width_in_mbs - 1); /* pic_width_in_mbs_minus1 */
    bs->put_ue(seq_param.picture_height_in_mbs - 1); /* pic_height_in_map_units_minus1 */
    bs->put_ui(seq_param.seq_fields.bits.frame_mbs_only_flag, 1); /* frame_mbs_only_flag */

    if (!seq_param.seq_fields.bits.frame_mbs_only_flag)
    {
        assert(0);
    }

    bs->put_ui(seq_param.seq_fields.bits.direct_8x8_inference_flag, 1); /* direct_8x8_inference_flag */
    bs->put_ui(seq_param.frame_cropping_flag, 1); /* frame_cropping_flag */

    if (seq_param.frame_cropping_flag)
    {
        bs->put_ue(seq_param.frame_crop_left_offset); /* frame_crop_left_offset */
        bs->put_ue(seq_param.frame_crop_right_offset); /* frame_crop_right_offset */
        bs->put_ue(seq_param.frame_crop_top_offset); /* frame_crop_top_offset */
        bs->put_ue(seq_param.frame_crop_bottom_offset); /* frame_crop_bottom_offset */
    }

    //if ( frame_bit_rate < 0 ) { //TODO EW: the vui header isn't correct
    if (1)
    {
        bs->put_ui(0, 1); /* vui_parameters_present_flag */
    }
    else
    {
        bs->put_ui(1, 1); /* vui_parameters_present_flag */
        bs->put_ui(0, 1); /* aspect_ratio_info_present_flag */
        bs->put_ui(0, 1); /* overscan_info_present_flag */
        bs->put_ui(0, 1); /* video_signal_type_present_flag */
        bs->put_ui(0, 1); /* chroma_loc_info_present_flag */
        bs->put_ui(1, 1); /* timing_info_present_flag */
        {
            bs->put_ui(15, 32);
            bs->put_ui(900, 32);
            bs->put_ui(1, 1);
        }
        bs->put_ui(1, 1); /* nal_hrd_parameters_present_flag */
        {
            // hrd_parameters 
            bs->put_ue(0); /* cpb_cnt_minus1 */
            bs->put_ui(4, 4); /* bit_rate_scale */
            bs->put_ui(6, 4); /* cpb_size_scale */

            bs->put_ue(vaH264Settings.BitrateKbps*1000 - 1); /* bit_rate_value_minus1[0] */
            bs->put_ue(vaH264Settings.BitrateKbps * 8000 - 1); /* cpb_size_value_minus1[0] */
            bs->put_ui(1, 1); /* cbr_flag[0] */

            bs->put_ui(23, 5); /* initial_cpb_removal_delay_length_minus1 */
            bs->put_ui(23, 5); /* cpb_removal_delay_length_minus1 */
            bs->put_ui(23, 5); /* dpb_output_delay_length_minus1 */
            bs->put_ui(23, 5); /* time_offset_length  */
        }
        bs->put_ui(0, 1); /* vcl_hrd_parameters_present_flag */
        bs->put_ui(0, 1); /* low_delay_hrd_flag */

        bs->put_ui(0, 1); /* pic_struct_present_flag */
        bs->put_ui(0, 1); /* bitstream_restriction_flag */
    }
    bs->rbspTrailingBits();
    /* rbsp_trailing_bits */
}

/**
 * 
 * @param bs
 */
void ADM_vaEncodingContextH264Base::pps_rbsp(vaBitstream *bs)
{
    bs->put_ue(pic_param.pic_parameter_set_id); /* pic_parameter_set_id */
    bs->put_ue(pic_param.seq_parameter_set_id); /* seq_parameter_set_id */

    bs->put_ui(pic_param.pic_fields.bits.entropy_coding_mode_flag, 1); /* entropy_coding_mode_flag */

    bs->put_ui(0, 1); /* pic_order_present_flag: 0 */

    bs->put_ue(0); /* num_slice_groups_minus1 */

    bs->put_ue(pic_param.num_ref_idx_l0_active_minus1); /* num_ref_idx_l0_active_minus1 */
    bs->put_ue(pic_param.num_ref_idx_l1_active_minus1); /* num_ref_idx_l1_active_minus1 1 */

    bs->put_ui(pic_param.pic_fields.bits.weighted_pred_flag, 1); /* weighted_pred_flag: 0 */
    bs->put_ui(pic_param.pic_fields.bits.weighted_bipred_idc, 2); /* weighted_bipred_idc: 0 */

    bs->put_se(pic_param.pic_init_qp - 26); /* pic_init_qp_minus26 */
    bs->put_se(0); /* pic_init_qs_minus26 */
    bs->put_se(0); /* chroma_qp_index_offset */

    bs->put_ui(pic_param.pic_fields.bits.deblocking_filter_control_present_flag, 1); /* deblocking_filter_control_present_flag */
    bs->put_ui(0, 1); /* constrained_intra_pred_flag */
    bs->put_ui(0, 1); /* redundant_pic_cnt_present_flag */

    /* more_rbsp_data */
    bs->put_ui(pic_param.pic_fields.bits.transform_8x8_mode_flag, 1); /*transform_8x8_mode_flag */
    bs->put_ui(0, 1); /* pic_scaling_matrix_present_flag */
    bs->put_se(pic_param.second_chroma_qp_index_offset); /*second_chroma_qp_index_offset */

    bs->rbspTrailingBits();
}

/**
 * 
 * @param bs
 */
void ADM_vaEncodingContextH264Base::slice_header(vaBitstream *bs)
{
    int first_mb_in_slice = slice_param.macroblock_address;

    bs->put_ue(first_mb_in_slice); /* first_mb_in_slice: 0 */
    bs->put_ue(slice_param.slice_type); /* slice_type */
    bs->put_ue(slice_param.pic_parameter_set_id); /* pic_parameter_set_id: 0 */
    bs->put_ui(pic_param.frame_num, seq_param.seq_fields.bits.log2_max_frame_num_minus4 + 4); /* frame_num */

    /* frame_mbs_only_flag == 1 */
    if (!seq_param.seq_fields.bits.frame_mbs_only_flag)
    {
        /* FIXME: */
        assert(0);
    }

    if (pic_param.pic_fields.bits.idr_pic_flag)
        bs->put_ue(slice_param.idr_pic_id); /* idr_pic_id: 0 */

    if (seq_param.seq_fields.bits.pic_order_cnt_type == 0)
    {
        bs->put_ui(pic_param.CurrPic.TopFieldOrderCnt, seq_param.seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4 + 4);
        /* pic_order_present_flag == 0 */
    }
    else
    {
        /* FIXME: */
        assert(0);
    }

    /* redundant_pic_cnt_present_flag == 0 */
    /* slice type */
    if (IS_P_SLICE(slice_param.slice_type))
    {
        bs->put_ui(slice_param.num_ref_idx_active_override_flag, 1); /* num_ref_idx_active_override_flag: */

        if (slice_param.num_ref_idx_active_override_flag)
            bs->put_ue(slice_param.num_ref_idx_l0_active_minus1);

        /* ref_pic_list_reordering */
        bs->put_ui(0, 1); /* ref_pic_list_reordering_flag_l0: 0 */
    }
    else if (IS_B_SLICE(slice_param.slice_type))
    {
        bs->put_ui(slice_param.direct_spatial_mv_pred_flag, 1); /* direct_spatial_mv_pred: 1 */

        bs->put_ui(slice_param.num_ref_idx_active_override_flag, 1); /* num_ref_idx_active_override_flag: */

        if (slice_param.num_ref_idx_active_override_flag)
        {
            bs->put_ue(slice_param.num_ref_idx_l0_active_minus1);
            bs->put_ue(slice_param.num_ref_idx_l1_active_minus1);
        }

        /* ref_pic_list_reordering */
        bs->put_ui(0, 1); /* ref_pic_list_reordering_flag_l0: 0 */
        bs->put_ui(0, 1); /* ref_pic_list_reordering_flag_l1: 0 */
    }

    if ((pic_param.pic_fields.bits.weighted_pred_flag &&
            IS_P_SLICE(slice_param.slice_type)) ||
            ((pic_param.pic_fields.bits.weighted_bipred_idc == 1) &&
            IS_B_SLICE(slice_param.slice_type)))
    {
        /* FIXME: fill weight/offset table */
        assert(0);
    }

    /* dec_ref_pic_marking */
    if (pic_param.pic_fields.bits.reference_pic_flag)
    { /* nal_ref_idc != 0 */
        unsigned char no_output_of_prior_pics_flag = 0;
        unsigned char long_term_reference_flag = 0;
        unsigned char adaptive_ref_pic_marking_mode_flag = 0;

        if (pic_param.pic_fields.bits.idr_pic_flag)
        {
            bs->put_ui(no_output_of_prior_pics_flag, 1); /* no_output_of_prior_pics_flag: 0 */
            bs->put_ui(long_term_reference_flag, 1); /* long_term_reference_flag: 0 */
        }
        else
        {
            bs->put_ui(adaptive_ref_pic_marking_mode_flag, 1); /* adaptive_ref_pic_marking_mode_flag: 0 */
        }
    }

    if (pic_param.pic_fields.bits.entropy_coding_mode_flag &&
            !IS_I_SLICE(slice_param.slice_type))
        bs->put_ue(slice_param.cabac_init_idc); /* cabac_init_idc: 0 */

    bs->put_se(slice_param.slice_qp_delta); /* slice_qp_delta: 0 */

    /* ignore for SP/SI */

    if (pic_param.pic_fields.bits.deblocking_filter_control_present_flag)
    {
        bs->put_ue(slice_param.disable_deblocking_filter_idc); /* disable_deblocking_filter_idc: 0 */

        if (slice_param.disable_deblocking_filter_idc != 1)
        {
            bs->put_se(slice_param.slice_alpha_c0_offset_div2); /* slice_alpha_c0_offset_div2: 2 */
            bs->put_se(slice_param.slice_beta_offset_div2); /* slice_beta_offset_div2: 2 */
        }
    }

    if (pic_param.pic_fields.bits.entropy_coding_mode_flag)
    {
        bs->byteAlign(1);
    }
}


/*
 * Return displaying order with specified periods and encoding order
 * displaying_order: displaying order
 * frame_type: frame type 
 */
void ADM_vaEncodingContextH264Base::encoding2display_order(uint64_t encoding_order, int intra_idr_period, vaFrameType *frame_type)
{
    // simple use case, no delay encoding, only I(DR) PPPPPP
    if (!encoding_order)
    {
        gop_start = 0;
        *frame_type = FRAME_IDR;
    }
    else
    {
        int offset;
        offset = encoding_order - gop_start;
        if (offset >= intra_idr_period)
        {
            gop_start = encoding_order;
            offset = 0;
            *frame_type = FRAME_IDR;
        }
        else
        {
            *frame_type = FRAME_P;
        }
    }
}

#if 0    
int encoding_order_gop = 0;

if (intra_period == 1)
{ /* all are I/IDR frames */
    *displaying_order = encoding_order;
    if (intra_idr_period == 0)
        *frame_type = (encoding_order == 0) ? FRAME_IDR : FRAME_I;
    else
        *frame_type = (encoding_order % intra_idr_period == 0) ? FRAME_IDR : FRAME_I;
    return;
}

if (intra_period == 0)
    intra_idr_period = 0;

/* new sequence like
 * IDR PPPPP IPPPPP
 * IDR (PBB)(PBB)(IBB)(PBB)
 */
encoding_order_gop = (intra_idr_period == 0) ? encoding_order :
        (encoding_order % (intra_idr_period + ((ip_period == 1) ? 0 : 1)));

if (encoding_order_gop == 0)
{ /* the first frame */
    *frame_type = FRAME_IDR;
    *displaying_order = encoding_order;
}
else if (((encoding_order_gop - 1) % ip_period) != 0)
{ /* B frames */
    *frame_type = FRAME_B;
    *displaying_order = encoding_order - 1;
}
else if ((intra_period != 0) && /* have I frames */
        (encoding_order_gop >= 2) &&
        ((ip_period == 1 && encoding_order_gop % intra_period == 0) || /* for IDR PPPPP IPPPP */
        /* for IDR (PBB)(PBB)(IBB) */
        (ip_period >= 2 && ((encoding_order_gop - 1) / ip_period % (intra_period / ip_period)) == 0)))
{
    *frame_type = FRAME_I;
    *displaying_order = encoding_order + ip_period - 1;
}
else
{
    *frame_type = FRAME_P;
    *displaying_order = encoding_order + ip_period - 1;
}
#endif


// EOF