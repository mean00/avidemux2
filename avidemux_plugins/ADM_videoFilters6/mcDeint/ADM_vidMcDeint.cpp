//
//
// Port to avidemux2 by mean
// Original filter by M Niedermayer
// See below
/*
    Copyright (C) 2006 Michael Niedermayer <michaelni@gmx.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "mcdeint.h"
#include "mcdeint_desc.cpp"
#include "DIA_factory.h"

extern "C"
{
    #include "libavcodec/avcodec.h"
}


struct vf_priv_s {
    int mode;
    int qp;
    int parity;
#if 0
    int temp_stride[3];
    uint8_t *src[3];
    int16_t *temp[3];
#endif
    int outbuf_size;
    uint8_t *outbuf;
    AVCodecContext *avctx_enc;
    AVFrame *frame;
    AVFrame *frame_dec;
};


/**
    \class horizontalFlipFilter
*/
class AVDMVideoMCDeint : public  ADM_coreVideoFilterCached
{
protected:
        mcdeint          _param;
        uint8_t         init();
        uint8_t         cleanup();
        vf_priv_s        priv;
public:
                    AVDMVideoMCDeint(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~AVDMVideoMCDeint();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void);           /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   AVDMVideoMCDeint,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "mcdeinterlace",            // internal name (must be uniq!)
                       QT_TRANSLATE_NOOP("mcdeint", "MCDeint"),            // Display name
                       QT_TRANSLATE_NOOP("mcdeint", "Motion compensation deinterlacer. Ported from MPlayer.") // Description
                    );

//********** Register chunk ************

static void filter(struct vf_priv_s *p, uint8_t *dst[3], uint8_t *src[3], int dst_stride[3], int src_stride[3], int width, int height);


/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         AVDMVideoMCDeint::getCoupledConf(CONFcouple **couples)
{
      return ADM_paramSave(couples, mcdeint_param,&_param);
}

void AVDMVideoMCDeint::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, mcdeint_param, &_param);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *AVDMVideoMCDeint::getConfiguration(void)
{
    static char conf[256];
    snprintf(conf,255,"MC deinterlacer : Mode %d, qp %d, parity %d ",_param.mode,_param.qp,_param.initial_parity);
    return conf;
}
/**
     \fn configure
*/
bool AVDMVideoMCDeint::configure()
{
  
   diaMenuEntry menuMode[4]={{0,QT_TRANSLATE_NOOP("mcdeint","Fast"),NULL},
                             {1,QT_TRANSLATE_NOOP("mcdeint","Medium"),NULL},
                             {2,QT_TRANSLATE_NOOP("mcdeint","Slow iterative motion search"),NULL},
                             {3,QT_TRANSLATE_NOOP("mcdeint","Extra slow (same as 3+multiple reference frames)"),NULL}
                          };
  
    diaElemMenu     menu1(&(_param.mode),QT_TRANSLATE_NOOP("mcdeint","_Mode:"), 4,menuMode);
    diaElemToggle     menu2(&(_param.initial_parity),QT_TRANSLATE_NOOP("mcdeint","Bottom :"));
    diaElemUInteger qp(&(_param.qp),QT_TRANSLATE_NOOP("mcdeint","_Qp:"),1,60);
    
    diaElem *elems[3]={&menu1,&menu2,&qp};
  
    return  diaFactoryRun(QT_TRANSLATE_NOOP("mcdeint","mcDeinterlace"),3,elems);

}
/**
    \fn ctor
*/
AVDMVideoMCDeint::AVDMVideoMCDeint( ADM_coreVideoFilter *in,CONFcouple *setup) 
: ADM_coreVideoFilterCached(4,in,setup)
{	
	 if(!setup || !ADM_paramLoad(setup,mcdeint_param,&_param))
    {
        // Default value
         _param.mode=0;
         _param.qp=1;
         _param.initial_parity=0;
        
    }
    init();
  	  	
}
/**
    \fn dtor
*/
AVDMVideoMCDeint::~AVDMVideoMCDeint()
{
      cleanup();
}


