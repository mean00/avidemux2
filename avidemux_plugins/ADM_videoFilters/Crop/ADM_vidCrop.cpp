/***************************************************************************
                          ADM_vidCrop.cpp  -  description
                             -------------------
          Crop top/left/right/bottom

          Each one ,must be even


    begin                : Sun Mar 24 2002
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
#include "DIA_factory.h"


#include "ADM_vidCrop_param.h"
class  AVDMVideoStreamCrop:public AVDMGenericVideoStream
 {

 protected:
                CROP_PARAMS          *_param;

 public:

                                AVDMVideoStreamCrop(  AVDMGenericVideoStream *in,CONFcouple *couples);
        virtual                 ~AVDMVideoStreamCrop();
        virtual uint8_t         getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                                ADMImage *data,uint32_t *flags);
        virtual char    *printConf(void) ;
                  uint8_t       configure( AVDMGenericVideoStream *instream);
          virtual uint8_t       getCoupledConf( CONFcouple **couples);


 }     ;
 
static FILTER_PARAM cropParam={4,{"left","right","top","bottom"}};


VF_DEFINE_FILTER_UI(AVDMVideoStreamCrop,cropParam,
    crop,
                                QT_TR_NOOP("Crop"),
                                1,
                                VF_TRANSFORM,
                                QT_TR_NOOP("Remove lines from top/bottom/left/right."));


char *AVDMVideoStreamCrop::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Crop %lu x %lu --> %lu x %lu",
 				_in->getInfo()->width,
 				_in->getInfo()->height,
 				_info.width,
 				_info.height);
 
}

//_______________________________________________________________

AVDMVideoStreamCrop::AVDMVideoStreamCrop(
									AVDMGenericVideoStream *in,CONFcouple *couples)
{


  	_in=in;		
   	memcpy(&_info,_in->getInfo(),sizeof(_info));
	
		if(couples)
		{
			_param=	NEW(CROP_PARAMS);
			GET(left);
			GET(right);
			GET(top);
			GET(bottom);
				// Consistency check
				if(  _in->getInfo()->width<(_param->right+_param->left))
						{
                      		printf("\n Warning Cropping too much width ! Width reseted !\n");
								_param->right=_param->left=0;
						}
				if(  _in->getInfo()->height<(_param->bottom+_param->top))
						{
                      		printf("\n Warning Cropping too much height ! Height reseted !\n");
								_param->bottom=_param->top=0;
						}

			_info.width=_in->getInfo()->width-_param->right-_param->left;		
			_info.height=_in->getInfo()->height-_param->bottom-_param->top;	
			
		}	
			else 			
		{	// default parameter	
				_param=	NEW(CROP_PARAMS); ;
				_param->left=_param->top=
				_param->right=_param->bottom=0;
		}				
					
 	//_uncompressed=(uint8_t *)malloc(3*_in->getInfo()->width*_in->getInfo()->height);
 	//_uncompressed=new uint8_t [3*_in->getInfo()->width*_in->getInfo()->height];
	_uncompressed=new ADMImage(_in->getInfo()->width,_in->getInfo()->height);
  	ADM_assert(_uncompressed);
  	_info.encoding=1;
	  

  	  	
}
AVDMVideoStreamCrop::~AVDMVideoStreamCrop()
{
 	delete _uncompressed;
 	DELETE(_param);
}

//
//	Basically ask a uncompressed frame from editor and ask
//		GUI to decompress it .
//

uint8_t AVDMVideoStreamCrop::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{

			ADM_assert(frame<_info.nb_frames);
			ADM_assert(_param);									
		if(frame>= _info.nb_frames) return 0;
			// read uncompressed frame
       		if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;
       		
       		// Crop Y luma
       		uint32_t y,x,line;
       		uint8_t *src,*src2,*dest;
       		
       		y=_in->getInfo()->height;
       		x=_in->getInfo()->width;
       		line=_info.width;
       		src=YPLANE(_uncompressed)+_param->top*x+_param->left;
       		dest=YPLANE(data);
       		
       		for(uint32_t k=_info.height;k>0;k--)
       			{
       			 	    memcpy(dest,src,line);
       			 	    src+=x;
       			 	    dest+=line;
       			}
       		 // Crop U  & V
       		 	src=UPLANE(_uncompressed)+(x*_param->top>>2)+(_param->left>>1);
       		 	src2=VPLANE(_uncompressed)+(x*_param->top>>2)+(_param->left>>1);
			dest=UPLANE(data);
       		 	line>>=1;
       		 	x>>=1;       		       		 	
       		 		for(uint32_t k=((_info.height)>>1);k>0;k--)
       		 		{
       		 	    	  	memcpy(dest,src,line);
       			 	    	src+=x;
       			 	    	dest+=line;
       		 		}
			dest=VPLANE(data);	
       		 		for(uint32_t k=((_info.height)>>1);k>0;k--)
       		 		{
       		 	    	  	memcpy(dest,src2,line);
       			 	    	src2+=x;
       			 	    	dest+=line;
       		 		}	
       		  *flags=0;
       		  *len= _info.width*_info.height+(_info.width*_info.height>>1);
		  data->copyInfo(_uncompressed);
      return 1;
}
/**
	Return the configuration as a couple of string
	Up to the client to free them afterward
*/
uint8_t	AVDMVideoStreamCrop::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(4);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
			CSET(left);
			CSET(right);
			CSET(top);
			CSET(bottom);
			return 1;

}
extern int DIA_getCropParams(const char *name, CROP_PARAMS *param, AVDMGenericVideoStream *in);
uint8_t AVDMVideoStreamCrop::configure( AVDMGenericVideoStream *instream)

{
		uint8_t r;
		uint32_t w,h;
    	if(r = (DIA_getCropParams("Crop Settings",_param,instream )))
    	{
			w=_param->left+_param->right;
			h=_param->top+_param->bottom;
			ADM_assert(w<instream->getInfo()->width);
			ADM_assert(h<instream->getInfo()->height);
			_info.width=instream->getInfo()->width-w;
			_info.height=instream->getInfo()->height-h;
		}
		return r;
}

