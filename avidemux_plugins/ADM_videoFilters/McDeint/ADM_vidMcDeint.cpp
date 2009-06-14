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
#include "ADM_lavcodec.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidMcDeint_param.h"
#include "DIA_factory.h"

static FILTER_PARAM mcDeintParam={3,{"mode","qp","initial_parity"}};

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


class  AVDMVideoMCDeint:public AVDMGenericVideoStream
 {

 protected:
                MCDEINT_PARAM *_param;
                VideoCache      *vidCache;
                uint8_t         init();
                uint8_t         cleanup();
                vf_priv_s       priv;
 public:
    virtual char          *printConf(void) ;
                  AVDMVideoMCDeint(  AVDMGenericVideoStream *in,CONFcouple *setup);
                 ~AVDMVideoMCDeint();
 virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                            ADMImage *data,uint32_t *flags);

  virtual uint8_t configure( AVDMGenericVideoStream *instream);
  virtual uint8_t getCoupledConf( CONFcouple **couples);
 }     ;

//********** Register chunk ************

    

VF_DEFINE_FILTER(AVDMVideoMCDeint,mcDeintParam,
                mcdeinterlace,
                QT_TR_NOOP("mcDeinterlace"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Motion compensation deinterlacer. Ported from MPlayer."));

static void filter(struct vf_priv_s *p, uint8_t *dst[3], uint8_t *src[3], int dst_stride[3], int src_stride[3], int width, int height);


char *AVDMVideoMCDeint::printConf( void )
{
    ADM_FILTER_DECLARE_CONF(" MC deinterlacer : Mode %d, qp %d, parity %d ",_param->mode,_param->qp,_param->initial_parity);
        
}
uint8_t AVDMVideoMCDeint::configure(AVDMGenericVideoStream * instream)
{
  
   diaMenuEntry menuMode[4]={{0,QT_TR_NOOP("Fast"),NULL},
                             {1,QT_TR_NOOP("Medium"),NULL},
                             {2,QT_TR_NOOP("Slow iterative motion search"),NULL},
                             {3,QT_TR_NOOP("Extra slow (same as 3+multiple reference frames)"),NULL}
                          };
   diaMenuEntry menuField[2]={{0,QT_TR_NOOP("Top"),NULL},
                             {1,QT_TR_NOOP("Bottom"),NULL}
                          };
  
    diaElemMenu     menu1(&(_param->mode),QT_TR_NOOP("_Mode:"), 4,menuMode);
    diaElemMenu     menu2(&(_param->initial_parity),QT_TR_NOOP("_Field dominance:"), 2,menuField);
    diaElemUInteger qp(&(_param->qp),QT_TR_NOOP("_Qp:"),1,60);
    
    diaElem *elems[3]={&menu1,&menu2,&qp};
  
    return  diaFactoryRun(QT_TR_NOOP("mcDeinterlace"),3,elems);

}
uint8_t AVDMVideoMCDeint::getCoupledConf( CONFcouple **couples)
{

                        ADM_assert(_param);
                        *couples=new CONFcouple(3);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
                CSET(mode);
                CSET(qp);
                CSET(initial_parity);
                return 1;

}

//_______________________________________________________________
AVDMVideoMCDeint::AVDMVideoMCDeint(AVDMGenericVideoStream *in,CONFcouple *couples)
{

        _in=in;
        memcpy(&_info,_in->getInfo(),sizeof(_info));
        _param=NEW(MCDEINT_PARAM);
        vidCache=new VideoCache(4,_in);
        if(couples)
        {
                GET(mode);
                GET(qp);
                GET(initial_parity);
        }
        else
        {
                _param->mode=0;
                _param->qp=1;
                _param->initial_parity=0;
                
        }
        _info.encoding=1;
        init();
}



// ___ destructor_____________
AVDMVideoMCDeint::~AVDMVideoMCDeint()
{
      cleanup();
      delete  vidCache;
      delete _param;
      _param=NULL;
      vidCache=NULL;
}
uint8_t AVDMVideoMCDeint::getFrameNumberNoAlloc(uint32_t frame,
                                uint32_t *len,
                                ADMImage *data,
                                uint32_t *flags)
{
                        if(frame>=_info.nb_frames) 
                        {
                                printf("MPdelogo : Filter : out of bound!\n");
                                return 0;
                        }
        
                        ADM_assert(_param);

ADMImage *curImage;
char txt[256];
                        curImage=vidCache->getImage(frame);
                        if(!curImage)
                        {
                                printf("MCDeint : error getting frame\n");
                                return 0;
                        }
              
                      // Prepare to call filter...
                  uint8_t *dplanes[3],*splanes[3];
                  int dstride[3],sstride[3];

                  dstride[0]=sstride[0]=_info.width;
                  dstride[2]=sstride[2]=dstride[1]=sstride[1]=_info.width>>1;

                  splanes[0]=YPLANE(curImage);
                  splanes[1]=UPLANE(curImage);
                  splanes[2]=VPLANE(curImage);

                  dplanes[0]=YPLANE(data);
                  dplanes[1]=UPLANE(data);
                  dplanes[2]=VPLANE(data);


                  filter(&priv, dplanes, splanes, dstride, sstride, _info.width, _info.height);
                  vidCache->unlockAll();
        return 1;
}
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
            avctx_enc->width = _info.width;
            avctx_enc->height = _info.height;
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

            switch(_param->mode)
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
        priv.frame= avcodec_alloc_frame();

        priv.outbuf_size= _info.width*_info.height*10;
        priv.outbuf= (uint8_t *)ADM_alloc(priv.outbuf_size);
        priv.parity=_param->initial_parity;
  return 1;
}
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
#ifdef titititititi_II
static int config(struct vf_instance_s* vf,
        int width, int height, int d_width, int d_height,
	unsigned int flags, unsigned int outfmt){
        int i;
        AVCodec *enc= avcodec_find_encoder(CODEC_ID_SNOW);

        for(i=0; i<3; i++){
            AVCodecContext *avctx_enc;
#if 0
            int is_chroma= !!i;
            int w= ((width  + 31) & (~31))>>is_chroma;
            int h= ((height + 31) & (~31))>>is_chroma;

            vf->priv->temp_stride[i]= w;
            vf->priv->temp[i]= malloc(vf->priv->temp_stride[i]*h*sizeof(int16_t));
            vf->priv->src [i]= malloc(vf->priv->temp_stride[i]*h*sizeof(uint8_t));
#endif
            avctx_enc=
            vf->priv->avctx_enc= avcodec_alloc_context();
            avctx_enc->width = width;
            avctx_enc->height = height;
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

            switch(vf->priv->mode){
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
        vf->priv->frame= avcodec_alloc_frame();

        vf->priv->outbuf_size= width*height*10;
        vf->priv->outbuf= malloc(vf->priv->outbuf_size);

	return vf_next_config(vf,width,height,d_width,d_height,flags,outfmt);
}

static void get_image(struct vf_instance_s* vf, mp_image_t *mpi){
    if(mpi->flags&MP_IMGFLAG_PRESERVE) return; // don't change
return; //caused problems, dunno why
    // ok, we can do pp in-place (or pp disabled):
    vf->dmpi=vf_get_image(vf->next,mpi->imgfmt,
        mpi->type, mpi->flags | MP_IMGFLAG_READABLE, mpi->width, mpi->height);
    mpi->planes[0]=vf->dmpi->planes[0];
    mpi->stride[0]=vf->dmpi->stride[0];
    mpi->width=vf->dmpi->width;
    if(mpi->flags&MP_IMGFLAG_PLANAR){
        mpi->planes[1]=vf->dmpi->planes[1];
        mpi->planes[2]=vf->dmpi->planes[2];
	mpi->stride[1]=vf->dmpi->stride[1];
	mpi->stride[2]=vf->dmpi->stride[2];
    }
    mpi->flags|=MP_IMGFLAG_DIRECT;
}

static int put_image(struct vf_instance_s* vf, mp_image_t *mpi, double pts){
    mp_image_t *dmpi;

    if(!(mpi->flags&MP_IMGFLAG_DIRECT)){
        // no DR, so get a new image! hope we'll get DR buffer:
        dmpi=vf_get_image(vf->next,mpi->imgfmt,
            MP_IMGTYPE_TEMP,
            MP_IMGFLAG_ACCEPT_STRIDE|MP_IMGFLAG_PREFER_ALIGNED_STRIDE,
            mpi->width,mpi->height);
        vf_clone_mpi_attributes(dmpi, mpi);
    }else{
        dmpi=vf->dmpi;
    }

    filter(vf->priv, dmpi->planes, mpi->planes, dmpi->stride, mpi->stride, mpi->w, mpi->h);

    return vf_next_put_image(vf,dmpi, pts);
}

static void uninit(struct vf_instance_s* vf){
    if(!vf->priv) return;

#if 0
    for(i=0; i<3; i++){
        if(vf->priv->temp[i]) free(vf->priv->temp[i]);
        vf->priv->temp[i]= NULL;
        if(vf->priv->src[i]) free(vf->priv->src[i]);
        vf->priv->src[i]= NULL;
    }
#endif
    av_freep(&vf->priv->avctx_enc);

    free(vf->priv->outbuf);
    free(vf->priv);
    vf->priv=NULL;
}

#endif
//EOF
