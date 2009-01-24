/***************************************************************************
                          ADM_vidRotate.cpp  -  description
                             -------------------
    begin                : Sat Jan 8 2003
    copyright            : (C) 2003 by Tracy Harton
    email                : tracy@amphibious.org
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

typedef struct ROTATE_PARAM
{
  uint32_t width;
  uint32_t height;
  uint32_t angle;
}ROTATE_PARAM;

class  ADMVideoRotate:public AVDMGenericVideoStream
{

 protected:
  virtual char                  *printConf(void);
  ROTATE_PARAM                  *_param;

 public:
                  ADMVideoRotate(AVDMGenericVideoStream *in, CONFcouple *setup);
  virtual         ~ADMVideoRotate();
  virtual uint8_t configure( AVDMGenericVideoStream *instream) ;
  virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len, ADMImage *data,uint32_t *flags);
  virtual uint8_t	getCoupledConf( CONFcouple **couples)				;
 }     ;

static FILTER_PARAM rotpParam={3,{"width","height","angle"}};


//********** Register chunk ************
static FILTER_PARAM flipParam={0,{""}};

//REGISTERX(VF_TRANSFORM, "rotate",QT_TR_NOOP("Rotate"),QT_TR_NOOP(
//    "Rotate the picture by 90, 180 or 270 degrees."),VF_ROTATE,1,rotate_create,rotate_script);
VF_DEFINE_FILTER(ADMVideoRotate,flipParam,
                                rotate,
                                QT_TR_NOOP("Rotate"),
                                1,
                                VF_TRANSFORM,
                                QT_TR_NOOP("Rotate the picture by 90, 180 or 270 degrees."));
//************************************

static void do_rotate(ADMImage *source,ADMImage *target,uint32_t angle);

char *ADMVideoRotate::printConf( void )
{
  ADM_FILTER_DECLARE_CONF(" Rotate %u degrees", _param->angle);
  
}

ADMVideoRotate::ADMVideoRotate(AVDMGenericVideoStream *in, CONFcouple *couples)
{
  _in=in;		

  memcpy(&_info,_in->getInfo(),sizeof(_info)); 
  _info.encoding=1;

 // _uncompressed=new uint8_t [3*_in->getInfo()->width*_in->getInfo()->height];
 

  if(couples)
  {
   	 _param=NEW(ROTATE_PARAM);
	GET(width);
	GET(height);
	GET(angle);
	_info.width=_param->width;
	_info.height=_param->height;

  }
  else
  {
    _param = NEW( ROTATE_PARAM);
    printf("New Angle 0.0\n");
    _param->angle = 0;
    _param->width = _info.width;
    _param->height = _info.height;
  }
 _uncompressed=new ADMImage(_in->getInfo()->width,_in->getInfo()->height);
  printf("New Rotate %ld %ld %f\n", _info.width, _info.height, _param->angle);
  ADM_assert(_uncompressed);    	  	

}



uint8_t	ADMVideoRotate::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(3);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
			CSET(width);
			CSET(height);
			CSET(angle);

			return 1;

}


ADMVideoRotate::~ADMVideoRotate()
{
 	delete  _uncompressed;
	_uncompressed=NULL;
	DELETE(_param);
}
uint8_t ADMVideoRotate::getFrameNumberNoAlloc(uint32_t frame,
                                              uint32_t *len,
                                              ADMImage *data,
                                              uint32_t *flags)
{
 if(frame>= _info.nb_frames) return 0;
								
  // read uncompressed frame
  if(!_in->getFrameNumberNoAlloc(frame, len, _uncompressed, flags)) return 0;

  
  do_rotate(_uncompressed,data,_param->angle);

  //printf("%ld,%ld\n", _info.width, _info.height);

  *flags=_uncompressed->flags;
  *len= (_info.width * _info.height) + ((_info.width * _info.height) / 2);
   data->copyInfo(_uncompressed);	
  return 1;
}

void do_rotate(ADMImage *source,ADMImage *target,uint32_t angle)
{
uint8_t *in;
uint32_t in_w,in_h;

in=source->data;
in_w=source->_width;
in_h=source->_height;

uint32_t x, y;
uint32_t u_offset = (in_w * in_h);
uint32_t v_offset = u_offset + (in_w/2 * in_h/2);
uint32_t in_sub_w = in_w/2;
uint32_t in_sub_h = in_h/2;
uint32_t out_sub_w, out_sub_h;

uint8_t *out=target->data;
uint32_t *out_w=&(target->_width);
uint32_t *out_h=&(target->_height);

ADM_assert(in_w*in_h==(*out_w)*(*out_h));

// In general, for 0 <= i < width and 0 <= j < height, the pixel (x + i, y + j) is colored with red value 
// red value   rgb_buf[(j * rowstride) + (i * 3) + 0], 
// green value rgb_buf[(j * rowstride) + (i * 3) + 1], 
// blue value  rgb_buf[(j * rowstride) + (i * 3) + 2].

        switch(angle)
        {
        case 0:
                *out_w = in_w;
                *out_h = in_h;
                out_sub_w = *out_w/2;
                out_sub_h = *out_h/2;
            
                for(x = 0; x < *out_w; x++)
                for(y = 0; y < *out_h; y++)
                    *(out + (*out_w * y) + x) = *(in + (in_w * y) + x);
            
                for(x = 0; x < out_sub_w; x++)
                for(y = 0; y < out_sub_h; y++)
                {
                    *(out + u_offset + (out_sub_w * y) + x) = *(in + u_offset + (in_sub_w * y) + x);
                    *(out + v_offset + (out_sub_w * y) + x) = *(in + v_offset + (in_sub_w * y) + x);
                }
                break;
        case 90:
        {
            *out_w = in_h;
            *out_h = in_w;
            out_sub_w = *out_w/2;
            out_sub_h = *out_h/2;
        
            for(x = 0; x < *out_w; x++)
            for(y = 0; y < *out_h; y++)
                *(out + (*out_w * y) + x) = *(in + (in_w * (in_h-x-1)) + y);
        
            for(x = 0; x < out_sub_w; x++)
            for(y = 0; y < out_sub_h; y++)
            {
                *(out + u_offset + (out_sub_w * y) + x) = *(in + u_offset + (in_sub_w * (in_sub_h-x-1)) + y);
                *(out + v_offset + (out_sub_w * y) + x) = *(in + v_offset + (in_sub_w * (in_sub_h-x-1)) + y);
            }
        }
        break;
        case 180:
        {
            *out_w = in_w;
            *out_h = in_h;
            out_sub_w = *out_w/2;
            out_sub_h = *out_h/2;
        
            for(x = 0; x < *out_w; x++)
            for(y = 0; y < *out_h; y++)
                *(out + (*out_w * y) + x) = *(in + (in_w * (in_h-y-1)) + in_w-x-1);
        
        
            for(x = 0; x < out_sub_w; x++)
            for(y = 0; y < out_sub_h; y++)
                {
                *(out + u_offset + (out_sub_w * y) + x) = *(in + u_offset + (in_sub_w * (in_sub_h-y-1)) + in_sub_w-x-1);
                *(out + v_offset + (out_sub_w * y) + x) = *(in + v_offset + (in_sub_w * (in_sub_h-y-1)) + in_sub_w-x-1);
                }
        }
            break;
        case 270:
        {
            *out_w = in_h;
            *out_h = in_w;
            out_sub_w = *out_w/2;
            out_sub_h = *out_h/2;
        
            for(x = 0; x < *out_w; x++)
            for(y = 0; y < *out_h; y++)
                *(out + (*out_w * y) + x) = *(in + (in_w * x) + (in_w-y-1));
        
            for(x = 0; x < out_sub_w; x++)
            for(y = 0; y < out_sub_h; y++)
            {
                *(out + u_offset + (out_sub_w * y) + x) = *(in + u_offset + (in_sub_w * x) + (in_sub_w-y-1));
                *(out + v_offset + (out_sub_w * y) + x) = *(in + v_offset + (in_sub_w * x) + (in_sub_w-y-1));
            }
        }
        break;
        default:
            ADM_assert(0);
        }
}
uint8_t ADMVideoRotate::configure( AVDMGenericVideoStream *instream)
{
  uint8_t r;
  
  diaMenuEntry rotateValues[]={
      {0,QT_TR_NOOP("None"),QT_TR_NOOP("None")},
      {90,QT_TR_NOOP("90 degrees"),QT_TR_NOOP("90°")},
      {180,QT_TR_NOOP("180 degrees"),QT_TR_NOOP("180°")},
      {270,QT_TR_NOOP("270 degrees"),QT_TR_NOOP("270°")}
  };
  diaElemMenu     rotate(&(_param->angle),QT_TR_NOOP("_Angle:"),4,rotateValues,NULL);
  diaElem *allWidgets[]={&rotate};
  if( !diaFactoryRun(QT_TR_NOOP("Rotate"),1,allWidgets)) return 0;
  
  uint32_t w,h;
  w=_in->getInfo()->width;
  h=_in->getInfo()->height;
  if((_param->angle%180)==90)
  {
    _info.width=_param->width=h;
    _info.height=_param->height=w;
  }else
  {
      _info.width=_param->width=w;
      _info.height=_param->height=h;
  }
  return 1;
}

