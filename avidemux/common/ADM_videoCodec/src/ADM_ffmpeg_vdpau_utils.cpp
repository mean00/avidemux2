/***************************************************************************
            \file              ADM_ffmpeg_vdpau.cpp  
            \brief Decoder using half ffmpeg/half VDPAU

    The ffmpeg part is to preformat inputs for VDPAU
    VDPAU is loaded dynamically to be able to make a binary
        and have something working even if the target machine
        does not have vdpau
    Some part, especially get/buffer and ip_age borrowed from xbmc
        as the api from ffmpeg is far from clear....


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/




/**
    \fn vdpauUsable
    \brief Return true if  vdpau can be used...
*/
bool vdpauUsable(void)
{
    bool v=false;
    if(!vdpauWorking) return false;
    if(!prefs->get(FEATURES_VDPAU,&v)) v=false;
    return v;
}
/**
    \fn vdpauProbe
    \brief Try loading vdpau...
*/
bool vdpauProbe(void)
{
    GUI_WindowInfo xinfo;
    void *draw;
    draw=UI_getDrawWidget();
    UI_getWindowInfo(draw,&xinfo );
#ifdef USE_VDPAU
    if( admCoreCodecSupports(ADM_CORE_CODEC_FEATURE_VDPAU)==false)
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","Core has been compiled without VDPAU support, but the application has been compiled with it.\nInstallation mismatch"));
        vdpauWorking=false;
    }
#endif
    if(false==admVdpau::init(&xinfo)) return false;
    vdpauWorking=true;
    return true;
}
/**
    \fn vdpauCleanup
*/
bool vdpauCleanup(void)
{
   return admVdpau::cleanup();
}
/**
    \fn ADM_VDPAUgetBuffer
    \brief trampoline to get a VDPAU surface
*/
int ADM_VDPAUgetBuffer(AVCodecContext *avctx, AVFrame *pic,int flags)
{
    decoderFF *ff=(decoderFF *)avctx->opaque;
    decoderFFVDPAU *dec=(decoderFFVDPAU *)ff->getHwDecoder();
    ADM_assert(dec);
    return dec->getBuffer(avctx,pic);
}

/**
    \fn ADM_VDPAUreleaseBuffer
 *  \param opaque is decoderFFVDPAU
*   \param data   is surface
 * 
*/
 void ADM_VDPAUreleaseBuffer(void *opaque, uint8_t *data)
{
    
    decoderFFVDPAU *dec=(decoderFFVDPAU *)opaque;
    ADM_assert(dec);
    struct vdpau_render_state *rdr=( struct vdpau_render_state  *)data;
    dec->releaseBuffer(rdr);
}


    
    
    /**
     * \fn vdpGetProcAddressWrapper
     * @param device
     * @param function_id
     * @param function_pointer
     * @return 
     */
 extern "C"
 {
static VdpStatus vdpGetProcAddressWrapper(    VdpDevice device,    VdpFuncId function_id,       void * *  function_pointer)
{
    ADM_info("Calling vdpGetProcAddressWrapper for function %d\n",function_id);
    VdpStatus r=VDP_STATUS_ERROR;
    
#define ADD_WRAPPER(x,y)     case VDP_FUNC_ID_##x: ADM_info("Wrapping "#x" "#y"\n"); *function_pointer= (void *)admVdpau::y;\
                                        r=VDP_STATUS_OK;\
                                        break;        
    
    switch(function_id)
    {
        ADD_WRAPPER(DECODER_CREATE,decoderCreate)
        ADD_WRAPPER(DECODER_RENDER,decoderRender)
        ADD_WRAPPER(DECODER_DESTROY,decoderDestroy)
                
        case VDP_FUNC_ID_VIDEO_SURFACE_QUERY_CAPABILITIES:
        case VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES:            
        default:
            VdpGetProcAddress *p=admVdpau::getProcAddress2();
            ADM_assert(p);
            r= p(device,function_id,function_pointer);
    }
    if(r==VDP_STATUS_OK)
    {
        ADM_info("Ok\n");
    }else
    {
        ADM_warning("Failed with er=%d\n",(int)r);
    }
    return r;
    
}
}


// EOF
