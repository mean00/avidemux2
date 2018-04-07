
struct __bitstream {
    unsigned int *buffer;
    int bit_offset;
    int max_size_in_dword;
};
typedef struct __bitstream bitstream;


static unsigned int 
va_swap32(unsigned int val)
{
    unsigned char *pval = (unsigned char *)&val;

    return ((pval[0] << 24)     |
            (pval[1] << 16)     |
            (pval[2] << 8)      |
            (pval[3] << 0));
}

static void
bitstream_start(bitstream *bs)
{
    bs->max_size_in_dword = BITSTREAM_ALLOCATE_STEPPING;
    bs->buffer = (unsigned int *)calloc(bs->max_size_in_dword * sizeof(int), 1);
    bs->bit_offset = 0;
}

static void
bitstream_end(bitstream *bs)
{
    int pos = (bs->bit_offset >> 5);
    int bit_offset = (bs->bit_offset & 0x1f);
    int bit_left = 32 - bit_offset;

    if (bit_offset) {
        bs->buffer[pos] = va_swap32((bs->buffer[pos] << bit_left));
    }
}
 
static void
bitstream_put_ui(bitstream *bs, unsigned int val, int size_in_bits)
{
    int pos = (bs->bit_offset >> 5);
    int bit_offset = (bs->bit_offset & 0x1f);
    int bit_left = 32 - bit_offset;

    if (!size_in_bits)
        return;

    bs->bit_offset += size_in_bits;

    if (bit_left > size_in_bits) {
        bs->buffer[pos] = (bs->buffer[pos] << size_in_bits | val);
    } else {
        size_in_bits -= bit_left;
        bs->buffer[pos] = (bs->buffer[pos] << bit_left) | (val >> size_in_bits);
        bs->buffer[pos] = va_swap32(bs->buffer[pos]);

        if (pos + 1 == bs->max_size_in_dword) {
            bs->max_size_in_dword += BITSTREAM_ALLOCATE_STEPPING;
            bs->buffer = (unsigned int *)realloc(bs->buffer, bs->max_size_in_dword * sizeof(unsigned int));
        }

        bs->buffer[pos + 1] = val;
    }
}

static void
bitstream_put_ue(bitstream *bs, unsigned int val)
{
    int size_in_bits = 0;
    int tmp_val = ++val;

    while (tmp_val) {
        tmp_val >>= 1;
        size_in_bits++;
    }

    bitstream_put_ui(bs, 0, size_in_bits - 1); // leading zero
    bitstream_put_ui(bs, val, size_in_bits);
}

static void
bitstream_put_se(bitstream *bs, int val)
{
    unsigned int new_val;

    if (val <= 0)
        new_val = -2 * val;
    else
        new_val = 2 * val - 1;

    bitstream_put_ue(bs, new_val);
}

static void
bitstream_byte_aligning(bitstream *bs, int bit)
{
    int bit_offset = (bs->bit_offset & 0x7);
    int bit_left = 8 - bit_offset;
    int new_val;

    if (!bit_offset)
        return;

    assert(bit == 0 || bit == 1);

    if (bit)
        new_val = (1 << bit_left) - 1;
    else
        new_val = 0;

    bitstream_put_ui(bs, new_val, bit_left);
}

static void 
rbsp_trailing_bits(bitstream *bs)
{
    bitstream_put_ui(bs, 1, 1);
    bitstream_byte_aligning(bs, 0);
}

static void nal_start_code_prefix(bitstream *bs)
{
    bitstream_put_ui(bs, 0x00000001, 32);
}

static void nal_header(bitstream *bs, int nal_ref_idc, int nal_unit_type)
{
    bitstream_put_ui(bs, 0, 1);                /* forbidden_zero_bit: 0 */
    bitstream_put_ui(bs, nal_ref_idc, 2);
    bitstream_put_ui(bs, nal_unit_type, 5);
}

