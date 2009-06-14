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
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidKernelDeint.h"

#include "DIA_factory.h"

static FILTER_PARAM kdintParam={5,{"order","threshold","sharp","twoway","map"}};


VF_DEFINE_FILTER(ADMVideoKernelDeint,kdintParam,
    kerneldeint,
                QT_TR_NOOP("KernelDeint"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Kernel deinterlacer by Donald Graft."));
static int PutHintingData(uint8_t *video, unsigned int hint);
static int GetHintingData(uint8_t *video, unsigned int *hint);

// extern uint8_t DIA_kerneldeint(uint32_t *order, uint32_t *threshold, uint32_t *sharp, 
// 		uint32_t *twoway, uint32_t *map);

#define PROGRESSIVE  0x00000001





uint8_t ADMVideoKernelDeint::configure( AVDMGenericVideoStream *instream)
{
  #define PX(x) &(_param->x)
_in=instream;

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
uint8_t	ADMVideoKernelDeint::getCoupledConf( CONFcouple **couples)
{

			*couples=new CONFcouple(5);

			CSET(order);
			CSET(threshold);
			CSET(sharp);
			CSET(twoway);
			CSET(map);	

		return 1;	
}
char *ADMVideoKernelDeint::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" D Graft Kernel Deint");
        
}

ADMVideoKernelDeint::~ADMVideoKernelDeint()
{
 if(vidCache) delete vidCache;
 	
}


 ADMVideoKernelDeint::ADMVideoKernelDeint( AVDMGenericVideoStream *in,CONFcouple *couples)
{

		if(!couples)
		{
			_param=NEW(KERNEL_CONF);
	    		_param->order=1; // Bff=0 / 1=tff
	    		_param->threshold=10;
	    		_param->sharp=0;
	    		_param->twoway=0;
	    		_param->map=0;
		}
		else
		{
			_param=NEW(KERNEL_CONF);
			GET(order);
			GET(threshold);
			GET(sharp);
			GET(twoway);
			GET(map);
						
		}
	    debug=0;   
	    _in=in;
	    
	   _uncompressed=NULL;

  	memcpy(&_info,_in->getInfo(),sizeof(_info));
	vidCache=new VideoCache(4,_in);


}
uint8_t ADMVideoKernelDeint::getFrameNumberNoAlloc(uint32_t frame,
							uint32_t *len,
							ADMImage *data,
							uint32_t *flags)
{
		
		uint32_t frame_prev;
		uint32_t page=_info.width*_info.height;
		ADMImage *mysrc=NULL, *myprev=NULL;
		
		if(frame>_info.nb_frames-1) return 0;


		
		frame_prev=frame;
		if(frame_prev) frame_prev--;
		
		
		
			mysrc=vidCache->getImage(frame);
			myprev=vidCache->getImage(frame_prev);
			ADM_assert(mysrc);
			ADM_assert(myprev);
		// Now go to kernel deint code
			
    const uint8_t *srcp, *prvp,*prvp_saved, *prvpp, *prvpn, *prvppp, *prvpnn, *prvp4p, *prvp4n;
	const uint8_t *srcp_saved;
	const uint8_t *srcpp, *srcppp, *srcpn, *srcpnn, *srcp3p, *srcp3n, *srcp4p, *srcp4n;
    uint8_t *dstp;
	uint8_t *dstp_saved;
 
	int plane;
	int src_pitch;
    int dst_pitch;
    int w;
    int h;
	int x, y, z;
	int val, hi, lo;
	double valf;
	unsigned int hint;
	char buf[80];
	
	uint32_t pitch;
	uint32_t offset;
	
	
	uint32_t		order, threshold;
	uint8_t			sharp, twoway, map;
	
	order=_param->order;
	threshold=_param->threshold;
	sharp=_param->sharp;
	twoway=_param->twoway;
	map=_param->map;
	
	
	for (z = 0; z < 3; z++)
	{
		
		pitch=_info.width;
		switch(z)
		{
			case 0:		offset=0;
					srcp=srcp_saved= YPLANE(mysrc);
					dstp = dstp_saved=YPLANE(data);
					prvp_saved=prvp=YPLANE(myprev);
					break;
			case 1:		offset=page;
					pitch>>=1;
					srcp=srcp_saved=UPLANE(mysrc);
					dstp = dstp_saved=UPLANE(data);
					prvp_saved=prvp=UPLANE(myprev);
					break;
			case 2:		offset=((page*5)>>2);
					pitch>>=1;
					srcp=srcp_saved=VPLANE(mysrc);
					dstp = dstp_saved=VPLANE(data);
					prvp_saved=prvp=VPLANE(myprev);
					break;
		
		}
		
		if (z==0 && (GetHintingData((uint8_t *) srcp, &hint) == false) && (hint & PROGRESSIVE))
		{
			if (debug ==true)
			{
				printf( "KernelDeint: frame %d: progressive\n", frame); 
				
			}
			memcpy(YPLANE(data),YPLANE(mysrc),page);
			memcpy(UPLANE(data),UPLANE(mysrc),page>>2);
			memcpy(VPLANE(data),VPLANE(mysrc),page>>2);
			vidCache->unlockAll();
			data->copyInfo(mysrc);
			return 1;
		}
		else
		{
			if (debug == true)
			{
				printf( "KernelDeint: frame %d: interkaced\n", frame); 
			}
		}
		
		src_pitch = pitch;	
		dst_pitch = pitch;
		
		w = pitch; //dst->GetRowSize(plane);
		h=_info.height;
		if(z) h>>=1;  //h = dst->GetHeight(plane);
		
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
		//prvp = prv->GetReadPtr(plane) + 5*src_pitch - (1-order)*src_pitch;
		prvp = prvp_saved + 5*src_pitch - (1-order)*src_pitch;
		
		
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
				if ((threshold == 0) || (frame == 0) ||
					(abs((int)prvp[x] - (int)srcp[x]) > threshold) ||
					(abs((int)prvpp[x] - (int)srcpp[x]) > threshold) ||
					(abs((int)prvpn[x] - (int)srcpn[x]) > threshold))
				{
					if (map == true)
					{
						int g = x & ~3;
						
						{
							if (z == 0) dstp[x] = 235;
							else dstp[x] = 128;
						}
					}
					else
					{
						
						{
							hi = (z == 0) ? 235 : 240;
							lo = 16;
						}
						
						if (sharp == true)
						{
							if (twoway == true)
								valf = + 0.526*((int)srcpp[x] +
								 (int)srcpn[x])
								   + 0.170*((int)srcp[x] + (int)prvp[x])
								   - 0.116*((int)srcppp[x] +
								    (int)srcpnn[x] + (int)prvppp[x] +
								    (int)prvpnn[x])
					 			   - 0.026*((int)srcp3p[x] +
								    (int)srcp3n[x])
								   + 0.031*((int)srcp4p[x] +
								    (int)srcp4n[x] + (int)prvp4p[x] +
								    (int)prvp4n[x]);
							else
								valf = + 0.526*((int)srcpp[x] +
								 (int)srcpn[x])
								   + 0.170*((int)prvp[x])
								   - 0.116*((int)prvppp[x] +
								    (int)prvpnn[x])
					 			   - 0.026*((int)srcp3p[x] +
								    (int)srcp3n[x])
								   + 0.031*((int)prvp4p[x] +
								    (int)prvp4p[x]);
							if (valf > hi) valf = hi;
							else if (valf < lo) valf = lo;
							dstp[x] = (int) valf;
						}
						else
						{
							if (twoway == true)
								val = (8*((int)srcpp[x] + (int)srcpn[x]) +
								 2*((int)srcp[x] + (int)prvp[x]) -
								 			 
									(int)(srcppp[x]) -
									 (int)(srcpnn[x]) -
									(int)(prvppp[x]) -
									 (int)(prvpnn[x])) >> 4;
							else
								val = (8*((int)srcpp[x] + (int)srcpn[x]) +
								 2*((int)prvp[x]) -
									(int)(prvppp[x]) -
									 (int)(prvpnn[x])) >> 4;
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
	data->copyInfo(mysrc);
	vidCache->unlockAll();
	return 1;
}



#define MAGIC_NUMBER (0xdeadbeef)

int PutHintingData(uint8_t *video, unsigned int hint)
{
	uint8_t *p;
	unsigned int i, magic_number = MAGIC_NUMBER;
	int error = false;

	p = video;
	for (i = 0; i < 32; i++)
	{
		*p &= ~1; 
		*p++ |= ((magic_number & (1 << i)) >> i);
	}
	for (i = 0; i < 32; i++)
	{
		*p &= ~1;
		*p++ |= ((hint & (1 << i)) >> i);
	}
	return error;
}

int GetHintingData(uint8_t *video, unsigned int *hint)
{
	uint8_t *p;
	unsigned int i, magic_number = 0;
	int error = false;

	p = video;
	for (i = 0; i < 32; i++)
	{
		magic_number |= ((*p++ & 1) << i);
	}
	if (magic_number != MAGIC_NUMBER)
	{
		error = true;
	}
	else
	{
		*hint = 0;
		for (i = 0; i < 32; i++)
		{
			*hint |= ((*p++ & 1) << i);
		}
	}
	return error;
}
