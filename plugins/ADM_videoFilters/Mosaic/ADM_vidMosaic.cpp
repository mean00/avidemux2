/***************************************************************************
                          
        Mosaic filter
                Put x*y images & resize them
    copyright            : (C) 2005 by mean
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


extern "C" {
#include "ADM_libraries/ADM_ffmpeg/ADM_lavcodec/avcodec.h"
#include "ADM_libraries/ADM_ffmpeg/ADM_lavutil/avutil.h"
#include "ADM_libraries/ADM_ffmpeg/ADM_libswscale/swscale.h"

}


#include "ADM_vidMosaic_param.h"

class  ADMVideoMosaic:public AVDMGenericVideoStream
 {

 protected:
                                MOSAIC_PARAMS   *_param;
                                SwsContext      *_context;
                                uint8_t         reset(void);
                                uint8_t         clean( void );
                                VideoCache     *vidCache;
                                uint32_t       tinyW,tinyH;
 public:

                                ADMVideoMosaic(  AVDMGenericVideoStream *in,CONFcouple *setup);
                                ADMVideoMosaic(        AVDMGenericVideoStream *in,uint32_t x,uint32_t y);
                                virtual                 ~ADMVideoMosaic();
          virtual               uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                                ADMImage *data,uint32_t *flags);
                                uint8_t configure( AVDMGenericVideoStream *instream);
        virtual                 char    *printConf(void) ;

          virtual uint8_t       getCoupledConf( CONFcouple **couples);


 }     ;
static FILTER_PARAM mosaicParam={4,{"hz","vz","shrink","show"}};

//REGISTERX(VF_MISC, "mosaic",QT_TR_NOOP("Mosaic"),
    //QT_TR_NOOP("Split the picture into tiny thumbnails."),VF_MOSAIC,1,     mosaic_create,mosaic_script);
//********** Register chunk ************

VF_DEFINE_FILTER(ADMVideoMosaic,mosaicParam,
    mosaic,
                QT_TR_NOOP("mosaic"),
                1,
                VF_MISC,
                QT_TR_NOOP("Split the picture into tiny thumbnails."));
//********** Register chunk ************

uint8_t ADMVideoMosaic::configure(AVDMGenericVideoStream * instream)
{
    UNUSED_ARG(instream);

#define PX(x) &(_param->x)
        
    diaElemUInteger   hz(PX(hz),QT_TR_NOOP("_Horizontal stacking:"),0,10);
    diaElemUInteger   vz(PX(vz),QT_TR_NOOP("_Vertical stacking:"),0,10);
    diaElemUInteger   shrink(PX(shrink),QT_TR_NOOP("_Shrink factor:"),0,10);
    diaElemToggle     show(PX(show),QT_TR_NOOP("Show _frame"));
    
    
       diaElem *elems[]={&hz,&vz,&shrink,&show};
  
   if(  diaFactoryRun(QT_TR_NOOP("Mosaic"),sizeof(elems)/sizeof(diaElem *),elems))
   {
        reset();
        return 1;
    }
    return 0;

}

//
//  
//
uint8_t ADMVideoMosaic::clean(void)
{
                if(_context)
                {
                        sws_freeContext(_context);
                }
                _context=NULL;
                if(_uncompressed) delete _uncompressed;
                _uncompressed=NULL;
                if(vidCache) delete vidCache;
                vidCache=NULL;
                return 1;
}

uint8_t ADMVideoMosaic::reset(void)
{
SwsFilter                  *srcFilter=NULL;
SwsFilter                  *dstFilter=NULL;
int                       flags=0;

                clean();
                flags=SWS_BICUBIC;


#ifdef ADM_CPU_X86
                
                #define ADD(x,y) if( CpuCaps::has##x()) flags|=SWS_CPU_CAPS_##y;
                ADD(MMX,MMX);           
                ADD(3DNOW,3DNOW);
                ADD(MMXEXT,MMX2);
#endif  
        
        

        memcpy(&_info,_in->getInfo(),sizeof(_info));
       

      

        tinyW=_info.width;
        tinyH=_info.height;
        
        tinyW/=_param->shrink;
        tinyH/=_param->shrink;

        if(tinyW&1) tinyW++;
        if(tinyH&1) tinyH++;
        

        _info.width=tinyW*_param->hz;
        _info.height=tinyH*_param->vz;


        _uncompressed=new ADMImage(_info.width,_info.height);
        vidCache=new VideoCache(_param->vz*_param->hz*2,_in);
        //_info.nb_frames=_info.nb_frames/(_param->vz*_param->hz);

  _context=sws_getContext(
                        _in->getInfo()->width,_in->getInfo()->height,
                        PIX_FMT_YUV420P,
                        tinyW,
                        tinyH,
                        PIX_FMT_YUV420P,
                        flags, srcFilter, dstFilter,NULL);

        if(!_context) return 0;
        return 1;


}
char *ADMVideoMosaic::printConf( void )
{
        ADM_FILTER_DECLARE_CONF(" Mosaic : %d hz, %d vz, %d shrink factor",
                                _in->getInfo()->width,
                                _param->hz,_param->vz,_param->shrink);
        
}
//_______________________________________________________________
ADMVideoMosaic::ADMVideoMosaic(
                                                                        AVDMGenericVideoStream *in,CONFcouple *couples)
{

        _in=in;
        memcpy(&_info,_in->getInfo(),sizeof(_info));
        _param=NEW(MOSAIC_PARAMS);

        if(couples)
        {

                GET(hz);
                GET(vz);
                GET(shrink);
                GET(show);
        
        }
        else
        {
                _param->hz=3;
                _param->vz =3;
                _param->shrink = 3;
                _param->show=1;
        }

        _context=NULL;
       vidCache=NULL;

        reset();
        _info.encoding=1;

}


uint8_t ADMVideoMosaic::getCoupledConf( CONFcouple **couples)
{

                        ADM_assert(_param);
                        *couples=new CONFcouple(4);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
        CSET(hz);
        CSET(vz);
        CSET(shrink);
        CSET(show);
                        return 1;

}
// ___ destructor_____________
ADMVideoMosaic::~ADMVideoMosaic()
{
        clean();
        DELETE(_param);

}

//
//      Basically ask a uncompressed frame from editor and ask
//              GUI to decompress it .
//

uint8_t ADMVideoMosaic::getFrameNumberNoAlloc(uint32_t frame,
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

ADMImage *curImage;
char txt[256];

                        for(int y=0;y<_param->vz;y++)
                          for(int x=0;x<_param->hz;x++)
                        {
                          curImage=vidCache->getImage(frame+y*_param->hz+x);
                          if(!curImage) continue;

                          if(_param->show)
                          {
                                sprintf(txt," %02d",frame+x+y*_param->hz);
                                drawString(curImage,2,0,txt);
                          }

                          
                          uint8_t *src[3];
                          uint8_t *dst[3];
                          int ssrc[3];
                          int ddst[3];

                          uint32_t page;

                          page=_in->getInfo()->width*_in->getInfo()->height;
                          src[0]=YPLANE(curImage);
                          src[1]=UPLANE(curImage);
                          src[2]=VPLANE(curImage);

                          ssrc[0]=_in->getInfo()->width;
                          ssrc[1]=ssrc[2]=_in->getInfo()->width>>1;

                          page=_info.width*tinyH;
                          
                          dst[0]=YPLANE(data)+page*y+tinyW*x;
                          dst[1]=UPLANE(data)+((page*y)/4)+((tinyW*x)/2);
                          dst[2]=VPLANE(data)+((page*y)/4)+((tinyW*x)/2);
                          ddst[0]=_info.width;
                          ddst[1]=ddst[2]=_info.width>>1;

                          sws_scale(_context,src,ssrc,0,_in->getInfo()->height,dst,ddst);
                        }
                        vidCache->unlockAll();
        return 1;
}

