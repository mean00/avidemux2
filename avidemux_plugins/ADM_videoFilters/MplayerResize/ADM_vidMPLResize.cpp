/***************************************************************************
                          ADM_MPLvidResize.cpp  -  description
                             -------------------

			     Wrapper for Mplayer resizer by Michael Niedermayer
			     See swscale* for the actual resizing code.


    begin                : Thu Apr 16 2003
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
#include "DIA_coreToolkit.h"
#include "ADM_videoFilterDynamic.h"



extern "C" {
#include "ADM_libraries/ADM_ffmpeg/ADM_lavcodec/avcodec.h"
#include "ADM_libraries/ADM_ffmpeg/ADM_lavutil/avutil.h"
#include "ADM_libraries/ADM_ffmpeg/ADM_libswscale/swscale.h"
}

typedef struct alg
{
					int in;
					char *name;
}alg;
#define DECLARE(y) {SWS_##y,(char *)#y}

/**
	Convert mplayer-resize numbering <--> avidemux one

*/
alg algs[]={
				DECLARE(BILINEAR),
				DECLARE(BICUBIC),
				DECLARE(LANCZOS)
		};




class  AVDMVideoStreamMPResize:public AVDMGenericVideoStream
 {

 protected:
				RESIZE_PARAMS 	*_param;
				SwsContext	*_context;
				uint8_t 	reset(uint32_t nw, uint32_t old,uint32_t algo);
				uint8_t		clean( void );
 public:

  				AVDMVideoStreamMPResize(  AVDMGenericVideoStream *in,CONFcouple *setup);
				AVDMVideoStreamMPResize(	AVDMGenericVideoStream *in,uint32_t x,uint32_t y);
  				virtual 		~AVDMVideoStreamMPResize();
          virtual 		uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          							ADMImage *data,uint32_t *flags);
				uint8_t configure( AVDMGenericVideoStream *instream);
	virtual 		char 	*printConf(void) ;

          virtual uint8_t	getCoupledConf( CONFcouple **couples);


 }     ;
static FILTER_PARAM mpresizeParam={3,{"w","h","algo"}};

    

//********** Register chunk ************
VF_DEFINE_FILTER_UI(AVDMVideoStreamMPResize,mpresizeParam,
                mpresize,
                QT_TR_NOOP("MPlayer resize"),
                1,
                VF_TRANSFORM,
                QT_TR_NOOP("Change image size. Faster than Avisynth's Resize."));
//****************************************
static int getResizeParams(uint32_t * w, uint32_t * h, uint32_t * algo,uint32_t ow,uint32_t oh,uint32_t fps);
extern uint8_t DIA_resize(uint32_t *width,uint32_t *height,uint32_t *alg,uint32_t originalw, uint32_t originalh,uint32_t fps1000);

uint8_t AVDMVideoStreamMPResize::configure(AVDMGenericVideoStream * instream)
{
    UNUSED_ARG(instream);

    RESIZE_PARAMS *par;
//    uint8_t ret=0;


    par = _param;



    if (!getResizeParams(&par->w, &par->h, &par->algo,instream->getInfo()->width,instream->getInfo()->height,
    				_info.fps1000))
      {
	  return 0;
      }
      printf("\n OK was selected \n");
    // update other parameters
    _info.width = _param->w;
    _info.height = _param->h;
	// do update the filter
	reset(_param->w,_param->h,_param->algo);
    return 1;

}

//
//  
//

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
uint8_t AVDMVideoStreamMPResize::clean(void)
{
		if(_context)
		{
			sws_freeContext(_context);
		}
		_context=NULL;
		return 1;
}

