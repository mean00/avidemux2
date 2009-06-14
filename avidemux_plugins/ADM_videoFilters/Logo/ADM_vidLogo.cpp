/***************************************************************************

		Put a logon on video

    copyright            : (C) 2007 by mean
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
/*
  Initial port from MPlayer by Moonz

*/
#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_colorspace.h"
#include "DIA_factory.h"

static FILTER_PARAM logoParam={4,
        { /* float */ "image",
          /*float*/ "x",
          /* int */ "y",
          /* int */ "alpha"}};


typedef struct 
{
		char *image;
		uint32_t x,y;
		uint32_t alpha;
}PARAM_LOGO;


class ADMVideoLogo : public AVDMGenericVideoStream
{
  
	PARAM_LOGO   *_param;
	ADMImage	 *_image;
	uint32_t	 _inited;
	uint8_t		 init(void);
	uint8_t		 cleanup(void);
  public:
                                
	  				ADMVideoLogo(AVDMGenericVideoStream *in,CONFcouple *couples);    
                    ~ADMVideoLogo(void);
    uint8_t         getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                          ADMImage *data,uint32_t *flags);
        
    char            *printConf( void );
    uint8_t         configure(AVDMGenericVideoStream *in);
    uint8_t         getCoupledConf( CONFcouple **couples);
};

//********** Register chunk ************
//REGISTERX(VF_MISC, "logo",QT_TR_NOOP("Logo"),QT_TR_NOOP("Add a png as logo."),VF_LOGO,1,logo_create,logo_script);
VF_DEFINE_FILTER(ADMVideoLogo,logoParam,
    logo,
                QT_TR_NOOP("Logo"),
                1,
                VF_MISC,
                QT_TR_NOOP("Add a png as logo."));
//********** Register chunk ************
//***********************************
char *ADMVideoLogo::printConf() 
{
	ADM_FILTER_DECLARE_CONF("Logo at %u %u, alpha %u",_param->x,_param->y);
}


uint8_t ADMVideoLogo::configure(AVDMGenericVideoStream * instream)
{
#define PX(x) &(_param->x)
	   diaElemFile       file(0,(char **)PX(image),QT_TR_NOOP("_Logo (jpg file):"), NULL, QT_TR_NOOP("Select JPEG file"));
	   diaElemUInteger   positionX(PX(x),QT_TR_NOOP("_X Position:"),0,_info.width);
	   diaElemUInteger   positionY(PX(y),QT_TR_NOOP("_Y Position:"),0,_info.height);
	   diaElemUInteger   alpha(PX(alpha),QT_TR_NOOP("_Alpha:"),0,255);
	    
	   diaElem *elems[4]={&file,&positionX,&positionY,&alpha};
	  
	   if( diaFactoryRun(QT_TR_NOOP("Logo"),4,elems))
	   {
		   init();
		   return 1;
	   }
	   return 0;
}

//_______________________________________________________________

ADMVideoLogo::ADMVideoLogo(AVDMGenericVideoStream *in, CONFcouple *couples) 
{
        _in=in;		
        memcpy(&_info,_in->getInfo(),sizeof(_info));
        _param = new PARAM_LOGO;
        ADM_assert(_param);
        
        
        if(couples) {
        
                GET(image)
                GET(x)
                GET(y)
                GET(alpha)
        }	
        else {
        	 		_param->image=ADM_strdup("/work/samples/r01.jpg");
        	 		_param->x=0;
        	 		_param->y=0;
        	 		_param->alpha=255;
        }
        _image=NULL;
        _inited=init();
        
        
        _info.encoding=1;

}
// **********************************
uint8_t ADMVideoLogo::init(void)
{
		cleanup();
		_image=createImageFromFile(_param->image);
		if(_image) return 1;
        return 0;
} 
// **********************************
uint8_t ADMVideoLogo::cleanup(void)
{
	if(_image) delete _image;
	_image=NULL;
	
	return 1;
} 

//*******************************************
ADMVideoLogo::~ADMVideoLogo() 
{
    
      
      if(_param) 
      {
    	 if(_param->image) delete _param->image;
    	 _param->image=NULL;
    	 delete _param;
    	 _param=NULL;
      }
      cleanup();

}
//*******************************************
uint8_t ADMVideoLogo::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len, ADMImage *data, uint32_t *flags) 
{
       

        if(frame>=_info.nb_frames)
        {
          printf("[Logo] out of bound %u/%u\n",frame,_info.nb_frames); 
          return 0;
        }
        ADM_assert(_param);

        if(!_in->getFrameNumberNoAlloc(frame, len, data, flags))
                return 0;

        if(!_image)
        {
        	printf("[LOGO] No image to put\n");
        	return 1;
        }
        // No alpha ATM
        _image->copyToAlpha(data,_param->x,_param->y,_param->alpha);
        return 1;
}

uint8_t	ADMVideoLogo::getCoupledConf(CONFcouple **couples) 
{
        *couples=new CONFcouple(4);

#define COUPLE_SET(x) (*couples)->setCouple((char *)#x,(_param->x));
        COUPLE_SET(image)
        COUPLE_SET(x)
        COUPLE_SET(y)
        COUPLE_SET(alpha)
        return 1;
}
/************************************************/
//EOF
