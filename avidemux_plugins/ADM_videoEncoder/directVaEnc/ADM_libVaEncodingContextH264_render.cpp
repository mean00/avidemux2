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
#include "va/va.h"
#include "va/va_enc_h264.h"
#include "ADM_coreLibVA_buffer.h"
#include "ADM_libVaEncodingContextH264.h"



#define partition(ref, field, key, ascending)   \
    while (i <= j) {                            \
        if (ascending) {                        \
            while (ref[i].field < key)          \
                i++;                            \
            while (ref[j].field > key)          \
                j--;                            \
        } else {                                \
            while (ref[i].field > key)          \
                i++;                            \
            while (ref[j].field < key)          \
                j--;                            \
        }                                       \
        if (i <= j) {                           \
            tmp = ref[i];                       \
            ref[i] = ref[j];                    \
            ref[j] = tmp;                       \
            i++;                                \
            j--;                                \
        }                                       \
    }                                           \


/**
 * 
 * @param ref
 * @param left
 * @param right
 * @param ascending
 * @param frame_idx
 */
static void sort_one(VAPictureH264 ref[], int left, int right,
                     int ascending, int frame_idx)
{
    int i = left, j = right;
    unsigned int key;
    VAPictureH264 tmp;

    if (frame_idx)
    {
        key = ref[(left + right) / 2].frame_idx;
        partition(ref, frame_idx, key, ascending);
    }
    else
    {
        key = ref[(left + right) / 2].TopFieldOrderCnt;
        partition(ref, TopFieldOrderCnt, (signed int) key, ascending);
    }

    /* recursion */
    if (left < j)
        sort_one(ref, left, j, ascending, frame_idx);

    if (i < right)
        sort_one(ref, i, right, ascending, frame_idx);
}

/**
 * 
 * @param ref
 * @param left
 * @param right
 * @param key
 * @param frame_idx
 * @param partition_ascending
 * @param list0_ascending
 * @param list1_ascending
 */
static void sort_two(VAPictureH264 ref[], int left, int right, unsigned int key, unsigned int frame_idx,
                     int partition_ascending, int list0_ascending, int list1_ascending)
{
    int i = left, j = right;
    VAPictureH264 tmp;

    if (frame_idx)
    {
        partition(ref, frame_idx, key, partition_ascending);
    }
    else
    {
        partition(ref, TopFieldOrderCnt, (signed int) key, partition_ascending);
    }


    sort_one(ref, left, i - 1, list0_ascending, frame_idx);
    sort_one(ref, j + 1, right, list1_ascending, frame_idx);
}

/**
 * 
 * @return 
 */
bool ADM_vaEncodingContextH264Base::update_ReferenceFrames(vaFrameType frameType)
{
    if (frameType == FRAME_B)     
        return true;

    CurrentCurrPic.flags = VA_PICTURE_H264_SHORT_TERM_REFERENCE;
    numShortTerm++;
    if (numShortTerm > num_ref_frames)
        numShortTerm = num_ref_frames;   
    for (int i = numShortTerm - 1; i > 0; i--)
        ReferenceFrames[i] = ReferenceFrames[i - 1];
    ReferenceFrames[0] = CurrentCurrPic;
    return true;
}

/**
 * 
 * @return 
 */
bool ADM_vaEncodingContextH264Base::update_RefPicList(vaFrameType frameType)
{    
    unsigned int current_poc = CurrentCurrPic.TopFieldOrderCnt;
    switch(frameType)
    {
    case  FRAME_P:
        memcpy(RefPicList0_P, ReferenceFrames, numShortTerm * sizeof (VAPictureH264));
        sort_one(RefPicList0_P, 0, numShortTerm - 1, 0, 1);
        break;
    case FRAME_B:
        memcpy(RefPicList0_B, ReferenceFrames, numShortTerm * sizeof (VAPictureH264));
        sort_two(RefPicList0_B, 0, numShortTerm - 1, current_poc, 0,  1, 0, 1);

        memcpy(RefPicList1_B, ReferenceFrames, numShortTerm * sizeof (VAPictureH264));
        sort_two(RefPicList1_B, 0, numShortTerm - 1, current_poc, 0,  0, 1, 0);
        break;
    default:
        break;
    }
    return true;
}