static void sps_rbsp(bitstream *bs)
{
    int profile_idc = PROFILE_IDC_BASELINE;

    if (h264_profile  == VAProfileH264High)
        profile_idc = PROFILE_IDC_HIGH;
    else if (h264_profile  == VAProfileH264Main)
        profile_idc = PROFILE_IDC_MAIN;

    bitstream_put_ui(bs, profile_idc, 8);               /* profile_idc */
    bitstream_put_ui(bs, !!(constraint_set_flag & 1), 1);                         /* constraint_set0_flag */
    bitstream_put_ui(bs, !!(constraint_set_flag & 2), 1);                         /* constraint_set1_flag */
    bitstream_put_ui(bs, !!(constraint_set_flag & 4), 1);                         /* constraint_set2_flag */
    bitstream_put_ui(bs, !!(constraint_set_flag & 8), 1);                         /* constraint_set3_flag */
    bitstream_put_ui(bs, 0, 4);                         /* reserved_zero_4bits */
    bitstream_put_ui(bs, seq_param.level_idc, 8);      /* level_idc */
    bitstream_put_ue(bs, seq_param.seq_parameter_set_id);      /* seq_parameter_set_id */

    if ( profile_idc == PROFILE_IDC_HIGH) {
        bitstream_put_ue(bs, 1);        /* chroma_format_idc = 1, 4:2:0 */ 
        bitstream_put_ue(bs, 0);        /* bit_depth_luma_minus8 */
        bitstream_put_ue(bs, 0);        /* bit_depth_chroma_minus8 */
        bitstream_put_ui(bs, 0, 1);     /* qpprime_y_zero_transform_bypass_flag */
        bitstream_put_ui(bs, 0, 1);     /* seq_scaling_matrix_present_flag */
    }

    bitstream_put_ue(bs, seq_param.seq_fields.bits.log2_max_frame_num_minus4); /* log2_max_frame_num_minus4 */
    bitstream_put_ue(bs, seq_param.seq_fields.bits.pic_order_cnt_type);        /* pic_order_cnt_type */

    if (seq_param.seq_fields.bits.pic_order_cnt_type == 0)
        bitstream_put_ue(bs, seq_param.seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4);     /* log2_max_pic_order_cnt_lsb_minus4 */
    else {
        assert(0);
    }

    bitstream_put_ue(bs, seq_param.max_num_ref_frames);        /* num_ref_frames */
    bitstream_put_ui(bs, 0, 1);                                 /* gaps_in_frame_num_value_allowed_flag */

    bitstream_put_ue(bs, seq_param.picture_width_in_mbs - 1);  /* pic_width_in_mbs_minus1 */
    bitstream_put_ue(bs, seq_param.picture_height_in_mbs - 1); /* pic_height_in_map_units_minus1 */
    bitstream_put_ui(bs, seq_param.seq_fields.bits.frame_mbs_only_flag, 1);    /* frame_mbs_only_flag */

    if (!seq_param.seq_fields.bits.frame_mbs_only_flag) {
        assert(0);
    }

    bitstream_put_ui(bs, seq_param.seq_fields.bits.direct_8x8_inference_flag, 1);      /* direct_8x8_inference_flag */
    bitstream_put_ui(bs, seq_param.frame_cropping_flag, 1);            /* frame_cropping_flag */

    if (seq_param.frame_cropping_flag) {
        bitstream_put_ue(bs, seq_param.frame_crop_left_offset);        /* frame_crop_left_offset */
        bitstream_put_ue(bs, seq_param.frame_crop_right_offset);       /* frame_crop_right_offset */
        bitstream_put_ue(bs, seq_param.frame_crop_top_offset);         /* frame_crop_top_offset */
        bitstream_put_ue(bs, seq_param.frame_crop_bottom_offset);      /* frame_crop_bottom_offset */
    }
    
    //if ( frame_bit_rate < 0 ) { //TODO EW: the vui header isn't correct
    if ( 1 ) {
        bitstream_put_ui(bs, 0, 1); /* vui_parameters_present_flag */
    } else {
        bitstream_put_ui(bs, 1, 1); /* vui_parameters_present_flag */
        bitstream_put_ui(bs, 0, 1); /* aspect_ratio_info_present_flag */
        bitstream_put_ui(bs, 0, 1); /* overscan_info_present_flag */
        bitstream_put_ui(bs, 0, 1); /* video_signal_type_present_flag */
        bitstream_put_ui(bs, 0, 1); /* chroma_loc_info_present_flag */
        bitstream_put_ui(bs, 1, 1); /* timing_info_present_flag */
        {
            bitstream_put_ui(bs, 15, 32);
            bitstream_put_ui(bs, 900, 32);
            bitstream_put_ui(bs, 1, 1);
        }
        bitstream_put_ui(bs, 1, 1); /* nal_hrd_parameters_present_flag */
        {
            // hrd_parameters 
            bitstream_put_ue(bs, 0);    /* cpb_cnt_minus1 */
            bitstream_put_ui(bs, 4, 4); /* bit_rate_scale */
            bitstream_put_ui(bs, 6, 4); /* cpb_size_scale */
           
            bitstream_put_ue(bs, frame_bitrate - 1); /* bit_rate_value_minus1[0] */
            bitstream_put_ue(bs, frame_bitrate*8 - 1); /* cpb_size_value_minus1[0] */
            bitstream_put_ui(bs, 1, 1);  /* cbr_flag[0] */

            bitstream_put_ui(bs, 23, 5);   /* initial_cpb_removal_delay_length_minus1 */
            bitstream_put_ui(bs, 23, 5);   /* cpb_removal_delay_length_minus1 */
            bitstream_put_ui(bs, 23, 5);   /* dpb_output_delay_length_minus1 */
            bitstream_put_ui(bs, 23, 5);   /* time_offset_length  */
        }
        bitstream_put_ui(bs, 0, 1);   /* vcl_hrd_parameters_present_flag */
        bitstream_put_ui(bs, 0, 1);   /* low_delay_hrd_flag */ 

        bitstream_put_ui(bs, 0, 1); /* pic_struct_present_flag */
        bitstream_put_ui(bs, 0, 1); /* bitstream_restriction_flag */
    }

    rbsp_trailing_bits(bs);     /* rbsp_trailing_bits */
}


