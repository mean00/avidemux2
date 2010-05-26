//
// C++ Implementation: ADM_pp
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ADMPP
#define ADMPP
typedef struct ADM_PP
{
	void    			*ppContext; // pp_context_t
	void    			*ppMode;    // pp_mode_t
 	uint32_t			postProcType;
	uint32_t			postProcStrength;
        uint32_t                        swapuv;
	uint32_t			forcedQuant;
	uint32_t			w,h;

}ADM_PP;
#define FORCE_QUANT			0x200000
//	PostProc : 1 Horiz deblock
//             2 Verti deblock
//             4 Dering
// strength between 0 and 5
 

void updatePostProc(ADM_PP *pp );
void deletePostProc(ADM_PP *pp );
void initPostProc(ADM_PP *pp,uint32_t w, uint32_t h);
#endif
