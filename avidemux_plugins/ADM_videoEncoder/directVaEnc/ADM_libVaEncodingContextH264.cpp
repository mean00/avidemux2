/***************************************************************************
                          \fn     libvaEnc_plugin
                          \brief  Plugin to use libva hw encoder (intel mostly)
                             -------------------

    copyright            : (C) 2018 by mean
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
 /***************************************************************************/
/* Derived from libva sample code */
/*
 * Copyright (c) 2007-2013 Intel Corporation. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_bitstream.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_videoInfoExtractor.h"


#include "va/va.h"
#include "va/va_enc_h264.h"
#include "ADM_coreLibVA_buffer.h"
#include "ADM_libVaEncodingContextH264.h"


/**
 * 
 */
ADM_vaEncodingContextH264Base::ADM_vaEncodingContextH264Base()
{
    aprintf("vaH264 ctor\n");
    context_id=VA_INVALID;
    config_id=VA_INVALID;
    
    current_frame_encoding=0;

    for(int i=0;i<VA_ENC_NB_SURFACE;i++)
        vaEncodingBuffers[i]=NULL;;
    for(int i=0;i<VA_ENC_NB_SURFACE;i++)    
    {
        vaSurface[i]=NULL;
        vaRefSurface[i]=NULL;
    }
    memset(&seq_param, 0, sizeof(seq_param));
    memset(&pic_param, 0, sizeof(pic_param));
    memset(&slice_param, 0, sizeof(slice_param));
    
    num_ref_frames = 1;
    
    
    numShortTerm = 0;
    MaxPicOrderCntLsb = (2<<8);
    Log2MaxFrameNum = 16;
    Log2MaxPicOrderCntLsb = 8;


    // RC
    initial_qp = 15;
    minimal_qp = 0;
    rc_mode = VA_RC_CBR; //VA_RC_CQP;
    aprintf("/vaH264 ctor\n");
    tmpBuffer=NULL;
}
/**
 * 
 */
ADM_vaEncodingContextH264Base::~ADM_vaEncodingContextH264Base()
{
    aprintf("vaH264 dtor\n");
    if(context_id!=VA_INVALID)
    {
        vaDestroyContext(admLibVA::getDisplay(),context_id);
        context_id=VA_INVALID;
    }
    if(config_id!=VA_INVALID)
    {
        vaDestroyConfig(admLibVA::getDisplay(),config_id);
        config_id=VA_INVALID;
    }
    for(int i=0;i<VA_ENC_NB_SURFACE;i++)
    {
        if(vaSurface[i])
        {
            delete vaSurface[i];
            vaSurface[i]=NULL;
        }
        if(vaRefSurface[i])
        {
            delete vaRefSurface[i];
            vaRefSurface[i]=NULL;
        }

    }
    aprintf("/vaH264 dtor\n");
}
/**
 * 
 * @param width
 * @param height
 * @param knownSurfaces
 * @return 
 */
bool ADM_vaEncodingContextH264Base::setup( int width, int height, int frameInc,std::vector<ADM_vaSurface *>knownSurfaces)
{
        ADM_info("vaH264 setup\n");
        
        h264=vaGetH264EncoderProfile();
        if(h264->profile==VAProfileNone)
        {
            ADM_error("No H264 encoding support\n");
            return false;
        }
        
        
        VAStatus va_status;
        frame_width=width;
        frame_height=height;
        frame_width_mbaligned=(width+15)&~15;
        frame_height_mbaligned=(height+15)&~15;
        int  i;
        usSecondsToFrac(frameInc,&frameNum,&frameDen);        
        ADM_info("xFps : %d : %d\n",frameNum,frameDen);
        // marshall new config...
        
        // copy common part
        int nAttrib=h264->newAttributes.count();
        int outAttrib=0;
        VAConfigAttrib *ttrib=new VAConfigAttrib[nAttrib+1];
        const VAConfigAttrib *old=h264->newAttributes.getPointer();
        memcpy(ttrib,old,nAttrib*sizeof(VAConfigAttrib));
        
        // add rate control, it is per instance
        ttrib[outAttrib].type=VAConfigAttribRateControl;
        ttrib[outAttrib].value=VA_RC_CBR;
        outAttrib++;
        
        CHECK_VA_STATUS_BOOL( vaCreateConfig(admLibVA::getDisplay(), h264->profile, VAEntrypointEncSlice, ttrib, outAttrib, &config_id));

        int n=knownSurfaces.size();                    
        VASurfaceID *tmp_surfaceId = new VASurfaceID[n];
        for(int i=0;i<n;i++)
        {
            tmp_surfaceId[i]=knownSurfaces[i]->surface;
        }

        /* Create a context for this encode pipe */
        CHECK_VA_STATUS_BOOL( vaCreateContext(admLibVA::getDisplay(), config_id,
                                    frame_width_mbaligned, frame_height_mbaligned,
                                    VA_PROGRESSIVE,
                                    tmp_surfaceId, n,
                                    &context_id));
        
        delete [] ttrib;
        delete [] tmp_surfaceId;
        tmp_surfaceId=NULL;

        int codedbuf_size = (frame_width_mbaligned * frame_height_mbaligned * 400) / (16*16);

        for (i = 0; i < SURFACE_NUM; i++) 
        {
            vaEncodingBuffers[i]= ADM_vaEncodingBuffers::allocate(context_id,codedbuf_size);
            if(!vaEncodingBuffers[i])
            {
                ADM_warning("Cannot create encoding buffer %d\n",i);
                return false;;
            }
        }

        // Allocate VAImage

        for(int i=0;i<VA_ENC_NB_SURFACE;i++)
        {
            vaSurface[i]=ADM_vaSurface::allocateWithSurface(width,height);
            if(!vaSurface[i]) 
            {
                ADM_warning("Cannot allocate surface\n");
                return false;
            }

            vaRefSurface[i]=ADM_vaSurface::allocateWithSurface(width,height);
            if(!vaRefSurface[i]) 
            {
                ADM_warning("Cannot allocate ref surface\n");
                return false;
            }
        }
        tmpBuffer=new uint8_t[codedbuf_size];
        render_sequence();
        ADM_info("/vaH264 setup\n");
        return true;                
}


