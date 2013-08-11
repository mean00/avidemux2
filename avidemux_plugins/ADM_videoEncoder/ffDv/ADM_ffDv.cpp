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
#include "ADM_ffDv.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif


/**
    \fn setup
*/
bool ADM_ffDvEncoder::setup(void)
{
    
   if(false== ADM_coreVideoEncoderFFmpeg::setup(CODEC_ID_DVVIDEO))
        return false;
   return true;
}
/**
        \fn ADM_ffFlv1Encoder
*/
ADM_ffDvEncoder::ADM_ffDvEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src,NULL,false)
{
    printf("[DV] Creating.\n");
   

}


/** 
    \fn ~ADM_ffFlv1Encoder
*/
ADM_ffDvEncoder::~ADM_ffDvEncoder()
{
    printf("[Dv] Destroying.\n");
   
    
}

/**
    \fn encode
*/
bool         ADM_ffDvEncoder::encode (ADMBitstream * out)
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
   
    
    _frame.reordered_opaque=image->Pts;
    if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, &_frame)) <= 0)
    {
        printf("[Dv] Error %d encoding video\n",sz);
        return false;
    }
    postEncode(out,sz);
    return true;
}

// EOF
