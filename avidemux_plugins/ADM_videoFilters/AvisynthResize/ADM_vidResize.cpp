/***************************************************************************
                          ADM_vidResize.cpp  -  description
                             -------------------
   Resize a picture in YUV12

   w,h 		size of final picture
   dw,dh  size of black bordered picture


    begin                : Thu Mar 21 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_default.h"

#include "ADM_videoFilterDynamic.h"
#include "ADM_resizebis.hxx"
#include "ADM_vidResize.h"

#include "DIA_coreToolkit.h"


static FILTER_PARAM mpresizeParam={3,{"w","h","algo"}};

//********** Register chunk ************
VF_DEFINE_FILTER_UI(AVDMVideoStreamResize,mpresizeParam,
                resize,
                QT_TR_NOOP("Resize"),
                1,
                VF_TRANSFORM,
                QT_TR_NOOP("Picture resizer ported from Avisynth (C Version, slow)."));
//****************************************

char *AVDMVideoStreamResize::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Resize %lu x %lu --> %lu x %lu",
 				_in->getInfo()->width,
 				_in->getInfo()->height,
 				_info.width,
 				_info.height);
        
}
//_______________________________________________________________
AVDMVideoStreamResize::AVDMVideoStreamResize(
									AVDMGenericVideoStream *in,CONFcouple *couples)
{

  	_in=in;
   	memcpy(&_info,_in->getInfo(),sizeof(_info));
	_uncompressed=new ADMImage(_info.width,_info.height);

		if(couples)
		{
			 _param=NEW(RESIZE_PARAMS);
			GET(w);
			GET(h);
			GET(algo);
			_info.width=_param->w;
			_info.height=_param->h;

		}
			else
			{
				_param=NEW( RESIZE_PARAMS);
				_param->w=_info.width;
				_param->h = _info.height;
				_param->algo = 0;
			}			
			_intermediate_buffer=new uint8_t [3*_info.width*_in->getInfo()->height];	

  _info.encoding=1;
  _init=0;
	Vpattern_luma=NULL;
    Vpattern_chroma=NULL;
	Hpattern_luma=NULL;
    Hpattern_chroma=NULL;
}
#if !defined(MPLAYER_RESIZE_PREFFERED) 
AVDMGenericVideoStream *createResizeFromParam(AVDMGenericVideoStream *in,uint32_t x,uint32_t y)
{

	return new AVDMVideoStreamResize(in,x,y);
}
#endif
AVDMVideoStreamResize::AVDMVideoStreamResize(
									AVDMGenericVideoStream *in,uint32_t x,uint32_t y)
{

  	_in=in;
   	memcpy(&_info,_in->getInfo(),sizeof(_info));
	_uncompressed=new ADMImage(_info.width,_info.height);
	_param=NEW( RESIZE_PARAMS);
	_param->w=x;
	_param->h = y;
	_info.width=_param->w;
	_info.height=_param->h;
	_param->algo = 0;
	_intermediate_buffer=new uint8_t [3*_info.width*_in->getInfo()->height];

	_info.encoding=1;
	_init=0;
	Vpattern_luma=NULL;
	Vpattern_chroma=NULL;
	Hpattern_luma=NULL;
	Hpattern_chroma=NULL;
}

uint8_t	AVDMVideoStreamResize::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(3);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
	CSET(w);
	CSET(h);
	CSET(algo);
			return 1;

}
// ___ destructor_____________
AVDMVideoStreamResize::~AVDMVideoStreamResize()
{
 	delete  _uncompressed;
	delete [] _intermediate_buffer;
	DELETE(_param);
	_uncompressed=NULL;
	_intermediate_buffer=NULL;
 	endcompute();
}

//
//	Basically ask a uncompressed frame from editor and ask
//		GUI to decompress it .
//

uint8_t AVDMVideoStreamResize::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
static Image in,out;
	if(frame>=_info.nb_frames) 
	{
		printf("Filter : out of bound!\n");
		return 0;
	}
	
	ADM_assert(_param);	
	if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;
       		
	// do the resize in 3 passes, Y, U then V
	in.width=_in->getInfo()->width;
	in.height=_in->getInfo()->height;
	in.data=_uncompressed->data;

	out.width=_info.width;
	out.height=_info.height;
	out.data=data->data;	
	if(!_init)
	{
		_init=1;
		printf("\n Precomputing with algo :%lu\n",_param->algo);
		if(_param->algo>2)
                {
			printf("\n Wrong algorithm selection");
			ADM_assert(0);
		}
		precompute(&out,&in, _param->algo );
	}
       	zoom(&out,&in)         ;       
       data->flags=*flags=_uncompressed->flags;

       *len= _info.width*_info.height+(_info.width*_info.height>>1);
       data->copyInfo(_uncompressed);	
      return 1;
}


static int getResizeParams(uint32_t * w, uint32_t * h, uint32_t * algo,uint32_t ow,uint32_t oh,uint32_t fps);


extern uint8_t DIA_resize(uint32_t *width,uint32_t *height,uint32_t *algo,uint32_t originalw, uint32_t originalh,uint32_t fps1000);

uint8_t AVDMVideoStreamResize::configure(AVDMGenericVideoStream * instream)
{
    UNUSED_ARG(instream);

    RESIZE_PARAMS *par;
//    uint8_t ret=0;

    _init = 0;          // force recompute
    par = _param;



    if (!getResizeParams(&par->w, &par->h, &par->algo,instream->getInfo()->width,instream->getInfo()->height,_info.fps1000))
      {
      return 0;
      }
      printf("\n OK was selected \n");
    // update other parameters
    _info.width = _param->w;
    _info.height = _param->h;
    // intermediate buffer
    if (_intermediate_buffer)
      {
      delete  [] _intermediate_buffer;
      _intermediate_buffer = NULL;
      }
    //_intermediate_buffer=(uint8_t *)malloc(3*_info.width*_in->getInfo()->height);
    _intermediate_buffer =
    new uint8_t[3 * _info.width * _in->getInfo()->height];

    return 1;

}

//
//  
//
//
//   static GtkWidget *resi;
int getResizeParams(uint32_t * w, uint32_t * h, uint32_t * algo,uint32_t ow,uint32_t oh,uint32_t fps)
{
uint32_t ww,hh;
    while(1)
    {
        ww=*w;
        hh=*h;
        if(!DIA_resize(&ww,&hh,algo,ow,oh,fps)) return 0;
                if(!ww || !hh) GUI_Error_HIG(QT_TR_NOOP("Width and height cannot be 0"), NULL);
        else
                  if( ww&1 || hh &1) GUI_Error_HIG(QT_TR_NOOP("Width and height cannot be odd"), NULL);
            else
            {
                *w=ww;
                *h=hh;
                return 1;
            }
    }
}