//-- Global Header

bool ADM_vaEncodingContextH264Base::generateExtraData(int *size, uint8_t **data)
{
    aprintf("vaH264 extraData\n");
    vaBitstream sps,pps;
    
    fillSeqParam();
    sps_rbsp(&sps);
    
    fillPPS(0,FRAME_IDR);
    pps_rbsp(&pps);
    
    sps.stop();
    pps.stop();
    
    int spsLen=sps.lengthInBytes();
    int ppsLen=pps.lengthInBytes();
    
    *data =new uint8_t[spsLen+ppsLen+20];
    
    uint8_t *p=*data;
    *p++=1;
    *p++=sps.getPointer()[0];
    *p++=sps.getPointer()[1];
    *p++=sps.getPointer()[2];
    *p++=0xff;
    // SPS
    *p++=0xe1;
    *p++=(1+spsLen)>>8;
    *p++=(1+spsLen)&0xff;
    *p++=NAL_SPS; // SPS NALU
    memcpy(p,sps.getPointer(),spsLen);
    p+=spsLen;
    // PPS
    *p++=1;
    *p++=(1+ppsLen)>>8;
    *p++=(1+ppsLen)&0xff;
    *p++=NAL_PPS;
    memcpy(p,pps.getPointer(),ppsLen);
    p+=ppsLen;
    *size=(intptr_t)(p)-(intptr_t)*data;
    
    mixDump(*data,*size);
    
    aprintf("/vaH264 extraData\n");
    return true;
}
/**
 * 
 * @param in
 * @param out
 * @return 
 */
bool ADM_vaEncodingContextH264Base::encode(ADMImage *in, ADMBitstream *out)
{
    aprintf("Encoding frame %d, H264 AVC\n",current_frame_encoding);
    vaFrameType current_frame_type;
    if(!vaSurface[current_frame_encoding%SURFACE_NUM]->fromAdmImage(in))
    {
        ADM_warning("Failed to upload image to vaSurface\n");
        return false;
    }

    encoding2display_order(current_frame_encoding, vaH264Settings.IntraPeriod,    &current_frame_type);
    aprintf("Encoding order = %d,  frame type=%d\n",(int)current_frame_encoding,current_frame_type);  
    int current_slot= (current_frame_encoding % SURFACE_NUM);

    CHECK_VA_STATUS_BOOL(vaBeginPicture(admLibVA::getDisplay(), context_id, vaSurface[current_slot]->surface));
    

    
    if (current_frame_type == FRAME_IDR) 
    {
        out->flags = AVI_KEY_FRAME;
    }else
    {
        out->flags = AVI_P_FRAME;
    }
    render_picture(current_frame_encoding,current_frame_type); 
    render_slice(current_frame_encoding,current_frame_type); 
    CHECK_VA_STATUS_BOOL( vaEndPicture(admLibVA::getDisplay(),context_id));
    //--    
    
    CHECK_VA_STATUS_BOOL( vaSyncSurface(admLibVA::getDisplay(), vaSurface[current_frame_encoding % SURFACE_NUM]->surface));
  
    #if 0 // Heavy convert

        int len=vaEncodingBuffers[current_frame_encoding % SURFACE_NUM]->read(tmpBuffer, out->bufferSize);
        int l=ADM_unescapeH264(len-4,tmpBuffer+4,out->data+4);
        out->data[0]=l>>24;
        out->data[1]=l>>16;
        out->data[2]=l>>8;
        out->data[3]=l>>0;
        out->len=l+4;

    #else            // light convert, no escape
        out->len=vaEncodingBuffers[current_frame_encoding % SURFACE_NUM]->read(out->data, out->bufferSize);
        ADM_assert(out->len>=0);

        // Set NAL Size (wtf ?)
        int l=out->len-4;
        out->data[0]=l>>24;
        out->data[1]=l>>16;
        out->data[2]=l>>8;
        out->data[3]=l>>0;
    #endif    
    /* reload a new frame data */

    update_ReferenceFrames(current_frame_type);        
    current_frame_encoding++;
    out->pts=in->Pts;
    out->dts=out->pts;
    return true;
}

// EOF
