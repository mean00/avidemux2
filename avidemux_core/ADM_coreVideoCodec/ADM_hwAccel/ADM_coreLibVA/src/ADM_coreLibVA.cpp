/***************************************************************************
    \file             : ADM_coreXvba.cpp
    \brief            : Wrapper around xvba functions
    \author           : (C) 2013 by mean fixounet@free.fr, derived from xbmc_pvr
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
#include "../include/ADM_coreLibVA.h"
#include "va/va_x11.h"

#ifdef USE_XVBA
#include "../include/ADM_coreLibVA_internal.h"
#include "ADM_dynamicLoading.h"
#include "ADM_windowInfo.h"

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

/**
 * 
 */
 
GUI_WindowInfo      admLibVA::myWindowInfo;

namespace ADM_coreLibVA
{
 void                   *context;
 VADisplay              display;
 namespace decoders
 {
        bool            h264; 
 }
}

static bool                  coreLibVAWorking=false;

#define CLEAR(x) memset(&x,0,sizeof(x))
#define CHECK_ERROR(x) {xError=x;displayXError(#x,ADM_coreLibVA::display,xError);}

/**
 * \fn displayXError
 * @param dis
 * @param er
 */
static void displayXError(const char *func,const VADisplay dis,const VAStatus er)
{
    if(!er) return;    
    ADM_warning("LibVA Error : <%s:%s>\n",func,vaErrorStr((er)));

}

/**
    \fn     init
    \brief
*/
bool admLibVA::init(GUI_WindowInfo *x)
{
    Display *dis=(Display *)x->display;
    ADM_coreLibVA::display=vaGetDisplay(dis);
    int maj=0,min=0,patch=0;
    ADM_info("[LIBVA] Initializing LibVA library ...\n");

    ADM_coreLibVA::context=NULL;
    ADM_coreLibVA::decoders::h264=false;
    
            
    myWindowInfo=*x;
    VAStatus xError;
    int majv,minv;
    CHECK_ERROR(vaInitialize(ADM_coreLibVA::display,&majv,&minv));
    if(xError)
    {
        ADM_warning("VA: init failed\n");
        return false;
    }
    ADM_info("VA %d.%d, Vendor = %s\n",majv,minv,vaQueryVendorString(ADM_coreLibVA::display));
    
    coreLibVAWorking=true;

    
    ADM_info("[LIBVA] VA  init ok.\n");
    return true;
}
/**
    \fn cleanup
*/
bool admLibVA::cleanup(void)
{
    VAStatus xError;
    ADM_info("[LIBVA] De-Initializing LibVA library ...\n");
    if(true==coreLibVAWorking)
    {
         CHECK_ERROR(vaTerminate(ADM_coreLibVA::display));   
          
    }
    coreLibVAWorking=false;
    return true;
}
/**
    \fn isOperationnal
*/
bool admLibVA::isOperationnal(void)
{
    return coreLibVAWorking;
}

#define CHECK_WORKING(x)   if(!coreLibVAWorking) {ADM_warning("Libva not operationnal\n");return x;}

/**
 * \fn createDecoder
 * @param width
 * @param height
 * @return 
 */

