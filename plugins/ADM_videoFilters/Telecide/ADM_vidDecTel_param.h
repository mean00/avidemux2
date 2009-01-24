//
// C++ Interface: ADM_vidDecTel_param
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//


typedef struct 
{
	uint32_t 	order;           /* Field Order 0 BFF, 1TFF */
	uint32_t 	back;            // Try backward field :0 Never, 1 when bad, 2 always tried MUST Have post !=0
	uint32_t	back_saved;
	uint32_t 	guide;          // (strategy) See GUIDE_xx 0 / NONE - 1 GUIDE_32/ivtc-2 GUIDE 22/PAL-3 PAL/NTSC
	double	 	gthresh;        // noise threhold
	uint32_t 	post;            // See POST_XX
	uint32_t 	chroma;          // True/false Use chroma to decide
	double 		vthresh;         // 
	double		vthresh_saved;
	double 		bthresh;         //
	double 		dthresh;         // Direct Threshold
	uint32_t 	blend;           // Blend or interpolate (blend=1/interpolate =0;
	uint32_t 	nt;
	uint32_t 	y0;
	uint32_t 	y1;
	uint32_t 	hints;           // ignore
	uint32_t 	show;            // Toggle
	uint32_t 	debug;           // Toggle
}TelecideParam;
