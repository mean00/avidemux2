
/***************************************************************************
                          ADM_vidDecTelecide  -  description
                             -------------------
    
    email                : fixounet@free.fr

    Port of Donal Graft Telecide which is (c) Donald Graft
    http://www.neuron2.net
    http://puschpull.org/avisynth/decomb_reference_manual.html

 ***************************************************************************/
/*
	Telecide plugin for Avisynth -- recovers original progressive
	frames from telecined streams. The filter operates by matching
	fields and automatically adapts to phase/pattern changes.

	Copyright (C) 2003 Donald A. Graft

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "ADM_default.h"
#include "Telecide.h"
#include "DIA_factory.h"
#include "telec_desc.cpp"
#include "ADM_vidMisc.h"
#include "Telecide_debug.h"
// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   Telecide,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "telecide",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("telecide","Decomb telecide"),            // Display name
                        QT_TRANSLATE_NOOP("telecide","Donald Graft Telecide. Replace ivtc pattern by progressive frames. Video stays at 30 fps.") // Description
                    );

/**
    \fn configure
*/

/**
    \fn Ctor
*/       
Telecide::Telecide(	ADM_coreVideoFilter *in,CONFcouple *couples)      : 
        ADM_coreVideoFilterCached(16,in,couples)
{

        int i;		
        int count;
        char *d, *dsaved;
        unsigned int *p, *x;
        teleCide *_param=&configuration;		
		

		if(!couples || !ADM_paramLoad(couples,teleCide_param,&configuration))
        {
                 
			 	_param->order = 1; 		// 0 Field ok, 1 field reverted 0 BFF/1 TFF
				_param->back = NO_BACK; // 0 Never, 1 when bad, 2 always tried MUST Have post !=0
				_param->chroma = false;
				_param->guide = GUIDE_32;// 0 / NONE - 1 GUIDE_32/ivtc-2 GUIDE 22/PAL-3 PAL/NTSC
				_param->gthresh = 10.0;
				_param->post = POST_METRICS;
				_param->vthresh = 50.0;
				_param->bthresh = 50.0;
				_param->dthresh = 7.0;
				_param->blend = false; // Interpolate is default
				_param->nt = 10;	// Noise tolerance
				_param->y0 = 0;		// Zone to try (avoid subs)
				_param->y1 = 0;
				_param->hints = true;
				_param->show = false;
				_param->debug = false; 

		}
				 
				
		tff = (_param->order == 0 ? false : true);	

		_param->back_saved = _param->back;

		// Set up pattern guidance.
		cache = (struct CACHE_ENTRY *) ADM_alloc(CACHE_SIZE * sizeof(struct CACHE_ENTRY));
		CachePurge();

		if (_param->guide == GUIDE_32)
		{
			// 24fps to 30 fps telecine.
			cycle = 5;
		}
		if (_param->guide == GUIDE_22)
		{
			// PAL guidance (expect the current match to be continued).
			cycle = 2;
		}
		else if (_param->guide == GUIDE_32322)
		{
			// 25fps to 30 fps telecine.
			cycle = 6;
		}

		// Get needed dynamic storage.
		vmetric = 0;
		_param->vthresh_saved = _param->vthresh;
		xblocks = (info.width+BLKSIZE-1) / BLKSIZE;
		yblocks = (info.height+BLKSIZE-1) / BLKSIZE;
#ifdef WINDOWED_MATCH
		matchp = (unsigned int *) ADM_alloc(xblocks * yblocks * sizeof(unsigned int));
		
		matchc = (unsigned int *) ADM_alloc(xblocks * yblocks * sizeof(unsigned int));
		
#endif
		sump = (unsigned int *) ADM_alloc(xblocks * yblocks * sizeof(unsigned int));
		
		sumc = (unsigned int *) ADM_alloc(xblocks * yblocks * sizeof(unsigned int));
}
/**
    \fn dtor
*/
Telecide::~Telecide()
{
		unsigned int *p;

		if (cache != NULL) ADM_dealloc(cache);
#ifdef WINDOWED_MATCH
		if (matchp != NULL) ADM_dealloc(matchp);
		if (matchc != NULL) ADM_dealloc(matchc);
                matchp=NULL;
                matchc=NULL;

#endif
		if (sump != NULL) ADM_dealloc(sump);
		if (sumc != NULL) ADM_dealloc(sumc);
                cache=NULL;
                sump=NULL;
                sumc=NULL;
}
/**
    \fn getCoupledConf
*/ 
bool         Telecide::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, teleCide_param,&configuration);
}

void Telecide::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, teleCide_param, &configuration);
}

/**
    \fn goToTime
    \brief Need to reset internals in case of seek
*/
bool                Telecide::goToTime(uint64_t usSeek)
{
    aprintf("Go to time %s\n",ADM_us2plain(usSeek));
    CachePurge();
    return ADM_coreVideoFilterCached::goToTime(usSeek);
}
// EOF