VAContextID        admLibVA::createDecoder(int width, int height)
{
    CHECK_WORKING(VA_INVALID);
#if 0
    int xError;
     CHECK_ERROR(vaCreateContext (  ADM_coreLibVA::display,
    VAConfigID config_id,
    w,
    H,
    int flag,
    VASurfaceID *render_targets,
    int num_render_targets,
    VAContextID *context		/* out */
    );
#endif


    return 0;
}

/**
 * \fn destroySession
 */
bool admLibVA::destroyDecoder(VAContextID session)
{
       
       int xError;
       CHECK_WORKING(false);
       
       aprintf("Destroying decoder %x\n",session);
        CHECK_ERROR(vaDestroyContext(ADM_coreLibVA::display,session));
        if(!xError)
        {
            aprintf("Decoder destroyed\n");
            return true;
        }
        aprintf("Error creating surface\n");
        return false;
}

/**
 * \fn allocateSurface
 * @param w
 * @param h
 * @return 
 */
VASurfaceID        admLibVA::allocateSurface(void *session,int w, int h)
{
       int xError;
       CHECK_WORKING(VA_INVALID);
       
       aprintf("Creating surface %d x %d\n",w,h);
       VASurfaceID s;
        CHECK_ERROR(vaCreateSurfaces(ADM_coreLibVA::display,w,h,VA_RT_FORMAT_YUV420,1,&s));
        if(!xError)
        {
            return s;
        }
        aprintf("Error creating surface\n");
        return VA_INVALID;
    
}
/**
 * \fn destroySurface
 * @param session
 * @param surface
 */
void        admLibVA::destroySurface(void *session, VASurfaceID surface)
{
      int xError;
      CHECK_WORKING();
      CHECK_ERROR(vaDestroySurfaces(ADM_coreLibVA::display,&surface,1));
        if(!xError)
        {
            return;
        }
        aprintf("Error destroying surface\n");
        return;
}
#if 0
/**
 * \fn destroySurface
 * @param session
 * @param surface
 */
XVBABufferDescriptor        *admLibVA::createDecodeBuffer(void *session,XVBA_BUFFER type)
{
      CHECK_WORKING(NULL);
      int xError;
      XVBA_Create_DecodeBuff_Input  in;
      XVBA_Create_DecodeBuff_Output out;
      PREPARE_SESSION_IN(session,in);
      PREPARE_OUT(out);
      in.buffer_type=type;
      in.num_of_buffers=1;
      
      CHECK_ERROR(ADM_coreLibVA::funcs.createDecodeBuffer(&in,&out));
        if(Success==xError && out.num_of_buffers_in_list==1)
        {
            return (XVBABufferDescriptor *)out.buffer_list;
        }
        aprintf("Error creating decode buffer of type %d\n",type);
        return NULL;
}

/**
 * \fn destroySurface
 * @param session
 * @param surface
 */
void        admLibVA::destroyDecodeBuffer(void *session,XVBABufferDescriptor *decodeBuffer)
{
        int xError;
        XVBA_Destroy_Decode_Buffers_Input in;
        CHECK_WORKING();
        PREPARE_SESSION_IN(session,in);
        in.num_of_buffers_in_list=1;
        in.buffer_list=decodeBuffer;
        CHECK_ERROR(ADM_coreLibVA::funcs.destroyDecodeBuffer(&in));
        if(Success==xError)
        {
            return;
        }
        aprintf("Error destroying decode buffer\n");
        return;
}
/**
 * 
 * @param session
 * @param surface
 * @return 
 */
bool        admLibVA::decodeStart(void *session, void *surface)
{
      int xError;
      CHECK_WORKING(false);
      XVBA_Decode_Picture_Start_Input in;
      PREPARE_SESSION_IN(session,in);

      in.target_surface=surface;
      
       CHECK_ERROR(ADM_coreLibVA::funcs.startDecodePicture(&in));
      
      if(xError!=Success)
      {
          ADM_info("decodeStart failed\n");
          return false;
      }
      return true;
}
/**
 * \fn decode
 * @param session
 * @param x
 * @return 
 */
bool        admLibVA::decode1(void *session,void *picture_desc,void *matrix_desc)
{
      int xError;
      CHECK_WORKING(false);
      XVBA_Decode_Picture_Input in;
      PREPARE_SESSION_IN(session,in);
      XVBABufferDescriptor *desc[3];
      in.buffer_list=desc;
      in.num_of_buffers_in_list=2;
      
      
      desc[0]=(XVBABufferDescriptor *)picture_desc;
      desc[1]=(XVBABufferDescriptor *)matrix_desc;
      
      
      CHECK_ERROR(ADM_coreLibVA::funcs.decodePicture(&in));
      if(Success!=xError)
      {
          return false;
      }
      return true;
      
}

/**
 * \fn decode
 * @param session
 * @param x
 * @return 
 */
bool        admLibVA::decode2(void *session,void *data,void *ctrl)
{
      int xError;
      CHECK_WORKING(false);
      XVBA_Decode_Picture_Input in;
      PREPARE_SESSION_IN(session,in);
      XVBABufferDescriptor *desc[3];
      in.buffer_list=desc;
      in.num_of_buffers_in_list=2;
      
      
      desc[0]=(XVBABufferDescriptor *)data;
      desc[1]=(XVBABufferDescriptor *)ctrl;
      
      desc[0]->data_offset=0;
      desc[1]->data_size_in_buffer=sizeof(XVBADataCtrl);
      
      CHECK_ERROR(ADM_coreLibVA::funcs.decodePicture(&in));
      if(Success!=xError)
      {
          return false;
      }
      return true;
      
}
/**
 *      \fn decodeEnd
 */
bool admLibVA::decodeEnd(void *session)
{
    int xError;
      CHECK_WORKING(false);
      XVBA_Decode_Picture_End_Input in;
      PREPARE_SESSION_IN(session,in);
      
       CHECK_ERROR(ADM_coreLibVA::funcs.endDecodePicture(&in));
      
      if(xError!=Success)
      {
          ADM_info("decodeEnd failed\n");
          return false;
      }
       return true;
}
/**
 * \fn syncSurface
 */
bool admLibVA::syncSurface(void *session, void *surface, bool *ready)
{
    int xError;
    CHECK_WORKING(false);

    XVBA_Surface_Sync_Input  in;
    XVBA_Surface_Sync_Output out;
    PREPARE_SESSION_IN(session,in);
    PREPARE_OUT(out);
    
    in.surface=surface;
    in.query_status=XVBA_GET_SURFACE_STATUS;
    
    CHECK_ERROR(ADM_coreLibVA::funcs.syncSurface(&in,&out));

    if(xError!=Success)
    {
        ADM_info("syncSurface failed\n");
        return false;
    }
    
    if(!(out.status_flags & XVBA_COMPLETED))
    {
         *ready=false;
    }
    *ready=true;
    return true;
    
}
/**
 * \fn transfer
 * \brief fetch back a decoded image
 * @param session
 * @param surface
 * @param img
 * @return 
 */
bool        admLibVA::transfer(void *session, int w, int h,void *surface, ADMImage *img,uint8_t *tmpBuffer)
{
    int xError;
    CHECK_WORKING(false);

    
    XVBA_GetSurface_Target target;
    XVBA_Get_Surface_Input input;
    PREPARE_SESSION_IN(session,input);
    
    int pw= (w+15) & ~15; 
    int ph= (h+15) & ~15; 
    
    
    target.size = sizeof(target);
    target.surfaceType = XVBA_YV12;
    target.flag = XVBA_FRAME;

    input.src_surface=surface;

    aprintf("Getting surface %d x %d, pitch = %d\n",w,h,pw);
    
    
    
    input.target_buffer         = tmpBuffer;
    input.target_pitch          = pw;
    input.target_width          = pw;
    input.target_height         = ph;
    input.target_parameter      = target;

  
    
        
    CHECK_ERROR(ADM_coreLibVA::funcs.getSurface(&input));

    if(xError!=Success)
    {
        ADM_info("transfer failed\n");
        free(tmpBuffer);
        return false;
    }
    int spitch;
    for(int i=0;i<3;i++)
    {
        ADM_PLANE plane=(ADM_PLANE)i;
        int pitch=img->GetPitch(plane);
        int height=img->GetHeight(plane);
        int width=img->GetWidth(plane);
        uint8_t *dst=img->GetWritePtr(plane);
        uint8_t *src;
        switch(i)
        {
        case 0: src=tmpBuffer;spitch=pw;break;
        case 1: src=tmpBuffer+pw*ph;spitch=pw/2;break;
        case 2: src=tmpBuffer+(pw*ph*5)/4;spitch=pw/2;break;
        }
        for(int y=0;y<height;y++)
        {
            memcpy(dst,src,width);
            src+=spitch;
            dst+=pitch;
        }
    }
    
    return true;
 }

#endif //#if 0
#endif // ifdef LIBVA