/**
    \fn getNextFrame
*/
bool         AVDMVideoMCDeint::getNextFrame(uint32_t *fn,ADMImage *image)
{
   
ADMImage *curImage;
char txt[256];
uint32_t frame=nextFrame++;
            *fn=frame;
            curImage=vidCache->getImage(frame);
            if(!curImage)
            {
                    printf("MCDeint : error getting frame\n");
                    return 0;
            }
              
                  // Prepare to call filter...
              uint8_t *dplanes[3],*splanes[3];
              int dstride[3],sstride[3];
              for(int i=0;i<3;i++)
                {
                    ADM_PLANE plane=(ADM_PLANE)i;

                    splanes[i]=curImage->GetReadPtr(plane);
                    sstride[i]=curImage->GetPitch(plane);

                    dplanes[i]=image->GetWritePtr(plane);
                    dstride[i]=image->GetPitch(plane);
                    
                }

              filter(&priv, dplanes, splanes, dstride, sstride, info.width, info.height);
              image->copyInfo(curImage);
              vidCache->unlockAll();
        return 1;
}
/**
    \fn init
*/
uint8_t AVDMVideoMCDeint::init( void )
{
  memset(&priv,0,sizeof(priv));
  int i;
  
  AVCodec *enc= avcodec_find_encoder(CODEC_ID_SNOW);
  ADM_assert(enc);

        for(i=0; i<3; i++)
        {
            AVCodecContext *avctx_enc;
            avctx_enc=     priv.avctx_enc= avcodec_alloc_context();
            avctx_enc->width = info.width;
            avctx_enc->height = info.height;
            avctx_enc->time_base= (AVRational){1,25};  // meaningless
            avctx_enc->gop_size = 300;
            avctx_enc->max_b_frames= 0;
            avctx_enc->pix_fmt = PIX_FMT_YUV420P;
            avctx_enc->flags = CODEC_FLAG_QSCALE | CODEC_FLAG_LOW_DELAY;
            avctx_enc->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
            avctx_enc->global_quality= 1;
            avctx_enc->flags2= CODEC_FLAG2_MEMC_ONLY;
            avctx_enc->me_cmp=
            avctx_enc->me_sub_cmp= FF_CMP_SAD; //SSE;
            avctx_enc->mb_cmp= FF_CMP_SSE;

            switch(_param.mode)
            {
            case 3:
                avctx_enc->refs= 3;
            case 2:
                avctx_enc->me_method= ME_ITER;
            case 1:
                avctx_enc->flags |= CODEC_FLAG_4MV;
                avctx_enc->dia_size=2;
//                avctx_enc->mb_decision = MB_DECISSION_RD;
            case 0:
                avctx_enc->flags |= CODEC_FLAG_QPEL;
            }

            avcodec_open(avctx_enc, enc);

        }
        priv.frame= av_frame_alloc();

        priv.outbuf_size= info.width*info.height*10;
        priv.outbuf= (uint8_t *)ADM_alloc(priv.outbuf_size);
        priv.parity=_param.initial_parity;
  return 1;
}
/**
    \fn cleanup
*/
uint8_t AVDMVideoMCDeint::cleanup( void )
{
  //
   avcodec_close(priv.avctx_enc);
   av_free(priv.avctx_enc);
   ADM_dezalloc(priv.outbuf);
   memset(&priv,0,sizeof(priv));
  return 1;
}


/*
Known Issues:
* The motion estimation is somewhat at the mercy of the input, if the input
  frames are created purely based on spatial interpolation then for example
  a thin black line or another random and not interpolateable pattern
  will cause problems
  Note: completly ignoring the "unavailable" lines during motion estimation 
  didnt look any better, so the most obvious solution would be to improve
  tfields or penalize problematic motion vectors ...

* If non iterative ME is used then snow currently ignores the OBMC window
  and as a result sometimes creates artifacts

* only past frames are used, we should ideally use future frames too, something
  like filtering the whole movie in forward and then backward direction seems 
  like a interresting idea but the current filter framework is FAR from
  supporting such things

* combining the motion compensated image with the input image also isnt
  as trivial as it seems, simple blindly taking even lines from one and
  odd ones from the other doesnt work at all as ME/MC sometimes simple
  has nothing in the previous frames which matches the current, the current
  algo has been found by trial and error and almost certainly can be
  improved ...
*/



