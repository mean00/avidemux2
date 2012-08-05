/***************************************************************************
                          ADM_vidKernelDeint  -  description
                             -------------------
 			Port of another D Graft deinterlacer
			http://neuron2.net/kerneldeint/kerneldeint.html
     ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/*
	KernelDeint() plugin for Avisynth.

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
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidKernelDeint.h"
#include "kdeint_desc.cpp"

#include "DIA_factory.h"
// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   kernelDeint,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "kerndelDeint",            // internal name (must be uniq!)
                        "Kernel Deint.",            // Display name
                        "Port of Donald Graft Kernel Deinterlacer." // Description
                    );

extern int PutHintingData(uint8_t *video, unsigned int hint);
extern int GetHintingData(uint8_t *video, unsigned int *hint);

#define PROGRESSIVE  0x00000001



/**
    \fn configure
*/
bool kernelDeint::configure( void)
{
  #define PX(x) &(param.x)

   diaMenuEntry menuField[2]={{1,QT_TR_NOOP("Top"),NULL},
                             {0,QT_TR_NOOP("Bottom"),NULL}
                          };
  
    
    diaElemMenu     menu1(PX(order),QT_TR_NOOP("_Field order:"), 2,menuField);
    diaElemUInteger threshold(PX(threshold),QT_TR_NOOP("_Threshold:"),0,100,QT_TR_NOOP("Smaller means more deinterlacing"));
    diaElemToggle   sharp(PX(sharp),QT_TR_NOOP("_Sharp"),QT_TR_NOOP("_Sharper engine:"));
    diaElemToggle   twoway(PX(twoway),QT_TR_NOOP("T_woway"),QT_TR_NOOP("Extrapolate better (better not to use it)"));
    diaElemToggle   map(PX(map),QT_TR_NOOP("_Map"),QT_TR_NOOP("Show interlaced areas (for test!)"));
    
    diaElem *elems[5]={&menu1,&threshold,&sharp,&twoway,&map};
  
   return  diaFactoryRun(QT_TR_NOOP("KernelDeint"),5,elems);
}
/**
    \fn getCoupledConf
*/
bool         kernelDeint::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, kdeint_param,&param);
}

void kernelDeint::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, kdeint_param, &param);
}

/**
    
*/
const char *kernelDeint::getConfiguration(void)
{
 	static char conf[100];
    conf[0]=0;
    snprintf(conf,100,"kdeint Borders : order:%"PRIu32" threshold:%"PRIu32" sharp:%"PRIu32" twoway:%"PRIu32" map:%"PRIu32"\n",
                param.order,param.threshold,param.sharp,param.twoway,param.map);
    return conf;
}
/**
    \fn dtor
*/
kernelDeint::~kernelDeint()
{
    
 	
}