void ADM_vaEncodingContextH264Base::fillSeqParam()
{
    seq_param.level_idc = 41 /*SH_LEVEL_3*/;
    seq_param.picture_width_in_mbs = frame_width_mbaligned / 16;
    seq_param.picture_height_in_mbs = frame_height_mbaligned / 16;
    seq_param.bits_per_second = VA_BITRATE;
    
#warning fixme
    seq_param.intra_idr_period = vaH264Settings.IntraPeriod;
    seq_param.ip_period = 10000;
#warning fixme too
    seq_param.max_num_ref_frames = 16; //num_ref_frames; // WTF ?
    seq_param.seq_fields.bits.frame_mbs_only_flag = 1;
    // FRAMERATE
    seq_param.time_scale = frameDen;
    seq_param.num_units_in_tick = frameNum; /* Tc = num_units_in_tick / time_sacle */
    //
    seq_param.seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4 = Log2MaxPicOrderCntLsb - 4;
    seq_param.seq_fields.bits.log2_max_frame_num_minus4 = Log2MaxFrameNum - 4;
    seq_param.seq_fields.bits.frame_mbs_only_flag = 1;
    seq_param.seq_fields.bits.chroma_format_idc = 1;
    seq_param.seq_fields.bits.direct_8x8_inference_flag = 1;
    // PAN / SCAN
    if (frame_width != frame_width_mbaligned ||
            frame_height != frame_height_mbaligned)
    {
        seq_param.frame_cropping_flag = 1;
        seq_param.frame_crop_left_offset = 0;
        seq_param.frame_crop_right_offset = (frame_width_mbaligned - frame_width) / 2;
        seq_param.frame_crop_top_offset = 0;
        seq_param.frame_crop_bottom_offset = (frame_height_mbaligned - frame_height) / 2;
    }
}
/**
 * 
 * @return 
 */
bool ADM_vaEncodingContextH264Base::render_sequence(void)
{
    VABufferID seq_param_buf, rc_param_buf, misc_param_tmpbuf, render_id[2];
    VAStatus va_status;
    VAEncMiscParameterBuffer *misc_param, *misc_param_tmp;
    VAEncMiscParameterRateControl *misc_rate_ctrl;

    fillSeqParam();
    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(), context_id,
                                        VAEncSequenceParameterBufferType,
                                        sizeof (seq_param), 1, &seq_param, &seq_param_buf));


    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(), context_id,
                                        VAEncMiscParameterBufferType,
                                        sizeof (VAEncMiscParameterBuffer) + sizeof (VAEncMiscParameterRateControl),
                                        1, NULL, &rc_param_buf));


    vaMapBuffer(admLibVA::getDisplay(), rc_param_buf, (void **) &misc_param);
    misc_param->type = VAEncMiscParameterTypeRateControl;
    misc_rate_ctrl = (VAEncMiscParameterRateControl *) misc_param->data;
    memset(misc_rate_ctrl, 0, sizeof (*misc_rate_ctrl));
    misc_rate_ctrl->bits_per_second = VA_BITRATE;
    misc_rate_ctrl->target_percentage = 95;
    misc_rate_ctrl->window_size = 1000;
    misc_rate_ctrl->initial_qp = initial_qp;
    misc_rate_ctrl->min_qp = minimal_qp;
    misc_rate_ctrl->basic_unit_size = 0;
    vaUnmapBuffer(admLibVA::getDisplay(), rc_param_buf);
    render_id[0] = seq_param_buf;
    render_id[1] = rc_param_buf;
    CHECK_VA_STATUS_BOOL(vaRenderPicture(admLibVA::getDisplay(), context_id, &render_id[0], 2));
    return true;
}

/**
 * 
 * @param pic_order_cnt_lsb
 * @return 
 */
