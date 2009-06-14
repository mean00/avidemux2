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
#include "DIA_coreToolkit.h"

#include "ADM_vidASS.h"
#include "ADM_colorspace.h"
#include "DIA_factory.h"



#ifndef DIR_SEP
# ifdef WIN32
#   define DIR_SEP '\\'
# else
#   define DIR_SEP '/'
# endif
#endif

static FILTER_PARAM assParam={7,
        { /* float */ "font_scale",
          /*float*/ "line_spacing",
          /* int */ "top_margin",
          /* int */ "bottom_margin",
          /* bool */ "extract_embedded_fonts",
          /* ADM_filename */ "fonts_dir",
          /* ADM_filename */ "subfile" }};

//********** Register chunk ************
//REGISTERX(VF_SUBTITLE, "ass",QT_TR_NOOP("ASS"),
//    QT_TR_NOOP("Add ASS/SSA subtitles to the picture."),VF_ASS,1,ass_create,ass_script);


VF_DEFINE_FILTER(ADMVideoSubASS,assParam,
                                ass,
                                QT_TR_NOOP("ASS"),
                                1,
                                VF_SUBTITLE,
                                QT_TR_NOOP("Add ASS/SSA subtitles to the picture."));
//************************************

char *ADMVideoSubASS::printConf() 
{
      static char buf[500];
      sprintf((char *)buf," ASS/SSA Subtitles: ");
      
      char *filename = (char*)_params->subfile;
      if(filename)
      {
          if(strrchr(filename, DIR_SEP) != NULL && *(strrchr(filename, DIR_SEP) + 1) != 0)
              filename = strrchr(filename, DIR_SEP) + 1;
          strncat(buf, filename, 49-strlen(buf));
          buf[49] = 0;
      }else
      {
        strcat(buf," (no sub)");       
      }
      return buf;
}


uint8_t ADMVideoSubASS::configure(AVDMGenericVideoStream * instream)
{
  UNUSED_ARG(instream);
  
#define PX(x) &(_params->x)
#define MKME(x,y) x=(ELEM_TYPE_FLOAT)_params->y
  ELEM_TYPE_FLOAT scale,spacing;
  
    MKME(scale,font_scale);
    MKME(spacing,line_spacing);
    
    diaElemFile       file(0,(char **)PX(subfile),QT_TR_NOOP("_Subtitle file (ASS/SSA):"), NULL, QT_TR_NOOP("Select Subtitle file"));
    diaElemFloat      dSpacing(&spacing,QT_TR_NOOP("_Line spacing:"),0.10,10.0);
    diaElemFloat      dScale(&scale,QT_TR_NOOP("_Font scale:"),0.10,10.0);
    diaElemUInteger   dTop(PX(top_margin),QT_TR_NOOP("_Top margin:"),0,200);
    diaElemUInteger   dBottom(PX(bottom_margin),QT_TR_NOOP("Botto_m margin"),0,200);
    
       diaElem *elems[5]={&file,&dSpacing,&dScale,&dTop,&dBottom};
  
   if( diaFactoryRun(QT_TR_NOOP("ASS"),5,elems))
   {
#undef MKME
#define MKME(x,y) _params->y=(float)x
    MKME(scale,font_scale);
    MKME(spacing,line_spacing);

     return 1;
   }
   return 0;
}

//_______________________________________________________________

