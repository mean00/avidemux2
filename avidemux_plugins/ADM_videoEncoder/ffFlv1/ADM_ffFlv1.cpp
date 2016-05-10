/***************************************************************************
                          \fn ADM_ffFlv1
                          \brief Front end for libavcodec Flv1 encoder
                             -------------------
    
    copyright            : (C) 2002/2009 by mean
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
#include "ADM_ffFlv1.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

FFcodecSettings Flv1Settings = FLV1_CONF_DEFAULT;

/**
        \fn ADM_ffFlv1Encoder
*/
ADM_ffFlv1Encoder::ADM_ffFlv1Encoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src,&Flv1Settings,false)
{
    printf("[ffFlv1] Creating.\n");
   

}

/**
    \fn pre-open
*/
bool ADM_ffFlv1Encoder::configureContext(void)
{
    switch(Settings.params.mode)
    {
      case COMPRESS_2PASS:
      case COMPRESS_2PASS_BITRATE:
           if(false==setupPass())
            {
                printf("[ffFlv1] Multipass setup failed\n");
                return false;
            }
            break;
      case COMPRESS_CQ:
            _context->flags |= CODEC_FLAG_QSCALE;
            _context->bit_rate = 0;
            break;
      case COMPRESS_CBR:
              _context->bit_rate=Settings.params.bitrate*1000; // kb->b;
            break;
     default:
            return false;
    }
    presetContext(&Settings);
    
    return true;
}

/**
    \fn setup
*/
bool ADM_ffFlv1Encoder::setup(void)
{
    if(false== ADM_coreVideoEncoderFFmpeg::setup(AV_CODEC_ID_FLV1))
        return false;

    printf("[ffFlv1] Setup ok\n");
    return true;
}


/** 
    \fn ~ADM_ffFlv1Encoder
*/
ADM_ffFlv1Encoder::~ADM_ffFlv1Encoder()
{
    printf("[ffFlv1] Destroying.\n");
   
    
}

/**
    \fn encode
*/
bool         ADM_ffFlv1Encoder::encode (ADMBitstream * out)
{
int sz,q;
again:
    sz=0;
    if(false==preEncode()) // Pop - out the frames stored in the queue due to B-frames
    {
        return false;
    }
    q=image->_Qp;
    
    if(!q) q=2;
    switch(Settings.params.mode)
    {
      case COMPRESS_2PASS:
      case COMPRESS_2PASS_BITRATE:
            switch(pass)
            {
                case 1: 
                        break;
                case 2: 
                        break; // Get Qz for this frame...
            }
      case COMPRESS_CQ:
            _frame->quality = (int) floor (FF_QP2LAMBDA * Settings.params.qz+ 0.5);
            break;
      case COMPRESS_CBR:
            break;
     default:
            printf("[ffFlv1] Unsupported encoding mode\n");
            return false;
    }
    aprintf("[CODEC] Flags = 0x%x, QSCALE=%x, bit_rate=%d, quality=%d qz=%d incoming qz=%d\n",_context->flags,CODEC_FLAG_QSCALE,
                                     _context->bit_rate,  _frame->quality, _frame->quality/ FF_QP2LAMBDA,q);     
    
    _frame->reordered_opaque=image->Pts;
    
    AVPacket pkt;
    pkt.data=out->data;
    pkt.size=out->bufferSize;
    
    int gotData;
    int r= avcodec_encode_video2 (_context,&pkt,_frame, &gotData);
    if(r<0)
    {
        ADM_warning("[ffFlv1] Error %d encoding video\n",r);
        return false;
    }
    if(!gotData)
    {
        ADM_warning("[ffFlv1] Encoder produced no data\n");
        pkt.size=0;
    }
    postEncode(out,pkt.size);
    return true;
}

/**
    \fn isDualPass

*/
bool         ADM_ffFlv1Encoder::isDualPass(void) 
{
    if(Settings.params.mode==COMPRESS_2PASS || Settings.params.mode==COMPRESS_2PASS_BITRATE ) return true;
    return false;

}

/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/

bool         ffFlv1Configure(void)
{         
diaMenuEntry meE[]={
  {1,QT_TRANSLATE_NOOP("flv1","None")},
  {2,QT_TRANSLATE_NOOP("flv1","Full")},
  {3,QT_TRANSLATE_NOOP("flv1","Log")},
  {4,QT_TRANSLATE_NOOP("flv1","Phods")},
  {5,QT_TRANSLATE_NOOP("flv1","EPZS")},
  {6,QT_TRANSLATE_NOOP("flv1","X1")}
};       

diaMenuEntry qzE[]={
  {0,QT_TRANSLATE_NOOP("flv1","H.263")},
  {1,QT_TRANSLATE_NOOP("flv1","MPEG")}
};       

diaMenuEntry rdE[]={
  {0,QT_TRANSLATE_NOOP("flv1","MB comparison")},
  {1,QT_TRANSLATE_NOOP("flv1","Fewest bits (vhq)")},
  {2,QT_TRANSLATE_NOOP("flv1","Rate distortion")}
};     

        FFcodecSettings *conf=&Flv1Settings;

uint32_t me=(uint32_t)conf->lavcSettings.me_method;  
#define PX(x) &(conf->lavcSettings.x)

         diaElemBitrate   bitrate(&(Flv1Settings.params),NULL);
         diaElemUInteger  qminM(PX(qmin),QT_TRANSLATE_NOOP("flv1","Mi_n. quantizer:"),1,31);
         diaElemUInteger  qmaxM(PX(qmax),QT_TRANSLATE_NOOP("flv1","Ma_x. quantizer:"),1,31);
         diaElemUInteger  qdiffM(PX(max_qdiff),QT_TRANSLATE_NOOP("flv1","Max. quantizer _difference:"),1,31);
         diaElemToggle    trellis(PX(_TRELLIS_QUANT),QT_TRANSLATE_NOOP("flv1","_Trellis quantization"));
         diaElemUInteger filetol(PX(vratetol),QT_TRANSLATE_NOOP("flv1","_Filesize tolerance (kb):"),0,100000);
         
         diaElemFloat    qzComp(PX(qcompress),QT_TRANSLATE_NOOP("flv1","_Quantizer compression:"),0,1);
         diaElemFloat    qzBlur(PX(qblur),QT_TRANSLATE_NOOP("flv1","Quantizer _blur:"),0,1);
         
        diaElemUInteger GopSize(PX(gop_size),QT_TRANSLATE_NOOP("flv1","_Gop Size:"),1,500); 
          /* First Tab : encoding mode */
        diaElem *diamode[]={&GopSize,&bitrate};
        diaElemTabs tabMode(QT_TRANSLATE_NOOP("flv1","User Interface"),2,diamode);
        
        /* 3nd Tab : Qz */
        
         diaElem *diaQze[]={&qminM,&qmaxM,&qdiffM,&trellis};
        diaElemTabs tabQz(QT_TRANSLATE_NOOP("flv1","Quantization"),4,diaQze);
        
        /* 4th Tab : RControl */
        
         diaElem *diaRC[]={&filetol,&qzComp,&qzBlur};
        diaElemTabs tabRC(QT_TRANSLATE_NOOP("flv1","Rate Control"),3,diaRC);
        
         diaElemTabs *tabs[]={&tabMode,&tabQz,&tabRC};
        if( diaFactoryRunTabs(QT_TRANSLATE_NOOP("flv1","libavcodec FLV1 configuration"),3,tabs))
        {
          conf->lavcSettings.me_method=(Motion_Est_ID)me;
          return true;
        }
         return false;
}
// EOF
