/*
	This structure is all parameters passed from application to mpeg2enc
	In case of mpeg3enc it is filled by parsing the command line


*/
#ifndef MPEG2PARAM_H
#define MPEG2PARAM_H
class mpeg2parm
{
public:

			void setDefault( void );

 int 			format ;
 int 			bitrate    ;
 int 			nonvid_bitrate ;
 int 			quant      ;
 int 			searchrad ;
 int 			mpeg     ;
 unsigned int 		aspect_ratio ;
 unsigned int 		frame_rate  ;
 int 			fieldenc;/* 0: progressive, 
                                     1 = frame pictures, 
                                     interlace frames with field
                                     MC and DCT in picture 
                                     2 = field pictures
                                  */
 int 			norm    ;  /* 'n': NTSC, 'p': PAL, 's': SECAM, else unspecified */
 int 			_44_red	;
 int 			_22_red	;	
 int 			hf_quant ;
 double 		hf_q_boost ;
 double 		act_boost ;
 double 		boost_var_ceil ;
 int 			video_buffer_size ;
 int 			seq_length_limit ;
 int 			min_GOP_size;
 int 			max_GOP_size;
 int 			closed_GOPs;
 int 			preserve_B ;
 int 			Bgrp_size;
 int 			num_cpus ;
 int 			_32_pulldown ;
 int 			svcd_scan_data ;
 int 			seq_hdr_every_gop;
 int 			seq_end_every_gop;
 int 			still_size ;
 int 			pad_stills_to_vbv_buffer_size ;
 int 			vbv_buffer_still_size ;
 int 			force_interlacing ;
 unsigned int 		input_interlacing;
 int 			hack_svcd_hds_bug ;
 int 			hack_altscan_bug ;
 int 			mpeg2_dc_prec ;
 int 			ignore_constraints ;

/* Input Stream parameter values that have to be further processed to
   set encoding options */

 uint16_t 		custom_intra_quantizer_matrix[64];
 uint16_t 		custom_nonintra_quantizer_matrix[64];
 /* MEANX */
 int			noPadding;
 /* /MEANX */

};
#endif
