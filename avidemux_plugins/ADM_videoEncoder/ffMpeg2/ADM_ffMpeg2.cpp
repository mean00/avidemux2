/***************************************************************************
                          \fn ADM_ffMpeg2
                          \brief Front end for libavcodec Mpeg2 asp encoder
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
#include "ADM_ffMpeg2.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif
#include "mpegMatrix.h"
mpeg2_encoder Mp2Settings = MPEG2_CONF_DEFAULT;

/**
        \fn ADM_ffMpeg2Encoder
*/
// It works because mpeg2_encoder.h is the same as FFCodecSettings + additional fields!
ADM_ffMpeg2Encoder::ADM_ffMpeg2Encoder(ADM_coreVideoFilter *src,bool globalHeader) : 
        ADM_coreVideoEncoderFFmpeg(src,(FFcodecSettings *)&(Mp2Settings),false)
{
    printf("[ffMpeg2Encoder] Creating.\n");
   

}

/**
    \fn setup
*/
bool ADM_ffMpeg2Encoder::setup(void)
{
    
    switch(Settings.params.mode)
    {
      case COMPRESS_2PASS:
      case COMPRESS_2PASS_BITRATE:
           if(false==setupPass())
            {
                printf("[ffmpeg] Multipass setup failed\n");
                return false;
            }
            break;
      case COMPRESS_SAME:
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
    // Override some parameters specific to this codec
    // Set matrix if any...
#define MX(a,b,c) case a: _context->intra_matrix=b,_context->inter_matrix=c;break;
    switch(Mp2Settings.matrix)
    {
        MX(MPEG2_MATRIX_DEFAULT,NULL,NULL);
        MX(MPEG2_MATRIX_TMPGENC,tmpgenc_intra,tmpgenc_inter);
        MX(MPEG2_MATRIX_ANIME,anime_intra,anime_inter);
        MX(MPEG2_MATRIX_KVCD,kvcd_intra,kvcd_inter);
        default:
                ADM_error("unknown matrix type : %d\n",(int)Mp2Settings.matrix);
                ADM_assert(0);
                break;
    }
    _context->rc_buffer_size=Mp2Settings.lavcSettings.bufferSize*8*1024;
    _context->rc_buffer_size_header=Mp2Settings.lavcSettings.bufferSize*8*1024;
    _context->rc_initial_buffer_occupancy=_context->rc_buffer_size;
    _context->rc_max_rate=Mp2Settings.lavcSettings.maxBitrate*1000;
    _context->rc_max_rate_header=Mp2Settings.lavcSettings.maxBitrate*1000;
    // /Override some parameters specific to this codec

    if(false== ADM_coreVideoEncoderFFmpeg::setup(CODEC_ID_MPEG2VIDEO))
        return false;
    printf("[ffMpeg] Setup ok\n");
    return true;
}


/** 
    \fn ~ADM_ffMpeg2Encoder
*/
ADM_ffMpeg2Encoder::~ADM_ffMpeg2Encoder()
{
    printf("[ffMpeg2Encoder] Destroying.\n");
   
    
}

/**
    \fn encode
*/
bool         ADM_ffMpeg2Encoder::encode (ADMBitstream * out)
{
int sz,q;
again:
    sz=0;
    if(false==preEncode()) // Pop - out the frames stored in the queue due to B-frames
    {
        if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, NULL)) <= 0)
        {
            printf("[ffMpeg2] Error %d encoding video\n",sz);
            return false;
        }
        printf("[ffMpeg2] Popping delayed bframes (%d)\n",sz);
        goto link;
        return false;
    }
    q=image->_Qp;
    
    if(!q) q=2;
    switch(Settings.params.mode)
    {
      case COMPRESS_SAME:
                // Keep same frame type & same Qz as the incoming frame...
            _frame.quality = (int) floor (FF_QP2LAMBDA * q+ 0.5);

            if(image->flags & AVI_KEY_FRAME)    _frame.pict_type=FF_I_TYPE;
            else if(image->flags & AVI_B_FRAME) _frame.pict_type=FF_B_TYPE;
            else                                _frame.pict_type=FF_P_TYPE;

            break;
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
            _frame.quality = (int) floor (FF_QP2LAMBDA * Settings.params.qz+ 0.5);
            break;
      case COMPRESS_CBR:
            break;
     default:
            printf("[ffMpeg2] Unsupported encoding mode\n");
            return false;
    }
    aprintf("[CODEC] Flags = 0x%x, QSCALE=%x, bit_rate=%d, quality=%d qz=%d incoming qz=%d\n",_context->flags,CODEC_FLAG_QSCALE,
                                     _context->bit_rate,  _frame.quality, _frame.quality/ FF_QP2LAMBDA,q);     
    
    _frame.reordered_opaque=image->Pts;
    if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, &_frame)) < 0)
    {
        printf("[ffMpeg2] Error %d encoding video\n",sz);
        return false;
    }
    
    if(sz==0) // no pic, probably pre filling, try again
        goto again;
link:
    postEncode(out,sz);
   
    return true;
}

/**
    \fn isDualPass

*/
bool         ADM_ffMpeg2Encoder::isDualPass(void) 
{
    if(Settings.params.mode==COMPRESS_2PASS || Settings.params.mode==COMPRESS_2PASS_BITRATE ) return true;
    return false;

}

/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/