static void pps_rbsp(bitstream *bs)
{
    bitstream_put_ue(bs, pic_param.pic_parameter_set_id);      /* pic_parameter_set_id */
    bitstream_put_ue(bs, pic_param.seq_parameter_set_id);      /* seq_parameter_set_id */

    bitstream_put_ui(bs, pic_param.pic_fields.bits.entropy_coding_mode_flag, 1);  /* entropy_coding_mode_flag */

    bitstream_put_ui(bs, 0, 1);                         /* pic_order_present_flag: 0 */

    bitstream_put_ue(bs, 0);                            /* num_slice_groups_minus1 */

    bitstream_put_ue(bs, pic_param.num_ref_idx_l0_active_minus1);      /* num_ref_idx_l0_active_minus1 */
    bitstream_put_ue(bs, pic_param.num_ref_idx_l1_active_minus1);      /* num_ref_idx_l1_active_minus1 1 */

    bitstream_put_ui(bs, pic_param.pic_fields.bits.weighted_pred_flag, 1);     /* weighted_pred_flag: 0 */
    bitstream_put_ui(bs, pic_param.pic_fields.bits.weighted_bipred_idc, 2);	/* weighted_bipred_idc: 0 */

    bitstream_put_se(bs, pic_param.pic_init_qp - 26);  /* pic_init_qp_minus26 */
    bitstream_put_se(bs, 0);                            /* pic_init_qs_minus26 */
    bitstream_put_se(bs, 0);                            /* chroma_qp_index_offset */

    bitstream_put_ui(bs, pic_param.pic_fields.bits.deblocking_filter_control_present_flag, 1); /* deblocking_filter_control_present_flag */
    bitstream_put_ui(bs, 0, 1);                         /* constrained_intra_pred_flag */
    bitstream_put_ui(bs, 0, 1);                         /* redundant_pic_cnt_present_flag */
    
    /* more_rbsp_data */
    bitstream_put_ui(bs, pic_param.pic_fields.bits.transform_8x8_mode_flag, 1);    /*transform_8x8_mode_flag */
    bitstream_put_ui(bs, 0, 1);                         /* pic_scaling_matrix_present_flag */
    bitstream_put_se(bs, pic_param.second_chroma_qp_index_offset );    /*second_chroma_qp_index_offset */

    rbsp_trailing_bits(bs);
}

