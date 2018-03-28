
static int
build_packed_pic_buffer(unsigned char **header_buffer)
{
    bitstream bs;

    bitstream_start(&bs);
    nal_start_code_prefix(&bs);
    nal_header(&bs, NAL_REF_IDC_HIGH, NAL_PPS);
    pps_rbsp(&bs);
    bitstream_end(&bs);

    *header_buffer = (unsigned char *)bs.buffer;
    return bs.bit_offset;
}

static int
build_packed_seq_buffer(unsigned char **header_buffer)
{
    bitstream bs;

    bitstream_start(&bs);
    nal_start_code_prefix(&bs);
    nal_header(&bs, NAL_REF_IDC_HIGH, NAL_SPS);
    sps_rbsp(&bs);
    bitstream_end(&bs);

    *header_buffer = (unsigned char *)bs.buffer;
    return bs.bit_offset;
}

static int 
build_packed_sei_buffer_timing(unsigned int init_cpb_removal_length,
				unsigned int init_cpb_removal_delay,
				unsigned int init_cpb_removal_delay_offset,
				unsigned int cpb_removal_length,
				unsigned int cpb_removal_delay,
				unsigned int dpb_output_length,
				unsigned int dpb_output_delay,
				unsigned char **sei_buffer)
{
    unsigned char *byte_buf;
    int bp_byte_size, i, pic_byte_size;

    bitstream nal_bs;
    bitstream sei_bp_bs, sei_pic_bs;

    bitstream_start(&sei_bp_bs);
    bitstream_put_ue(&sei_bp_bs, 0);       /*seq_parameter_set_id*/
    bitstream_put_ui(&sei_bp_bs, init_cpb_removal_delay, cpb_removal_length); 
    bitstream_put_ui(&sei_bp_bs, init_cpb_removal_delay_offset, cpb_removal_length); 
    if ( sei_bp_bs.bit_offset & 0x7) {
        bitstream_put_ui(&sei_bp_bs, 1, 1);
    }
    bitstream_end(&sei_bp_bs);
    bp_byte_size = (sei_bp_bs.bit_offset + 7) / 8;
    
    bitstream_start(&sei_pic_bs);
    bitstream_put_ui(&sei_pic_bs, cpb_removal_delay, cpb_removal_length); 
    bitstream_put_ui(&sei_pic_bs, dpb_output_delay, dpb_output_length); 
    if ( sei_pic_bs.bit_offset & 0x7) {
        bitstream_put_ui(&sei_pic_bs, 1, 1);
    }
    bitstream_end(&sei_pic_bs);
    pic_byte_size = (sei_pic_bs.bit_offset + 7) / 8;
    
    bitstream_start(&nal_bs);
    nal_start_code_prefix(&nal_bs);
    nal_header(&nal_bs, NAL_REF_IDC_NONE, NAL_SEI);

	/* Write the SEI buffer period data */    
    bitstream_put_ui(&nal_bs, 0, 8);
    bitstream_put_ui(&nal_bs, bp_byte_size, 8);
    
    byte_buf = (unsigned char *)sei_bp_bs.buffer;
    for(i = 0; i < bp_byte_size; i++) {
        bitstream_put_ui(&nal_bs, byte_buf[i], 8);
    }
    free(byte_buf);
	/* write the SEI timing data */
    bitstream_put_ui(&nal_bs, 0x01, 8);
    bitstream_put_ui(&nal_bs, pic_byte_size, 8);
    
    byte_buf = (unsigned char *)sei_pic_bs.buffer;
    for(i = 0; i < pic_byte_size; i++) {
        bitstream_put_ui(&nal_bs, byte_buf[i], 8);
    }
    free(byte_buf);

    rbsp_trailing_bits(&nal_bs);
    bitstream_end(&nal_bs);

    *sei_buffer = (unsigned char *)nal_bs.buffer; 
   
    return nal_bs.bit_offset;
}

static int build_packed_slice_buffer(unsigned char **header_buffer)
{
    bitstream bs;
    int is_idr = !!pic_param.pic_fields.bits.idr_pic_flag;
    int is_ref = !!pic_param.pic_fields.bits.reference_pic_flag;

    bitstream_start(&bs);
    nal_start_code_prefix(&bs);

    if (IS_I_SLICE(slice_param.slice_type)) {
        nal_header(&bs, NAL_REF_IDC_HIGH, is_idr ? NAL_IDR : NAL_NON_IDR);
    } else if (IS_P_SLICE(slice_param.slice_type)) {
        nal_header(&bs, NAL_REF_IDC_MEDIUM, NAL_NON_IDR);
    } else {
        assert(IS_B_SLICE(slice_param.slice_type));
        nal_header(&bs, is_ref ? NAL_REF_IDC_LOW : NAL_REF_IDC_NONE, NAL_NON_IDR);
    }

    slice_header(&bs);
    bitstream_end(&bs);

    *header_buffer = (unsigned char *)bs.buffer;
    return bs.bit_offset;
}

/*
 * Return displaying order with specified periods and encoding order
 * displaying_order: displaying order
 * frame_type: frame type 
 */
#define FRAME_P 0
#define FRAME_B 1
#define FRAME_I 2
#define FRAME_IDR 7
void encoding2display_order(
    unsigned long long encoding_order,int intra_period,
    int intra_idr_period,int ip_period,
    unsigned long long *displaying_order,
    int *frame_type)
{
    int encoding_order_gop = 0;

    if (intra_period == 1) { /* all are I/IDR frames */
        *displaying_order = encoding_order;
        if (intra_idr_period == 0)
            *frame_type = (encoding_order == 0)?FRAME_IDR:FRAME_I;
        else
            *frame_type = (encoding_order % intra_idr_period == 0)?FRAME_IDR:FRAME_I;
        return;
    }

    if (intra_period == 0)
        intra_idr_period = 0;

    /* new sequence like
     * IDR PPPPP IPPPPP
     * IDR (PBB)(PBB)(IBB)(PBB)
     */
    encoding_order_gop = (intra_idr_period == 0)? encoding_order:
        (encoding_order % (intra_idr_period + ((ip_period == 1)?0:1)));
         
    if (encoding_order_gop == 0) { /* the first frame */
        *frame_type = FRAME_IDR;
        *displaying_order = encoding_order;
    } else if (((encoding_order_gop - 1) % ip_period) != 0) { /* B frames */
	*frame_type = FRAME_B;
        *displaying_order = encoding_order - 1;
    } else if ((intra_period != 0) && /* have I frames */
               (encoding_order_gop >= 2) &&
               ((ip_period == 1 && encoding_order_gop % intra_period == 0) || /* for IDR PPPPP IPPPP */
                /* for IDR (PBB)(PBB)(IBB) */
                (ip_period >= 2 && ((encoding_order_gop - 1) / ip_period % (intra_period / ip_period)) == 0))) {
	*frame_type = FRAME_I;
	*displaying_order = encoding_order + ip_period - 1;
    } else {
	*frame_type = FRAME_P;
	*displaying_order = encoding_order + ip_period - 1;
    }
}