ADMVideoSubASS::ADMVideoSubASS(AVDMGenericVideoStream *in, CONFcouple *conf) 
{
        _in=in;		
        memcpy(&_info,_in->getInfo(),sizeof(_info));
        _params = new ASSParams;
        ADM_assert(_params);
        
        
        if(conf) {
                #define _COUPLE_GET(x) conf->getCouple(#x, &(_params->x));
                _COUPLE_GET(font_scale)
                _COUPLE_GET(line_spacing)
                _COUPLE_GET(top_margin)
                _COUPLE_GET(bottom_margin)
                _COUPLE_GET(subfile)
                _COUPLE_GET(fonts_dir)
                _COUPLE_GET(extract_embedded_fonts)
        }	
        else {
                _params->font_scale = 1.;
                _params->line_spacing = _params->top_margin = _params->bottom_margin = 0;
                _params->subfile = NULL;
                _params->fonts_dir = (ADM_filename*)ADM_alloc(6*sizeof(ADM_filename));
                strcpy((char*)_params->fonts_dir, "/tmp/");
                _params->extract_embedded_fonts = 1;
        }

        _uncompressed=new ADMImage(_in->getInfo()->width,_in->getInfo()->height);
        ADM_assert(_uncompressed);
        _info.encoding=1;

        /* ASS initialization */
        _ass_lib = ass_library_init();
        _ass_track = NULL;
        _ass_rend = NULL;
        ADM_assert(_ass_lib);
        if(_params->subfile)
        {
              if(!init())
              {
                GUI_Error_HIG("Format ?","Are you sure this is an ass file ?"); 
              }
        }

}
// **********************************
uint8_t ADMVideoSubASS::init(void)
{
bool use_margins = ( _params->top_margin | _params->bottom_margin ) != 0;

        memcpy(&_info,_in->getInfo(),sizeof(_info));
        _info.height += _params->top_margin + _params->bottom_margin;

        ass_set_fonts_dir(_ass_lib, (const char*)_params->fonts_dir);
        ass_set_extract_fonts(_ass_lib, _params->extract_embedded_fonts);
        ass_set_style_overrides(_ass_lib, NULL);
#if ASS_HAS_GLOBAL
         if(_ass_rend) 
        {
              ass_renderer_done(_ass_rend);
              _ass_rend = NULL;
         }
#endif
        _ass_rend = ass_renderer_init(_ass_lib);

        ADM_assert(_ass_rend);
 
        ass_set_frame_size(_ass_rend, _info.width, _info.height);
        ass_set_margins(_ass_rend, _params->top_margin, _params->bottom_margin, 0, 0);
        ass_set_use_margins(_ass_rend, use_margins);
        ass_set_font_scale(_ass_rend, _params->font_scale);
        ass_set_fonts(_ass_rend, NULL, "Sans");
        //~ ass_set_aspect_ratio(_ass_rend, ((double)_info.width) / ((double)_info.height));
#if ASS_HAS_GLOBAL
        if(_ass_track) 
        {
              ass_free_track(_ass_track);
              _ass_track = NULL;
        }
#endif
       _ass_track = ass_read_file(_ass_lib, (char*)_params->subfile, NULL);

//        ADM_assert(_ass_track);
        if(!_ass_track)
          GUI_Error_HIG("SSA Error","Cannot read_file for *%s*",(char*)_params->subfile);
        return 1;
} 

//*******************************************
ADMVideoSubASS::~ADMVideoSubASS() 
{
      if(_uncompressed) DELETE(_uncompressed);
      
      if(_params) 
      {
        DELETE(_params->subfile);
        DELETE( _params->fonts_dir);
        DELETE(_params);
      }
#if ASS_HAS_GLOBAL
        if(_ass_rend) 
        {
              ass_renderer_done(_ass_rend);
              _ass_rend = NULL;
         }

        if(_ass_track) 
        {
              ass_free_track(_ass_track);
              _ass_track = NULL;
        }
        if(_ass_lib) 
        {
              ass_library_done(_ass_lib);
              _ass_lib = NULL;
        }
#endif
}
//*******************************************
#define _r(c)  ((c)>>24)
#define _g(c)  (((c)>>16)&0xFF)
#define _b(c)  (((c)>>8)&0xFF)
#define _a(c)  ((c)&0xFF)
#define rgba2y(c)  ( (( 263*_r(c)  + 516*_g(c) + 100*_b(c)) >> 10) + 16  )
#define rgba2u(c)  ( (( 450*_r(c) - 376*_g(c) -  73*_b(c)) >> 10) + 128 )
#define rgba2v(c)  ( ((-152*_r(c) - 298*_g(c) + 450*_b(c)) >> 10) + 128 )

