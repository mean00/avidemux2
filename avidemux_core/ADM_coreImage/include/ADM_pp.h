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

#include "ADM_coreImage6_export.h"

/**
    \class ADM_PP
    \brief wrapper around libavcodec postprocessing
*/
class ADM_COREIMAGE6_EXPORT ADM_PP
{
protected:
    void    			*ppContext; // pp_context_t
	void    			*ppMode;    // pp_mode_t
    bool                cleanup(void);
public:
	
 	uint32_t			postProcType;
	uint32_t			postProcStrength;
    bool                swapuv;
	uint32_t			forcedQuant;
	uint32_t			 w,h;

public:
                ADM_PP(uint32_t width, uint32_t h);
                ~ADM_PP();
    bool        update(void);
    bool        process(class ADMImage *src, class ADMImage *dest);

};
#define FORCE_QUANT			0x200000
//	PostProc : 1 Horiz deblock
//             2 Verti deblock
//             4 Dering
// strength between 0 and 5
 

#endif
