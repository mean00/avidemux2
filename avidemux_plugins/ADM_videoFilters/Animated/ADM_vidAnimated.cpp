
/***************************************************************************
    (c)Mean 2006 fixounet@free.fr
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

#include "DIA_factory.h"

#include "ADM_vidAnimated.h"
#include "ADM_inputs/ADM_inpics/ADM_pics.h"
#include "ADM_codecs/ADM_codec.h" //For decoders
static FILTER_PARAM animated_template={10,
    {"tc0","tc1","tc2",
    "tc3","tc4","tc5",
    "isNTSC","backgroundImg","vignetteW","vignetteH"
    }};

//REGISTERX(VF_MISC, "animatedmenu",QT_TR_NOOP("Animated Menu"), QT_TR_NOOP("Create a video made of 6 mini windows, very useful to do DVD menus."),
//        VF_ANIMATED,1,animated_create,animated_script);

//********************************************
VF_DEFINE_FILTER(ADMVideoAnimated,cropParam,
    animatedmenu,
                QT_TR_NOOP("Animated Menu"),
                1,
                VF_MISC,
                QT_TR_NOOP("Create a video made of 6 mini windows, very useful to do DVD menus."));
//********************************************
#define LOOKUP 4
#define CACHE_SIZE (LOOKUP*2+3)


extern uint8_t DIA_animated(ANIMATED_PARAM *param,uint32_t w,uint32_t h, uint32_t n);

uint8_t ADMVideoAnimated::configure(AVDMGenericVideoStream *in)
{
  ADV_Info *f=in->getInfo();
   if(DIA_animated(_param,f->width,f->height,f->nb_frames))
   {
      setup();
      return 1;
   }
   return 0;
}

char *ADMVideoAnimated::printConf( void )
{
	ADM_FILTER_DECLARE_CONF(" Animated Menu ");
}
uint8_t ADMVideoAnimated::setup( void)
{
    cleanup();
    for(int i=0;i<MAX_VIGNETTE;i++) _caches[i]=new VideoCache(CACHE_SIZE,_in);  // 18 is good for mpeg2
    _resizer=new ADMImageResizer(_in->getInfo()->width,_in->getInfo()->height,
            _param->vignetteW,_param->vignetteH);
    _image=new ADMImage(_param->vignetteW,_param->vignetteH);
    loadImage();
}
uint8_t ADMVideoAnimated::cleanup( void)
{
   
    for(int i=0;i<MAX_VIGNETTE;i++) 
    {
        if(_caches[i]) delete _caches[i];
        _caches[i]=NULL;
    }
    if(_resizer) delete _resizer;
    if(_image) delete _image;
    if(_BkgGnd) delete _BkgGnd;
    _resizer=NULL;
    _image=NULL;
    _BkgGnd=NULL;

}
ADMVideoAnimated::ADMVideoAnimated(AVDMGenericVideoStream *in,CONFcouple *couples) 
{
   _resizer=NULL;
   _image=NULL;
   _BkgGnd=NULL;
   for(int i=0;i<MAX_VIGNETTE;i++) _caches[i]=NULL;

   _in=in;
   memcpy(&_info,_in->getInfo(),sizeof(_info));    
   _info.encoding=1;
   _uncompressed=NULL;
   _param=NEW(ANIMATED_PARAM);
   if(couples)
   {
        GET(isNTSC);
        GET(backgroundImg);
        GET(vignetteW);
        GET(vignetteH);
#undef GET
#define GET(x)  couples->getCouple((char *)"tc"#x,&(_param->timecode[x]))
        GET(0);
        GET(1);
        GET(2);
        GET(3);
        GET(4);
        GET(5);
   }
   else // Default
   {
#define MKP(x,y) _param->x=y;
            MKP(isNTSC,0);
            MKP(vignetteW,160);
            MKP(vignetteH,120);
#ifdef __WIN32
       MKP(backgroundImg,(ADM_filename *)ADM_strdup("c:\\test.jpg"));
#else
            MKP(backgroundImg,(ADM_filename *)ADM_strdup("/tmp/taist.jpg"));
#endif
#undef MKP
#define MKP(x,y) _param->timecode[x]=y
        MKP(0,0);
        MKP(1,100);
        MKP(2,200);
        MKP(3,300);
        MKP(4,500);
        MKP(5,600);
   }
   if(_param->isNTSC) _info.height=480;
        else _info.height=576;
    _info.width=720;
    setup();
}
//____________________________________________________________________
ADMVideoAnimated::~ADMVideoAnimated()
{
   if(_param->backgroundImg) ADM_dealloc(_param->backgroundImg);
   delete _param;
   _param=NULL;
   _uncompressed=NULL;
   cleanup();
}
#define BYTE uint8_t 
#ifndef MAX
#define MAX(x,y) ((x)>(y) ?(x):(y))
#endif
#ifndef MIN
#define MIN(x,y) ((x)<(y) ?(x):(y))
#endif
//_____________________________________________________________
// It is slow as we should do the resize in place (one 
//   bunch of memcpy saved)
// Since it is killing the editor cache anyway, speed it not an issue
//______________________________________________________________
uint8_t ADMVideoAnimated::getFrameNumberNoAlloc(uint32_t frame,
  uint32_t *len,
  ADMImage *data,
  uint32_t *flags)
  {
    ADMImage *in;
    uint32_t offset,pool;

    if(frame>= _info.nb_frames) return 0;
    
    // Clean the image
    if(_BkgGnd)
        data ->duplicate(_BkgGnd);
    else
        {
                data->blacken();
                printf("No background image\n");
        }
 const int x_coordinate[3]={LEFT_MARGIN,360-(_param->vignetteW)/2,720-(_param->vignetteW)-LEFT_MARGIN};
 const int y_coordinate[2]={TOP_MARGIN,TOP_MARGIN+(_param->vignetteH)+TOP_MARGIN};

     // Do 3 top
     for(int line=0;line<2;line++)
        for(int i=0;i<3;i++)
        {
            pool=i+(3*line);
            if(_param->timecode[pool]+frame<_in->getInfo()->nb_frames) 
            {
                // Load LOOKUP frames ...
                if(!(frame%LOOKUP))
                    for(int lookup=0;lookup<LOOKUP;lookup++)
                    {
                        _caches[pool]->getImage(_param->timecode[pool]+frame+lookup);
                    }
                // Unlock them
                _caches[pool]->unlockAll();
                in=_caches[pool]->getImage(_param->timecode[pool]+frame);
                if(in) 
                {
                     _resizer->resize(in,_image);
                    _caches[pool]->unlockAll();
                }else
                {   
                    printf("[Animated] Failed to get image %u for pool %u\n",_param->timecode[pool]+frame,pool);
                    _image->blacken();
                }
            }else
            {   // Blacken 
                printf("[Animated] out of bound : %u for pool %u\n",_param->timecode[pool]+frame,pool);
                 _image->blacken();
            }
            // Blit

            _image->copyTo(data,x_coordinate[i],y_coordinate[line]);
        }

	return 1;

}


uint8_t	ADMVideoAnimated::getCoupledConf( CONFcouple **couples)
{
   
      ADM_assert(_param);
      *couples=new CONFcouple(10);
#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
       CSET(isNTSC);
       CSET(backgroundImg);
        CSET(vignetteW);
        CSET(vignetteH);
#undef CSET
#define CSET(x)  (*couples)->setCouple((char *)"tc"#x,(_param->timecode[x]))
        CSET(0);
        CSET(1);
        CSET(2);
        CSET(3);
        CSET(4);
        CSET(5);

      return 1;
}
uint8_t	ADMVideoAnimated::loadImage(void)
{
picHeader *pic=NULL;
decoders *decoder=NULL;
ADMImage  *fullSize=NULL;
ADMImageResizer *resizer=NULL;
uint32_t len=0,flags_removed=0;
uint32_t w,h;
uint8_t *extraData=NULL;
uint32_t extraDataSize=0;

uint8_t *rdBuffer=NULL;

    if(_BkgGnd) delete _BkgGnd;
    _BkgGnd=NULL;

    // open the jpg file and load it to binary
    
    pic=new picHeader;
    if(!pic->open((char *)_param->backgroundImg))
    {
        printf("[Animated] Cannot load background image\n");
        goto skipIt;
    }
  
    // Ok, now we need its size
    
    w=pic->getWidth();
    h=pic->getHeight();
    printf("[Animated]Pic: %d x %d\n",w,h);
    pic->getExtraHeaderData(&extraDataSize,&extraData);
    //********
    {
        aviInfo info;
        pic->getVideoInfo(&info);
        decoder=getDecoder (info.fcc, w,h,extraDataSize,extraData);
    }
     if(!decoder) 
    {
        printf("[Animated]Cannot get decoder\n");
        goto skipIt;
    }
    // Build new image
    fullSize=new ADMImage(w,h);
    fullSize->blacken();
    rdBuffer=new uint8_t[w*h*3];     // Hardcoded!
    ADMCompressedImage img;
    img.data=rdBuffer;
    if(!pic->getFrameNoAlloc(0,&img))
    {
        printf("[Animated]Get frame failed\n");
        goto skipIt;
    }
    // Decode it
    if(!decoder->uncompress (&img, fullSize))
    {
        printf("[Animated]Decoding failed\n");
        goto skipIt;
    }
    if(fullSize->_colorspace!=ADM_COLOR_YV12)
    {
        printf("[Animated]Wrong colorspace, only yv12 supported\n");
        goto skipIt;
    }
    // Need to packit ?
    if(fullSize->_planeStride[0])
        fullSize->pack(0);
    // Resize it
    _BkgGnd=new ADMImage(_info.width,_info.height);
    resizer=new ADMImageResizer(w,h,_info.width,_info.height);
    //Util_saveJpg ("/tmp/before.jpg",w,h,fullSize);
    if(!resizer->resize(fullSize,_BkgGnd))
    {
        delete _BkgGnd;
        _BkgGnd=NULL;
        printf("[Animated]Resize failed\n");
        
    }else
    {
        printf("[Animated]Image ready\n");
    }
    
skipIt:
    {
        if(decoder) delete decoder;
        decoder=NULL;
        if(pic)     delete pic;
        pic=NULL;
        if(fullSize)     delete fullSize;
        fullSize=NULL;
        if(resizer)     delete resizer;
        resizer=NULL;
        if(rdBuffer)     delete [] rdBuffer;
        rdBuffer=NULL;
    }
    return 1;


}


// EOF