int ADM_vaEncodingContextH264Base::calc_poc(int pic_order_cnt_lsb,vaFrameType frameType)
{
    static int PicOrderCntMsb_ref = 0, pic_order_cnt_lsb_ref = 0;
    int prevPicOrderCntMsb, prevPicOrderCntLsb;
    int PicOrderCntMsb, TopFieldOrderCnt;

    if (frameType == FRAME_IDR)
        prevPicOrderCntMsb = prevPicOrderCntLsb = 0;
    else
    {
        prevPicOrderCntMsb = PicOrderCntMsb_ref;
        prevPicOrderCntLsb = pic_order_cnt_lsb_ref;
    }

    if ((pic_order_cnt_lsb < prevPicOrderCntLsb) &&
            ((prevPicOrderCntLsb - pic_order_cnt_lsb) >= (int) (MaxPicOrderCntLsb / 2)))
        PicOrderCntMsb = prevPicOrderCntMsb + MaxPicOrderCntLsb;
    else if ((pic_order_cnt_lsb > prevPicOrderCntLsb) &&
            ((pic_order_cnt_lsb - prevPicOrderCntLsb) > (int) (MaxPicOrderCntLsb / 2)))
        PicOrderCntMsb = prevPicOrderCntMsb - MaxPicOrderCntLsb;
    else
        PicOrderCntMsb = prevPicOrderCntMsb;

    TopFieldOrderCnt = PicOrderCntMsb + pic_order_cnt_lsb;

    if (frameType != FRAME_B)
    {
        PicOrderCntMsb_ref = PicOrderCntMsb;
        pic_order_cnt_lsb_ref = pic_order_cnt_lsb;
    }

    return TopFieldOrderCnt;
}
/**
 * 
 */
void ADM_vaEncodingContextH264Base::fillPPS(int frameNumber, vaFrameType frameType)
{
    int current_slot = (frameNumber % SURFACE_NUM);
    pic_param.CurrPic.picture_id = vaRefSurface[current_slot]->surface;
    pic_param.CurrPic.frame_idx = frameNumber-gop_start;
    pic_param.CurrPic.flags = 0;
    pic_param.CurrPic.TopFieldOrderCnt = calc_poc((frameNumber - gop_start) % MaxPicOrderCntLsb,frameType);
    pic_param.CurrPic.BottomFieldOrderCnt = pic_param.CurrPic.TopFieldOrderCnt;
    CurrentCurrPic = pic_param.CurrPic;    
    
    if(frameType==FRAME_IDR)
    {
        numShortTerm = 0;
    }
    
    if(numShortTerm)
        memcpy(pic_param.ReferenceFrames, ReferenceFrames, numShortTerm * sizeof (VAPictureH264));
    for (int i = numShortTerm; i < SURFACE_NUM; i++)
    {
        pic_param.ReferenceFrames[i].picture_id = VA_INVALID_SURFACE;
        pic_param.ReferenceFrames[i].flags = VA_PICTURE_H264_INVALID;
    }

    pic_param.pic_fields.bits.idr_pic_flag = (frameType == FRAME_IDR);
    pic_param.pic_fields.bits.reference_pic_flag = (frameType != FRAME_B);
    pic_param.pic_fields.bits.entropy_coding_mode_flag = 1;
    pic_param.pic_fields.bits.deblocking_filter_control_present_flag = 1;
    pic_param.frame_num = frameNumber-gop_start;
    pic_param.coded_buf = vaEncodingBuffers[current_slot]->getId();
    pic_param.last_picture = 0;
    pic_param.pic_init_qp = initial_qp;
}
/**
 * 
 * @return 
 */
bool ADM_vaEncodingContextH264Base::render_picture(int frameNumber,vaFrameType frameType)
{
    VABufferID pic_param_buf;
    VAStatus va_status;
    int i = 0;
    fillPPS(frameNumber,frameType);
    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(), context_id, VAEncPictureParameterBufferType,
                                        sizeof (pic_param), 1, &pic_param, &pic_param_buf));

    CHECK_VA_STATUS_BOOL(vaRenderPicture(admLibVA::getDisplay(), context_id, &pic_param_buf, 1));
    return true;
}




//--

/**
 * 
 * @return 
 */
bool ADM_vaEncodingContextH264Base::render_slice(int frameNumber,vaFrameType frameType)
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

    
    

    CHECK_VA_STATUS_BOOL(vaCreateBuffer(admLibVA::getDisplay(), context_id, VAEncSliceParameterBufferType,
                                        sizeof (slice_param), 1, &slice_param, &slice_param_buf));
    CHECK_VA_STATUS_BOOL(vaRenderPicture(admLibVA::getDisplay(), context_id, &slice_param_buf, 1));
    return true;
}

// EOF
