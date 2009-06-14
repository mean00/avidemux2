/***************************************************************************
                       \file TIsophote.cpp
                       \brief Port of TIsophote by Tritical http://bengal.missouri.edu/~kes25c/
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
**                     TIsophote v0.9.1 for AviSynth 2.5.x
**
**   TIsophote is a simple unconstrained level-set (isophote) smoothing filter.
**
**   Copyright (C) 2004 Kevin Stone
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   This program is distributed in the hope that it will be useful,
**   but WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**   GNU General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidTisophote.h"

#include "DIA_factory.h"
static FILTER_PARAM kdintParam={4,{"iterations","tStep","type","chroma"}};


VF_DEFINE_FILTER(ADMVideoTIsophote,kdintParam,
    tisophote,
                QT_TR_NOOP("TIsophote"),
                1,
                VF_NOISE,
                QT_TR_NOOP("Port of TIsophote by tritical ."));





uint8_t ADMVideoTIsophote::configure( AVDMGenericVideoStream *instream)
{
  #define PX(x) &(_param->x)
_in=instream;

   diaMenuEntry menuField[3]={{0,QT_TR_NOOP("Simple"),NULL},
                             {1,QT_TR_NOOP("Medium"),NULL},
                             {2,QT_TR_NOOP("Slow"),NULL}
                          };


    diaElemMenu     menu1(PX(type),QT_TR_NOOP("_Type:"), 3,menuField);
    diaElemUInteger iterations(PX(iterations),QT_TR_NOOP("_Iterations:"),1,10,QT_TR_NOOP("Smaller means more deinterlacing"));
    diaElemToggle   chroma(PX(chroma),QT_TR_NOOP("_Chroma:"),QT_TR_NOOP("Process chroma."));
    diaElemFloat   step(PX(tStep),QT_TR_NOOP("_Step:"),0,1,QT_TR_NOOP("Step."));

    diaElem *elems[4]={&menu1,&iterations,&chroma,&step};

   return  diaFactoryRun(QT_TR_NOOP("KernelDeint"),4,elems);
}
uint8_t	ADMVideoTIsophote::getCoupledConf( CONFcouple **couples)
{

			*couples=new CONFcouple(4);
			CSET(iterations);
			CSET(tStep);
			CSET(type);
			CSET(chroma);

		return 1;
}
char *ADMVideoTIsophote::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" TIsoPhote by Tritical");
        
}

ADMVideoTIsophote::~ADMVideoTIsophote()
{
 if(vidCache) delete vidCache;
 if(_param) delete _param;
  if(dst1) delete dst1;
  if(dst2) delete dst2;
}


 ADMVideoTIsophote::ADMVideoTIsophote( AVDMGenericVideoStream *in,CONFcouple *couples)
{

		if(!couples)
		{
			_param=NEW(TISO_CONF);
	    		_param->iterations=4;
	    		_param->tStep=0.2;
	    		_param->type=0;
	    		_param->chroma=0;
		}
		else
		{
			_param=NEW(TISO_CONF);
			GET(iterations);
			GET(tStep);
			GET(type);
			GET(chroma);


		}
	    debug=0;
	    _in=in;

	   _uncompressed=NULL;

  	memcpy(&_info,_in->getInfo(),sizeof(_info));
	vidCache=new VideoCache(4,_in);
    dst1=new ADMImage(_info.width,_info.height);
    dst2=new ADMImage(_info.width,_info.height);

}
/**
    \fn getFrameNumberNoAlloc
    \brief Get a processed image

*/
uint8_t ADMVideoTIsophote::getFrameNumberNoAlloc(uint32_t frame,
							uint32_t *len,
							ADMImage *data,
							uint32_t *flags)
{
  // Get the current frame.
  uint32_t n=frame;

    if(frame>_info.nb_frames-1) return 0;

    ADMImage *mysrc=NULL;
    mysrc=vidCache->getImage(frame);

    //*****
    ADMImage *src = vidCache->getImage(n);

	unsigned char *srcp1 = src->GetReadPtr(PLANAR_Y);
	int src1_pitch = src->GetPitch(PLANAR_Y);
	int width = src->GetRowSize(PLANAR_Y) - 1;
	int height = src->GetHeight(PLANAR_Y) - 1;
	unsigned char *srcpp, *srcp, *srcpn;
	unsigned char *dstp1 = dst1->GetWritePtr(PLANAR_Y);
	int dst1_pitch = dst1->GetPitch(PLANAR_Y);
	unsigned char *dstp2 = dst2->GetWritePtr(PLANAR_Y);
	int dst2_pitch = dst2->GetPitch(PLANAR_Y);
	unsigned char *dstp;
	double off = 0.0000000001;
	int x, y, b, dst_pitch, src_pitch, temp;
	int Ix, Ix2, Iy, Iy2, Ixy, Ixx, Iyy;
	BitBlit(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
	BitBlit(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
	for (b=0; b<_param->iterations; ++b)
	{
		if (b&1)
		{
			srcp = dstp2 + dst2_pitch;
			src_pitch = dst2_pitch;
			srcpp = srcp - src_pitch;
			srcpn = srcp + src_pitch;
			dstp = dstp1 + dst1_pitch;
			dst_pitch = dst1_pitch;
		}
		else
		{
			srcp = dstp1 + dst1_pitch;
			src_pitch = dst1_pitch;
			srcpp = srcp - src_pitch;
			srcpn = srcp + src_pitch;
			dstp = dstp2 + dst2_pitch;
			dst_pitch = dst2_pitch;
		}
		if (_param->type == 0)
		{
			for (y=1; y<height; ++y)
			{
				for (x=1; x<width; ++x)
				{
					Ix = srcp[x+1] - srcp[x-1];
					Ix2 = Ix*Ix;
					Iy = srcpn[x] - srcpp[x];
					Iy2 = Iy*Iy;
					Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
					Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
					Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
					temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*_param->tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
					if (temp > 255) temp = 255;
					else if (temp < 0) temp = 0;
					dstp[x] = temp;
				}
				srcpp += src_pitch;
				srcp += src_pitch;
				srcpn += src_pitch;
				dstp += dst_pitch;
			}
		}
		else if (_param->type == 1)
		{
			for (y=1; y<height; ++y)
			{
				for (x=1; x<width; ++x)
				{
					Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
					Ix2 = Ix*Ix;
					Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
					Iy2 = Iy*Iy;
					Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
					Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
					Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
					temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*_param->tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
					if (temp > 255) temp = 255;
					else if (temp < 0) temp = 0;
					dstp[x] = temp;
				}
				srcpp += src_pitch;
				srcp += src_pitch;
				srcpn += src_pitch;
				dstp += dst_pitch;
			}
		}
		else
		{
			for (y=1; y<height; ++y)
			{
				for (x=1; x<width; ++x)
				{
					Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
					Ix2 = Ix*Ix;
					Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
					Iy2 = Iy*Iy;
					Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
					Ixx = srcpp[x+1] + srcp[x+1] + srcp[x+1] + srcpn[x+1] + srcpp[x-1] + srcp[x-1] +
						srcp[x-1] + srcpn[x-1] - (srcpp[x]<<1) - (srcp[x]<<2) - (srcpn[x]<<1);
					Iyy = srcpp[x-1] + srcpp[x] + srcpp[x] + srcpp[x+1] + srcpn[x-1] + srcpn[x] +
						srcpn[x] + srcpn[x+1] - (srcp[x-1]<<1) - (srcp[x]<<2) - (srcp[x+1]<<1);
					temp = srcp[x] + ((int)((((Ix2*Iyy - 4*Ix*Iy*Ixy + Iy2*Ixx)*_param->tStep) / (((Ix2 + Iy2) << 3) + off)) + 0.5f));
					if (temp > 255) temp = 255;
					else if (temp < 0) temp = 0;
					dstp[x] = temp;
				}
				srcpp += src_pitch;
				srcp += src_pitch;
				srcpn += src_pitch;
				dstp += dst_pitch;
			}
		}
	}
	srcp1 = src->GetReadPtr(PLANAR_U);
	src1_pitch = src->GetPitch(PLANAR_U);
	width = src->GetRowSize(PLANAR_U) - 1;
	height = src->GetHeight(PLANAR_U) - 1;
	dstp1 = dst1->GetWritePtr(PLANAR_U);
	dst1_pitch = dst1->GetPitch(PLANAR_U);
	dstp2 = dst2->GetWritePtr(PLANAR_U);
	dst2_pitch = dst2->GetPitch(PLANAR_U);
	if (_param->chroma)
	{
		BitBlit(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
		BitBlit(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
		for (b=0; b<_param->iterations; ++b)
		{
			if (b&1)
			{
				srcp = dstp2 + dst2_pitch;
				src_pitch = dst2_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp1 + dst1_pitch;
				dst_pitch = dst1_pitch;
			}
			else
			{
				srcp = dstp1 + dst1_pitch;
				src_pitch = dst1_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp2 + dst2_pitch;
				dst_pitch = dst2_pitch;
			}
			if (_param->type == 0)
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcp[x+1] - srcp[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x] - srcpp[x];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*_param->tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else if (_param->type == 1)
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*_param->tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcpp[x+1] + srcp[x+1] + srcp[x+1] + srcpn[x+1] + srcpp[x-1] + srcp[x-1] +
							srcp[x-1] + srcpn[x-1] - (srcpp[x]<<1) - (srcp[x]<<2) - (srcpn[x]<<1);
						Iyy = srcpp[x-1] + srcpp[x] + srcpp[x] + srcpp[x+1] + srcpn[x-1] + srcpn[x] +
							srcpn[x] + srcpn[x+1] - (srcp[x-1]<<1) - (srcp[x]<<2) - (srcp[x+1]<<1);
						temp = srcp[x] + ((int)((((Ix2*Iyy - 4*Ix*Iy*Ixy + Iy2*Ixx)*_param->tStep) / (((Ix2 + Iy2) << 3) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
		}
		srcp1 = src->GetReadPtr(PLANAR_V);
		dstp1 = dst1->GetWritePtr(PLANAR_V);
		dstp2 = dst2->GetWritePtr(PLANAR_V);
		BitBlit(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
		BitBlit(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
		for (b=0; b<_param->iterations; ++b)
		{
			if (b&1)
			{
				srcp = dstp2 + dst2_pitch;
				src_pitch = dst2_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp1 + dst1_pitch;
				dst_pitch = dst1_pitch;
			}
			else
			{
				srcp = dstp1 + dst1_pitch;
				src_pitch = dst1_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp2 + dst2_pitch;
				dst_pitch = dst2_pitch;
			}
			if (_param->type == 0)
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcp[x+1] - srcp[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x] - srcpp[x];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*_param->tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else if (_param->type == 1)
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*_param->tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcpp[x+1] + srcp[x+1] + srcp[x+1] + srcpn[x+1] + srcpp[x-1] + srcp[x-1] +
							srcp[x-1] + srcpn[x-1] - (srcpp[x]<<1) - (srcp[x]<<2) - (srcpn[x]<<1);
						Iyy = srcpp[x-1] + srcpp[x] + srcpp[x] + srcpp[x+1] + srcpn[x-1] + srcpn[x] +
							srcpn[x] + srcpn[x+1] - (srcp[x-1]<<1) - (srcp[x]<<2) - (srcp[x+1]<<1);
						temp = srcp[x] + ((int)((((Ix2*Iyy - 4*Ix*Iy*Ixy + Iy2*Ixx)*_param->tStep) / (((Ix2 + Iy2) << 3) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
		}
	}
	else
	{
		if (_param->iterations&1) BitBlit(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
		else BitBlit(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
		srcp1 = src->GetReadPtr(PLANAR_V);
		dstp1 = dst1->GetWritePtr(PLANAR_V);
		dstp2 = dst2->GetWritePtr(PLANAR_V);
		if (_param->iterations&1) BitBlit(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
		else BitBlit(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
	}
	if (_param->iterations&1) data->duplicate(dst2);
        else data->duplicate(dst1);

    //*****
    vidCache->unlockAll();
    return 1;

}

//***
