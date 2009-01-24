//
// C++ Interface: ADM_vidDecDec_param
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
typedef struct DECIMATE_PARAM
{
	uint32_t    cycle; // Integer 2..40
        uint32_t    mode;  // 0..3
        uint32_t    quality; //0..4
	double      threshold;
        double      threshold2;
};