bool         ffMpeg2Configure(void)
{   

diaMenuEntry  arE[]=
{
    {0,QT_TR_NOOP("Normal (4:3)")},
    {1,QT_TR_NOOP("Wide (16:9)")}
};
      
diaMenuEntry  matrixE[]=
{
    {MPEG2_MATRIX_DEFAULT,QT_TR_NOOP("Default")},
    {MPEG2_MATRIX_TMPGENC,QT_TR_NOOP("Tmpgenc")},
    {MPEG2_MATRIX_ANIME,QT_TR_NOOP("Animes")},
    {MPEG2_MATRIX_KVCD,QT_TR_NOOP("KVCD")},
};
      
diaMenuEntry rdE[]={
  {0,QT_TR_NOOP("MB comparison")},
  {1,QT_TR_NOOP("Fewest bits (vhq)")},
  {2,QT_TR_NOOP("Rate distortion")}
};     
diaMenuEntry threads[]={
  {0,QT_TR_NOOP("One thread")},
  {2,QT_TR_NOOP("Two threads)")},
  {3,QT_TR_NOOP("Three threads")},
  {99,QT_TR_NOOP("Auto (#cpu)")}
};     
   
diaMenuEntry interE[]={
  {0,QT_TR_NOOP("Progressive")},
  {1,QT_TR_NOOP("Interlaced")},
};     
diaMenuEntry foE[]={
  {0,QT_TR_NOOP("Top Field First")},
  {1,QT_TR_NOOP("Bottom Field First")},
};     

        mpeg2_encoder *conf=&Mp2Settings;

uint32_t me=(uint32_t)conf->lavcSettings.me_method;  
#define PX(x) &(conf->lavcSettings.x)

         diaElemBitrate   bitrate(&(Mp2Settings.params),NULL);

         diaElemMenu      threadM(PX(MultiThreaded),QT_TR_NOOP("Threading"),4,threads);
         diaElemUInteger  qminM(PX(qmin),QT_TR_NOOP("Mi_n. quantizer:"),1,31);
         diaElemUInteger  qmaxM(PX(qmax),QT_TR_NOOP("Ma_x. quantizer:"),1,31);
         diaElemUInteger  qdiffM(PX(max_qdiff),QT_TR_NOOP("Max. quantizer _difference:"),1,31);
         diaElemUInteger  bufferS(PX(bufferSize),QT_TR_NOOP("VBV Buffer Size:"),1,1024);
         diaElemUInteger  maxBitrate(PX(maxBitrate),QT_TR_NOOP("Max bitrate (kb/s):"),1,50000);
         
         diaElemToggle    trellis(PX(_TRELLIS_QUANT),QT_TR_NOOP("_Trellis quantization"));
         
         diaElemUInteger  max_b_frames(PX(max_b_frames),QT_TR_NOOP("_Number of B frames:"),0,32);
         uint32_t widescreen= conf->lavcSettings.widescreen;
         uint32_t iinterlaced= conf->lavcSettings.interlaced;
         uint32_t bff= conf->lavcSettings.bff;
         diaElemMenu     rdM(PX(mb_eval),QT_TR_NOOP("_Macroblock decision:"),3,rdE);
         diaElemMenu     arM(&(widescreen),QT_TR_NOOP("Aspect ratio:"),2,arE);
         diaElemMenu     matrixM(&(Mp2Settings.matrix),QT_TR_NOOP("Matrices:"),MPEG2_MATRIX_LAST,matrixE);
         diaElemUInteger filetol(PX(vratetol),QT_TR_NOOP("_Filesize tolerance (kb):"),0,100000);
         
         diaElemFloat    qzComp(PX(qcompress),QT_TR_NOOP("_Quantizer compression:"),0,1);
         diaElemFloat    qzBlur(PX(qblur),QT_TR_NOOP("Quantizer _blur:"),0,1);
         
        diaElemUInteger GopSize(PX(gop_size),QT_TR_NOOP("_Gop Size:"),1,30); 

        diaElemMenu     interlaced(&(iinterlaced),QT_TR_NOOP("_Interlaced:"),2,interE);
        diaElemMenu     fieldOrder(&(bff),QT_TR_NOOP("Field Order:"),2,foE);

          /* First Tab : encoding mode */
        diaElem *diamode[]={&arM,&threadM,&bitrate};
        diaElemTabs tabMode(QT_TR_NOOP("Basic Settings"),3,diamode);
        
        /* 2nd Tab : advanced*/
        diaElem *diaAdv[]={&bufferS,&matrixM,&max_b_frames,&GopSize,&maxBitrate};
        diaElemTabs tabAdv(QT_TR_NOOP("Adv. Settings"),5,diaAdv);

        /* 2ndb Tab : interlacing*/
        diaElem *diaInter[]={&interlaced,&fieldOrder};
        diaElemTabs tabInter(QT_TR_NOOP("Interlacing"),2,diaInter);

        /* 3nd Tab : Qz */
        
        diaElem *diaQze[]={&rdM,&qminM,&qmaxM,&qdiffM,&trellis};
        diaElemTabs tabQz(QT_TR_NOOP("Quantization"),5,diaQze);
        
        /* 4th Tab : RControl */
        
         diaElem *diaRC[]={&filetol,&qzComp,&qzBlur};
        diaElemTabs tabRC(QT_TR_NOOP("Rate Control"),3,diaRC);
        
         diaElemTabs *tabs[]={&tabMode,&tabAdv,&tabInter,&tabQz,&tabRC};
        if( diaFactoryRunTabs(QT_TR_NOOP("libavcodec MPEG-2 configuration"),5,tabs))
        {
          conf->lavcSettings.me_method=(Motion_Est_ID)me;
          conf->lavcSettings.widescreen= widescreen;
          conf->lavcSettings.interlaced= iinterlaced;
          conf->lavcSettings.bff= bff;
          return true;
        }
         return false;
}
// EOF
