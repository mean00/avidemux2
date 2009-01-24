#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "global.h"
#include "motionsearch.h"
#include "predict_ref.h"
#include "transfrm_ref.h"
#include "quantize_ref.h"
#include "format_codes.h"
#include "mpegconsts.h"
#include "fastintfns.h"
#include "mpeg2parm.h"
/*
	Put 'Good' default value

*/
void mpeg2parm::setDefault(void)
{
 format = MPEG_FORMAT_MPEG1;
 searchrad  = 16;
 mpeg       = 1;
 fieldenc=  	 -1; /* 0: progressive, 
                                     1 = frame pictures, 
                                     interlace frames with field
                                     MC and DCT in picture 
                                     2 = field pictures
                                  */
 _44_red	= 2;
 _22_red	= 3;	
 hf_q_boost = 0.0;
 act_boost = 0.0;
 boost_var_ceil = 10*10;
 min_GOP_size = -1;
 max_GOP_size = -1;
  Bgrp_size=3;
 
 num_cpus = 1;
 svcd_scan_data = -1;
 force_interlacing = Y4M_UNKNOWN;
 hack_svcd_hds_bug = 1;
 mpeg2_dc_prec = 1;

}