#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define ABS(a) ((a) > 0 ? (a) : (-(a)))

//===========================================================================//


static void filter(struct vf_priv_s *p, uint8_t *dst[3], uint8_t *src[3], int dst_stride[3], int src_stride[3], int width, int height){
    int x, y, i;
    int out_size;

    for(i=0; i<3; i++){
        p->frame->data[i]= src[i];
        p->frame->linesize[i]= src_stride[i];
    }

    p->avctx_enc->me_cmp=
    p->avctx_enc->me_sub_cmp= FF_CMP_SAD /*| (p->parity ? FF_CMP_ODD : FF_CMP_EVEN)*/;
    p->frame->quality= p->qp*FF_QP2LAMBDA;
    out_size = avcodec_encode_video(p->avctx_enc, p->outbuf, p->outbuf_size, p->frame);
    p->frame_dec = p->avctx_enc->coded_frame;

    for(i=0; i<3; i++){
        int is_chroma= !!i;
        int w= width >>is_chroma;
        int h= height>>is_chroma;
        int fils= p->frame_dec->linesize[i];
        int srcs= src_stride[i];

        for(y=0; y<h; y++){
            if((y ^ p->parity) & 1){
                for(x=0; x<w; x++){
                    if((x-2)+(y-1)*w>=0 && (x+2)+(y+1)*w<w*h){ //FIXME either alloc larger images or optimize this
                        uint8_t *filp= &p->frame_dec->data[i][x + y*fils];
                        uint8_t *srcp= &src[i][x + y*srcs];
                        int diff0= filp[-fils] - srcp[-srcs];
                        int diff1= filp[+fils] - srcp[+srcs];
                        int spatial_score= ABS(srcp[-srcs-1] - srcp[+srcs-1])
                                          +ABS(srcp[-srcs  ] - srcp[+srcs  ])
                                          +ABS(srcp[-srcs+1] - srcp[+srcs+1]) - 1;
                        int temp= filp[0];

#define CHECK(j)\
    {   int score= ABS(srcp[-srcs-1+j] - srcp[+srcs-1-j])\
                 + ABS(srcp[-srcs  +j] - srcp[+srcs  -j])\
                 + ABS(srcp[-srcs+1+j] - srcp[+srcs+1-j]);\
        if(score < spatial_score){\
            spatial_score= score;\
            diff0= filp[-fils+j] - srcp[-srcs+j];\
            diff1= filp[+fils-j] - srcp[+srcs-j];

                        CHECK(-1) CHECK(-2) }} }}
                        CHECK( 1) CHECK( 2) }} }}
#if 0
                        if((diff0 ^ diff1) > 0){
                            int mindiff= ABS(diff0) > ABS(diff1) ? diff1 : diff0;
                            temp-= mindiff;
                        }
#elif 1
                        if(diff0 + diff1 > 0)
                            temp-= (diff0 + diff1 - ABS( ABS(diff0) - ABS(diff1) )/2)/2;
                        else
                            temp-= (diff0 + diff1 + ABS( ABS(diff0) - ABS(diff1) )/2)/2;
#else
                        temp-= (diff0 + diff1)/2;
#endif
#if 1
                        filp[0]=
                        dst[i][x + y*dst_stride[i]]= temp > 255U ? ~(temp>>31) : temp;
#else
                        dst[i][x + y*dst_stride[i]]= filp[0];
                        filp[0]= temp > 255U ? ~(temp>>31) : temp;
#endif
                    }else
                        dst[i][x + y*dst_stride[i]]= p->frame_dec->data[i][x + y*fils];
                }
            }
        }
        for(y=0; y<h; y++){
            if(!((y ^ p->parity) & 1)){
                for(x=0; x<w; x++){
#if 1
                    p->frame_dec->data[i][x + y*fils]=
                    dst[i][x + y*dst_stride[i]]= src[i][x + y*srcs];
#else
                    dst[i][x + y*dst_stride[i]]= p->frame_dec->data[i][x + y*fils];
                    p->frame_dec->data[i][x + y*fils]= src[i][x + y*srcs];
#endif
                }
            }
        }
    }
    p->parity ^= 1;

}

//EOF