uint8_t ADMVideoSubASS::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len, ADMImage *data, uint32_t *flags) 
{
        uint32_t  i, j, k, l, val;
        uint8_t y, u, v, opacity;
        int32_t orig_u, orig_v,klong,newu,newv;
        uint8_t orig_y;
        uint8_t *bitmap, *ydata, *udata, *vdata;

        if(frame>=_info.nb_frames)
        {
          printf("[SubAss] out of bound %u/%u\n",frame,_info.nb_frames); 
          return 0;
        }
        ADM_assert(_params);

        int64_t  where = (int64_t)(_info.orgFrame + frame) * 1000000LL /
                          (int64_t)_info.fps1000;

        if(!_in->getFrameNumberNoAlloc(frame, len, _uncompressed, flags))
                return 0;

        /* Add black borders top / bottom*/

        uint32_t page=_info.width*_info.height;
        uint32_t top, bottom;
        top=_info.width * (_params->top_margin &0xfffe);
        if(top>page) top=0;
        
        if(top)
        {
            memset(YPLANE(data),16,top);
            memset(UPLANE(data),128,top>>2);
            memset(VPLANE(data),128,top>>2);
        }
        memcpy(YPLANE(data) + top, YPLANE(_uncompressed),page-top);
        memcpy(UPLANE(data) + (top>>2), UPLANE(_uncompressed), (page-top)>>2);
        memcpy(VPLANE(data) + (top>>2), VPLANE(_uncompressed), (page-top)>>2);

        // Now do bottom
        top=_info.width * (_params->bottom_margin&0xfffe);
        if(top>page) top=0;
        if(top)
        {
            memset(YPLANE(data)+page-top,16,top);
            memset(UPLANE(data)+((page-top)>>2),128,top>>2);
            memset(VPLANE(data)+((page-top)>>2),128,top>>2);
        }
        // Do we have something to render ?
        if(!_ass_rend || !_ass_track)
        {
          printf("[Ass] No sub to render\n");
          return 1; 
        }
        int changed=0;
        ass_image_t *img = ass_render_frame(_ass_rend, _ass_track, where,&changed);
        

        while(img) {
                  y = rgba2y(img->color);
                  u = rgba2u(img->color);
                  v = rgba2v(img->color);

                  opacity = 255 - _a(img->color);

                  bitmap = img->bitmap;

                  uint32_t offset=img->dst_y * _info.width +img->dst_x;
                  uint32_t offset2=(img->dst_y>>1) * (_info.width>>1)+(img->dst_x >> 1); 

                  ydata = YPLANE(data) + offset ;
                  udata = UPLANE(data) + offset2 ;
                  vdata = VPLANE(data) + offset2 ;

                  for(i = 0; i < img->h; ++i) 
                  {
                          for(j = 0; j < img->w; ++j) 
                          {
                                  k = *(bitmap+j) * opacity / 255;
                                  orig_y = *(ydata+j);
                                  *(ydata+j) = (k*y + (255-k)*orig_y) / 255;
                          }

                          bitmap += img->stride;
                          ydata += _info.width;
                  }

                  bitmap = img->bitmap;
                  
                  newu=u-128;
                  newv=v-128;

                  for(i = 0; i < img->h; i += 2) 
                  {
                          for(j = 0, l = 0; j < img->w; j += 2, ++l) 
                          {
                                  val = 0;
                                  val += *(bitmap + j);
                                  val += *(bitmap + j + 1);
                                  val += *(bitmap + img->stride + j);
                                  val += *(bitmap + img->stride + j + 1);
                                  val >>= 2;

                                  k = val * opacity / 255;
                                  orig_u = *(udata+l);
                                  orig_v = *(vdata+l);

                                  orig_u=( k*u+(255-k)*orig_u)/255;
                                  orig_v=( k*v+(255-k)*orig_v)/255;
                                  *(udata+l) = orig_u;
                                  *(vdata+l) = orig_v;
                          }

                          bitmap += img->stride << 1;
                          udata += _info.width >> 1;
                          vdata += _info.width >> 1;
                  }

                  img = img->next;
        }

        return 1;
}

uint8_t	ADMVideoSubASS::getCoupledConf(CONFcouple **conf) 
{
        *conf=new CONFcouple(7);

#define _COUPLE_SET(x) (*conf)->setCouple(#x, _params->x);
        _COUPLE_SET(font_scale)
        _COUPLE_SET(line_spacing)
        _COUPLE_SET(top_margin)
        _COUPLE_SET(bottom_margin)
        _COUPLE_SET(subfile)
        _COUPLE_SET(fonts_dir)
        _COUPLE_SET(extract_embedded_fonts)

        return 1;
}
/************************************************/