uint8_t AVDMVideoStreamMPResize::reset(uint32_t nw, uint32_t old,uint32_t algo)
{
 				 SwsFilter 		    *srcFilter=NULL;
				 SwsFilter		    *dstFilter=NULL;
				 int 			   flags=0;


				clean();

				 // swsGetFlagsAndFilterFromCmdLine(&flags, &srcFilter, &dstFilter);

//SwsContext *getSwsContextFromCmdLine(int srcW, int srcH, int srcFormat, int dstW, int dstH, int dstFormat);

					flags=algo;
					switch(flags)
					{
						case 0: //bilinear
								flags=SWS_BILINEAR;break;
						case 1: //bicubic
								flags=SWS_BICUBIC;break;
						case 2: //Lanczos
								flags=SWS_LANCZOS;break;
						default:ADM_assert(0);

					}
#ifdef ADM_CPU_X86		
		#define ADD(x,y) if( CpuCaps::has##x()) flags|=SWS_CPU_CAPS_##y;
		ADD(MMX,MMX);		
		ADD(3DNOW,3DNOW);
		ADD(MMXEXT,MMX2);
#endif	
#ifdef ADM_CPU_ALTIVEC
		flags|=SWS_CPU_CAPS_ALTIVEC;
#endif
				    _context=sws_getContext(
				    		_in->getInfo()->width,_in->getInfo()->height,
						PIX_FMT_YUV420P,
		 				nw,old,
	   					PIX_FMT_YUV420P,
	    					flags, srcFilter, dstFilter,NULL);

				if(!_context) return 0;
				return 1;


}
char *AVDMVideoStreamMPResize::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" MPL Resize %lu x %lu --> %lu x %lu (%s)",
 				_in->getInfo()->width,
 				_in->getInfo()->height,
 				_info.width,
 				_info.height,
				algs[_param->algo].name);
        
}
//_______________________________________________________________
AVDMVideoStreamMPResize::AVDMVideoStreamMPResize(
									AVDMGenericVideoStream *in,CONFcouple *couples)
{

  	_in=in;
   	memcpy(&_info,_in->getInfo(),sizeof(_info));
	//_uncompressed=(uint8_t *)malloc(3*_info.width*_info.height);
//	_uncompressed=new uint8_t[3*_info.width*_info.height];
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

	_context=NULL;
	reset(_param->w,_param->h,_param->algo);
  _info.encoding=1;

}
#ifdef MPLAYER_RESIZE_PREFFERED
AVDMGenericVideoStream *createResizeFromParam(AVDMGenericVideoStream *in,uint32_t x,uint32_t y)
{

	return new AVDMVideoStreamMPResize(in,x,y);
}
#endif
AVDMVideoStreamMPResize::AVDMVideoStreamMPResize(
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

	_context=NULL;
	reset(_param->w,_param->h,_param->algo);

}

uint8_t	AVDMVideoStreamMPResize::getCoupledConf( CONFcouple **couples)
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
AVDMVideoStreamMPResize::~AVDMVideoStreamMPResize()
{
 	delete  _uncompressed;
	_uncompressed=NULL;

	DELETE(_param);
	clean();

}

//
//	Basically ask a uncompressed frame from editor and ask
//		GUI to decompress it .
//

uint8_t AVDMVideoStreamMPResize::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
			if(frame>=_info.nb_frames) 
			{
				printf("Filter : out of bound!\n");
				return 0;
			}
	
			ADM_assert(_param);

       			if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;

			uint8_t *src[3];
			uint8_t *dst[3];
			int ssrc[3];
			int ddst[3];

			uint32_t page;

			page=_in->getInfo()->width*_in->getInfo()->height;
			src[0]=YPLANE(_uncompressed);
			src[1]=UPLANE(_uncompressed);
			src[2]=VPLANE(_uncompressed);

			ssrc[0]=_in->getInfo()->width;
			ssrc[1]=ssrc[2]=_in->getInfo()->width>>1;

			page=_info.width*_info.height;
			dst[0]=YPLANE(data);
			dst[1]=UPLANE(data);
			dst[2]=VPLANE(data);
			ddst[0]=_info.width;
			ddst[1]=ddst[2]=_info.width>>1;

			sws_scale(_context,src,ssrc,0,_in->getInfo()->height,dst,ddst);
			data->copyInfo(_uncompressed);
	return 1;
}