static void slice_header(bitstream *bs)
{
    int first_mb_in_slice = slice_param.macroblock_address;

    bitstream_put_ue(bs, first_mb_in_slice);        /* first_mb_in_slice: 0 */
    bitstream_put_ue(bs, slice_param.slice_type);   /* slice_type */
    bitstream_put_ue(bs, slice_param.pic_parameter_set_id);        /* pic_parameter_set_id: 0 */
    bitstream_put_ui(bs, pic_param.frame_num, seq_param.seq_fields.bits.log2_max_frame_num_minus4 + 4); /* frame_num */

    /* frame_mbs_only_flag == 1 */
    if (!seq_param.seq_fields.bits.frame_mbs_only_flag) {
        /* FIXME: */
        assert(0);
    }

    if (pic_param.pic_fields.bits.idr_pic_flag)
        bitstream_put_ue(bs, slice_param.idr_pic_id);		/* idr_pic_id: 0 */

    if (seq_param.seq_fields.bits.pic_order_cnt_type == 0) {
        bitstream_put_ui(bs, pic_param.CurrPic.TopFieldOrderCnt, seq_param.seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4 + 4);
        /* pic_order_present_flag == 0 */
    } else {
        /* FIXME: */
        assert(0);
    }

    /* redundant_pic_cnt_present_flag == 0 */
    /* slice type */
    if (IS_P_SLICE(slice_param.slice_type)) {
        bitstream_put_ui(bs, slice_param.num_ref_idx_active_override_flag, 1);            /* num_ref_idx_active_override_flag: */

        if (slice_param.num_ref_idx_active_override_flag)
            bitstream_put_ue(bs, slice_param.num_ref_idx_l0_active_minus1);

        /* ref_pic_list_reordering */
        bitstream_put_ui(bs, 0, 1);            /* ref_pic_list_reordering_flag_l0: 0 */
    } else if (IS_B_SLICE(slice_param.slice_type)) {
        bitstream_put_ui(bs, slice_param.direct_spatial_mv_pred_flag, 1);            /* direct_spatial_mv_pred: 1 */

        bitstream_put_ui(bs, slice_param.num_ref_idx_active_override_flag, 1);       /* num_ref_idx_active_override_flag: */

        if (slice_param.num_ref_idx_active_override_flag) {
            bitstream_put_ue(bs, slice_param.num_ref_idx_l0_active_minus1);
            bitstream_put_ue(bs, slice_param.num_ref_idx_l1_active_minus1);
        }

        /* ref_pic_list_reordering */
        bitstream_put_ui(bs, 0, 1);            /* ref_pic_list_reordering_flag_l0: 0 */
        bitstream_put_ui(bs, 0, 1);            /* ref_pic_list_reordering_flag_l1: 0 */
    }

    if ((pic_param.pic_fields.bits.weighted_pred_flag &&
         IS_P_SLICE(slice_param.slice_type)) ||
        ((pic_param.pic_fields.bits.weighted_bipred_idc == 1) &&
         IS_B_SLICE(slice_param.slice_type))) {
        /* FIXME: fill weight/offset table */
        assert(0);
    }

    /* dec_ref_pic_marking */
    if (pic_param.pic_fields.bits.reference_pic_flag) {     /* nal_ref_idc != 0 */
        unsigned char no_output_of_prior_pics_flag = 0;
        unsigned char long_term_reference_flag = 0;
        unsigned char adaptive_ref_pic_marking_mode_flag = 0;

        if (pic_param.pic_fields.bits.idr_pic_flag) {
            bitstream_put_ui(bs, no_output_of_prior_pics_flag, 1);            /* no_output_of_prior_pics_flag: 0 */
            bitstream_put_ui(bs, long_term_reference_flag, 1);            /* long_term_reference_flag: 0 */
        } else {
            bitstream_put_ui(bs, adaptive_ref_pic_marking_mode_flag, 1);            /* adaptive_ref_pic_marking_mode_flag: 0 */
        }
    }

    if (pic_param.pic_fields.bits.entropy_coding_mode_flag &&
        !IS_I_SLICE(slice_param.slice_type))
        bitstream_put_ue(bs, slice_param.cabac_init_idc);               /* cabac_init_idc: 0 */

    bitstream_put_se(bs, slice_param.slice_qp_delta);                   /* slice_qp_delta: 0 */

    /* ignore for SP/SI */

    if (pic_param.pic_fields.bits.deblocking_filter_control_present_flag) {
        bitstream_put_ue(bs, slice_param.disable_deblocking_filter_idc);           /* disable_deblocking_filter_idc: 0 */

        if (slice_param.disable_deblocking_filter_idc != 1) {
            bitstream_put_se(bs, slice_param.slice_alpha_c0_offset_div2);          /* slice_alpha_c0_offset_div2: 2 */
            bitstream_put_se(bs, slice_param.slice_beta_offset_div2);              /* slice_beta_offset_div2: 2 */
        }
    }

    if (pic_param.pic_fields.bits.entropy_coding_mode_flag) {
        bitstream_byte_aligning(bs, 1);
    }
}