/**
    \fn ctor
*/
 kernelDeint::kernelDeint(ADM_coreVideoFilter *previous,CONFcouple *setup)
            :  ADM_coreVideoFilterCached(4,previous,setup)
{
    if(!setup || !ADM_paramLoad(setup,kdeint_param,&param))
    {
        // Default value
        param.order=1;// Bff=0 / 1=tff
        param.threshold=10;
        param.sharp=0;
        param.twoway=0;
        param.map=0;
    }  	  			
    debug=false;   
	
}
/**
    \fn getNextFrame
*/
bool kernelDeint::getNextFrame(uint32_t *fn,ADMImage *image)
{

    uint32_t page=info.width*info.height;
    ADMImage *src=NULL, *prv=NULL;

    uint32_t		order, threshold;
	uint8_t			sharp, twoway, map;
	
	order=param.order;
	threshold=param.threshold;
	sharp=param.sharp;
	twoway=param.twoway;
	map=param.map;		
    *fn=nextFrame;            
    uint32_t frame_prev=nextFrame;
    if(frame_prev) frame_prev--;
    
        src=vidCache->getImage(nextFrame);
        if(!src) 
        {
            ADM_warning("kerneldeint:Cannot get frame\n");
            vidCache->unlockAll();
            nextFrame++;
            return false;
        }
        prv=vidCache->getImage(frame_prev);
        if(!prv)
        {
            vidCache->unlockAll();
            image->duplicate(src);
            image->copyInfo(src);
            nextFrame++;
            return true;
        }
/** From here it is mostly untouched .. */
    const unsigned char *srcp, *prvp, *prvpp, *prvpn, *prvppp, *prvpnn, *prvp4p, *prvp4n;
	const unsigned char *srcp_saved;
	const unsigned char *srcpp, *srcppp, *srcpn, *srcpnn, *srcp3p, *srcp3n, *srcp4p, *srcp4n;
    unsigned char *dstp;
	unsigned char *dstp_saved;
 
	ADM_PLANE plane;
	int src_pitch;
    int dst_pitch;
    int w;
    int h;
	int x, y, z;
	int val, hi, lo;
	double valf;
	unsigned int hint;
	

	for ( z = 0; z <  3; z++)
	{
		if (z == 0) plane = PLANAR_Y;
		else if (z == 1) plane = PLANAR_U;
		else plane = PLANAR_V;

		srcp = srcp_saved = src->GetReadPtr(plane);
		if (plane == PLANAR_Y && (GetHintingData((unsigned char *) srcp, &hint) == false) && (hint & PROGRESSIVE))
		{
			if (debug ==true)
			{
				ADM_info( "KernelDeint: frame %d: progressive\n", nextFrame); 
				
			}
            image->duplicate(src);
			image->copyInfo(src);
			vidCache->unlockAll();
            nextFrame++;
			return true;
		}
		else
		{
			if (debug == true)
			{
				ADM_info( "KernelDeint: frame %d: interlaced\n", nextFrame); 
			}
		}
		src_pitch = src->GetPitch(plane);
		dstp = dstp_saved = image->GetWritePtr(plane);
		dst_pitch = image->GetPitch(plane);
		w = image->GetRowSize(plane);
		h = image->GetHeight(plane);
		srcp = srcp_saved  + (1-order) * src_pitch;
		dstp = dstp_saved  + (1-order) * dst_pitch;
		for (y = 0; y < h; y+=2)
		{
			memcpy(dstp, srcp, w);
			srcp += 2*src_pitch;
			dstp += 2*dst_pitch;
		}

		// Copy through the lines that will be missed below.
		memcpy(dstp_saved + order*dst_pitch, srcp_saved + (1-order)*src_pitch, w);
		memcpy(dstp_saved + (2+order)*dst_pitch, srcp_saved + (3-order)*src_pitch, w);
		memcpy(dstp_saved + (h-2+order)*dst_pitch, srcp_saved + (h-1-order)*src_pitch, w);
		memcpy(dstp_saved + (h-4+order)*dst_pitch, srcp_saved + (h-3-order)*src_pitch, w);
		/* For the other field choose adaptively between using the previous field
		   or the interpolant from the current field. */
		prvp = prv->GetReadPtr(plane) + 5*src_pitch - (1-order)*src_pitch;
		prvpp = prvp - src_pitch;
		prvppp = prvp - 2*src_pitch;
		prvp4p = prvp - 4*src_pitch;
		prvpn = prvp + src_pitch;
		prvpnn = prvp + 2*src_pitch;
		prvp4n = prvp + 4*src_pitch;
		srcp = srcp_saved + 5*src_pitch - (1-order)*src_pitch;
		srcpp = srcp - src_pitch;
		srcppp = srcp - 2*src_pitch;
		srcp3p = srcp - 3*src_pitch;
		srcp4p = srcp - 4*src_pitch;
		srcpn = srcp + src_pitch;
		srcpnn = srcp + 2*src_pitch;
		srcp3n = srcp + 3*src_pitch;
		srcp4n = srcp + 4*src_pitch;
		dstp =  dstp_saved  + 5*dst_pitch - (1-order)*dst_pitch;
		for (y = 5 - (1-order); y <= h - 5 - (1-order); y+=2)
		{
			for (x = 0; x < w; x++)
			{
				if ((threshold == 0) || (!nextFrame) ||
					(abs((int)prvp[x] - (int)srcp[x]) > threshold) ||
					(abs((int)prvpp[x] - (int)srcpp[x]) > threshold) ||
					(abs((int)prvpn[x] - (int)srcpn[x]) > threshold))
				{
					if (map == true)
					{
						int g = x & ~3;
						if (0) // RGB
						{
							dstp[g++] = 255;
							dstp[g++] = 255;
							dstp[g++] = 255;
							dstp[g] = 255;
							x = g;
						}
						else if (0) // YUY2
						{
							dstp[g++] = 235;
							dstp[g++] = 128;
							dstp[g++] = 235;
							dstp[g] = 128;
							x = g;
						}
						else
						{
							if (plane == PLANAR_Y) dstp[x] = 235;
							else dstp[x] = 128;
						}
					}
					else
					{
						if (0)
						{
							hi = 255;
							lo = 0;
						}
						else if (1)
						{
							hi = (plane == PLANAR_Y) ? 235 : 240;
							lo = 16;
						}
						else if (0)
						{
							hi = (x & 1) ? 240 : 235;
							lo = 16;
						}
						//else env->ThrowError("KernelDeint: Unknown color space");
						if (sharp == true)
						{
							if (twoway == true)
								valf = + 0.526*((int)srcpp[x] + (int)srcpn[x])
								   + 0.170*((int)srcp[x] + (int)prvp[x])
								   - 0.116*((int)srcppp[x] + (int)srcpnn[x] + (int)prvppp[x] + (int)prvpnn[x])
					 			   - 0.026*((int)srcp3p[x] + (int)srcp3n[x])
								   + 0.031*((int)srcp4p[x] + (int)srcp4n[x] + (int)prvp4p[x] + (int)prvp4n[x]);
							else
								valf = + 0.526*((int)srcpp[x] + (int)srcpn[x])
								   + 0.170*((int)prvp[x])
								   - 0.116*((int)prvppp[x] + (int)prvpnn[x])
					 			   - 0.026*((int)srcp3p[x] + (int)srcp3n[x])
								   + 0.031*((int)prvp4p[x] + (int)prvp4n[x]);
							if (valf > hi) valf = hi;
							else if (valf < lo) valf = lo;
							dstp[x] = (int) valf;
						}
						else
						{
							if (twoway == true)
								val = (8*((int)srcpp[x] + (int)srcpn[x]) + 2*((int)srcp[x] + (int)prvp[x]) -
									(int)(srcppp[x]) - (int)(srcpnn[x]) -
									(int)(prvppp[x]) - (int)(prvpnn[x])) >> 4;
							else
								val = (8*((int)srcpp[x] + (int)srcpn[x]) + 2*((int)prvp[x]) -
									(int)(prvppp[x]) - (int)(prvpnn[x])) >> 4;
							if (val > hi) val = hi;
							else if (val < lo) val = lo;
							dstp[x] = (int) val;
						}
					}
				}
				else
				{
					dstp[x] = srcp[x];
				}
			}
			prvp  += 2*src_pitch;
			prvpp  += 2*src_pitch;
			prvppp  += 2*src_pitch;
			prvpn  += 2*src_pitch;
			prvpnn  += 2*src_pitch;
			prvp4p  += 2*src_pitch;
			prvp4n  += 2*src_pitch;
			srcp  += 2*src_pitch;
			srcpp += 2*src_pitch;
			srcppp += 2*src_pitch;
			srcp3p += 2*src_pitch;
			srcp4p += 2*src_pitch;
			srcpn += 2*src_pitch;
			srcpnn += 2*src_pitch;
			srcp3n += 2*src_pitch;
			srcp4n += 2*src_pitch;
			dstp  += 2*dst_pitch;
		}
	}
    vidCache->unlockAll();
    image->copyInfo(src);
    nextFrame++;
    return true;
}
// EOF
