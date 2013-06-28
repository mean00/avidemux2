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
#include "../include/ADM_coreXvba.h"
#include "fglrxinfo.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>


#ifdef USE_XVBA
#include "../include/ADM_coreXvbaInternal.h"
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
 
GUI_WindowInfo      admXvba::myWindowInfo;

namespace ADM_coreXvba
{
 XvbaFunctions          funcs; 
 void                   *context;
 Display                *display;
 namespace decoders
 {
        bool            h264;
        XVBADecodeCap   h264_decode_cap;
        bool            vc1;
 
 }
}

static ADM_LibWrapper        xvbaDynaLoader;
static bool                  coreXvbaWorking=false;

#define CLEAR(x) memset(&x,0,sizeof(x))
#define PREPARE_IN(z) {CLEAR(z);z.size=sizeof(z);z.context=ADM_coreXvba::context;}
#define PREPARE_SESSION_IN(session,z) {CLEAR(z);z.size=sizeof(z);z.session=session;}
#define PREPARE_OUT(z) {CLEAR(z);z.size=sizeof(z);}
#define CHECK_ERROR(x) {xError=x;displayXError(#x,ADM_coreXvba::display,xError);}

/**
 * \fn displayXError
 * @param dis
 * @param er
 */
static void displayXError(const char *func,Display *dis,int er)
{
    if(er==Success) return;
    char errString[200];
    XGetErrorText (dis,er,errString,sizeof(errString)-1);
    ADM_warning("X11 Error : <%s:%s>\n",func,errString);

}

/**
    \fn     init
    \brief
*/
bool admXvba::init(GUI_WindowInfo *x)
{
    ADM_coreXvba::display=(Display *)x->display;
    int maj=0,min=0,patch=0;
    ADM_info("Checking it is an ATI/AMD device..\n");
    if(! fglrx_get_version(ADM_coreXvba::display,DefaultScreen(ADM_coreXvba::display),&maj,&min,&patch))
    {
        ADM_info("nope\n");
        return false;
    }
    ADM_info("ATI version %d.%d.%d\n",maj,min,patch);

    
    unsigned int deviceId=0;
    if(! fglrx_get_device_id(ADM_coreXvba::display,DefaultScreen(ADM_coreXvba::display),&deviceId))
    {
        ADM_info("cant get deviceId\n");
        return false;
    }
     ADM_info("Device ID %u\n",deviceId);
     if(fglrx_is_dri_capable(ADM_coreXvba::display,DefaultScreen(ADM_coreXvba::display)))
     {
         ADM_info("Device is DRI capable\n");
     }else
     {
         ADM_info("Device is not DRI capable\n");
     }
    ADM_info("Loading Xvba library ...\n");
    memset(&ADM_coreXvba::funcs,0,sizeof(ADM_coreXvba::funcs));
    ADM_coreXvba::context=NULL;
    ADM_coreXvba::decoders::h264=false;
    ADM_coreXvba::decoders::vc1=false;
            
    myWindowInfo=*x;
    if(false==xvbaDynaLoader.loadLibrary("libXvBAW.so.1"))
    {
        ADM_info("Cannot load libxvba.so\n");
        return false;
    }
    int xError;
  
    
    
#define GetMe(fun,id)         {ADM_coreXvba::funcs.fun= (typeof( ADM_coreXvba::funcs.fun))xvbaDynaLoader.getSymbol(#id);\
                                if(! ADM_coreXvba::funcs.fun) {ADM_error("Cannot load symbol %s\n",#id);return false;}}
        
  GetMe(             queryExtension,           XVBAQueryExtension)
  GetMe(             createContext,            XVBACreateContext)
  GetMe(             destroyContext,           XVBADestroyContext)
  GetMe(             getSessionInfo,           XVBAGetSessionInfo)
  GetMe(             createSurface,            XVBACreateSurface)
  GetMe(             createGLSharedSurface,    XVBACreateGLSharedSurface)
  GetMe(             destroySurface,           XVBADestroySurface)
  GetMe(             createDecodeBuffer,       XVBACreateDecodeBuffers)
  GetMe(             destroyDecodeBuffer,      XVBADestroyDecodeBuffers)
  GetMe(             getCapDecode,             XVBAGetCapDecode) 
  GetMe(             createDecode,             XVBACreateDecode)
  GetMe(             destroyDecode,            XVBADestroyDecode)
  GetMe(             startDecodePicture,       XVBAStartDecodePicture)
  GetMe(             decodePicture,            XVBADecodePicture)
  GetMe(             endDecodePicture,         XVBAEndDecodePicture)
  GetMe(             syncSurface,              XVBASyncSurface)
  GetMe(             getSurface,               XVBAGetSurface)
  GetMe(             transferSurface,          XVBATransferSurface)
    
  // Time to query
   int version=0;
   if(!ADM_coreXvba::funcs.queryExtension((Display *)x->display,&version))
   {
       ADM_warning("Xvba Query extension failed\n");
       return false;
   }
  ADM_info("Xvba version %d\n",version);
  // ----------Create global context------------------
  
  XVBA_Create_Context_Input  contextInput;
  XVBA_Create_Context_Output contextOutput;
  CLEAR(contextInput);
  PREPARE_OUT(contextOutput);
  
  contextInput.size=sizeof(contextInput);
  contextInput.display=ADM_coreXvba::display;
  contextInput.draw= x->window; // fixme
  
  CHECK_ERROR(ADM_coreXvba::funcs.createContext(&contextInput,&contextOutput))
  if(Success!=xError)
  {
      ADM_warning("Xvba context creation failed\n");
      return false;
  }

    ADM_info("[XVBA] Context created ok\n");
    ADM_coreXvba::context=contextOutput.context;
    //--------------- Session info -------------------------
     XVBA_GetSessionInfo_Input   infoInput;
     XVBA_GetSessionInfo_Output  infoOutput;
     PREPARE_IN(infoInput);
     PREPARE_OUT(infoOutput);
     
     ADM_info("[XVBA] Getting session info...\n");
    CHECK_ERROR(ADM_coreXvba::funcs.getSessionInfo(&infoInput,&infoOutput))
    if(Success!=xError)
    {
        ADM_warning("Xvba getSessionInfo failed\n");
        return false;
    }
    // -------------- Get decode cap---------------------
#if 1    // crash ???    
    XVBA_GetCapDecode_Input  capin;
    XVBA_GetCapDecode_Output capout;
    PREPARE_IN(capin);
    PREPARE_OUT(capout);
    
    uint8_t buffer[1024*1024]; // try to prevent xvba from trashing the stack
    
// -
    CHECK_ERROR(ADM_coreXvba::funcs.getCapDecode(&capin,&capout));
    if(Success!=xError)
    {
        ADM_warning("Can't get Xvba decode capabilities\n");
        return false;
    }
     
    for(int c=0;c<capout.num_of_decodecaps;c++)
    {
        switch(capout.decode_caps_list[c].capability_id)
        {
            case XVBA_H264:       ADM_info("\tH264");
                                  ADM_coreXvba::decoders::h264=true;
                                  ADM_coreXvba::decoders::h264_decode_cap=capout.decode_caps_list[c];
                                  break;
            case XVBA_VC1:        ADM_info("\tVC1");ADM_coreXvba::decoders::vc1=true;break;
            case XVBA_MPEG2_IDCT: ADM_info("\tMPEG2 IDCT");break;
            case XVBA_MPEG2_VLD : ADM_info("\tMPEG2 VLD");break;
            default :             ADM_info("\t???\n");break;
        }
        printf(" decoder supported\n");
    }
#endif        
    
    coreXvbaWorking=true;

    
    ADM_info("[XVBA] Xvba  init ok.\n");
    return true;
}
/**
    \fn cleanup
*/
bool admXvba::cleanup(void)
{
    if(true==coreXvbaWorking)
    {
           
           if(ADM_coreXvba::context)
           {
               ADM_coreXvba::funcs.destroyContext(ADM_coreXvba::context);
               ADM_coreXvba::context=NULL;
           }
    }
    coreXvbaWorking=false;
    return true;
}
/**
    \fn isOperationnal
*/
bool admXvba::isOperationnal(void)
{
    return coreXvbaWorking;
}

#define CHECK_WORKING(x)   if(!coreXvbaWorking) {ADM_warning("Xvba not operationnal\n");return x;}

/**
 * 
 * @param width
 * @param height
 * @return 
 */
void        *admXvba::createDecoder(int width, int height)
{
    CHECK_WORKING(NULL);
    
    int xError;
    
    XVBADecodeCap                     cap;
    XVBA_Create_Decode_Session_Input  sessionInput;
    XVBA_Create_Decode_Session_Output sessionOutput;
    PREPARE_IN(sessionInput);    
    PREPARE_OUT(sessionOutput);
        
    sessionInput.width=(width+15) & ~15;
    sessionInput.height=(height+15) & ~15;
    sessionInput.decode_cap=&(ADM_coreXvba::decoders::h264_decode_cap);
     if( XVBA_H264!=ADM_coreXvba::decoders::h264_decode_cap.capability_id)
     {
         ADM_warning("Cap is not H264\n");
     }
    printf("H264:");
    
    switch(ADM_coreXvba::decoders::h264_decode_cap.flags)
    {
        case XVBA_H264_BASELINE: printf("Baseline\n");break;
        case XVBA_H264_MAIN:printf("Main\n");break;
        case XVBA_H264_HIGH:printf("High\n");break;
        default:  printf("Profile:???\n");break;
    }
    switch(ADM_coreXvba::decoders::h264_decode_cap.surface_type)
    {
#define MKS(x) case x: printf("Format :"#x"\n");break;
        MKS(  XVBA_NV12)
        MKS(  XVBA_YUY2)
        MKS(  XVBA_ARGB)
        MKS(  XVBA_AYUV)
        MKS(  XVBA_YV12)
        default:  printf("surface format ???\n");
    }
    // NV12 & AYUV works
    //ADM_coreXvba::decoders::h264_decode_cap.surface_type=XVBA_AYUV;
    
    ADM_info("Creating xvba decoder, %d x %d, %d x %d \n",width,height,sessionInput.width,sessionInput.height);
    CHECK_ERROR(ADM_coreXvba::funcs.createDecode(&sessionInput,&sessionOutput));
    if(Success==xError)
    {
        ADM_info("Xvba session created successfully\n");
        return sessionOutput.session;
    }
     ADM_warning("Xvba session failed\n");
     return NULL;
}

/**
 * \fn destroySession
 */
bool admXvba::destroyDecoder(void *session)
{
    bool r=false;
     CHECK_WORKING(false);
     if(Success == ADM_coreXvba::funcs.destroyDecode(session))
     {
         ADM_info("Xvba decoder destroyed\n");
         r=true;
     }else
        ADM_info("Error destroying Xvba decoder\n");
     return r;
}
/**
 * \fn allocateSurface
 * @param w
 * @param h
 * @return 
 */
void        *admXvba::allocateSurface(void *session,int w, int h)
{
       int xError;
       CHECK_WORKING(NULL);
       w=(w+15) & ~15; 
       h=(h+15) & ~15; 
       XVBA_Create_Surface_Input in;
       XVBA_Create_Surface_Output out;
       
       PREPARE_SESSION_IN(session,in);
       
       
       PREPARE_OUT(out);
       in.width=w;
       in.height=h;
       in.surface_type=XVBA_NV12;
       
       aprintf("Creating surface %d x %d\n",w,h);
        CHECK_ERROR(ADM_coreXvba::funcs.createSurface(&in,&out));
        if(Success==xError)
        {
            return out.surface;
        }
        aprintf("Error creating surface\n");
        return NULL;
    
}
/**
 * \fn destroySurface
 * @param session
 * @param surface
 */
void        admXvba::destroySurface(void *session, void *surface)
{
      int xError;
      CHECK_WORKING();
      CHECK_ERROR(ADM_coreXvba::funcs.destroySurface(surface));
        if(Success==xError)
        {
            return;
        }
        aprintf("Error destroying surface\n");
        return;
}

/**
 * \fn destroySurface
 * @param session
 * @param surface
 */
XVBABufferDescriptor        *admXvba::createDecodeBuffer(void *session,XVBA_BUFFER type)
{
      CHECK_WORKING(NULL);
      int xError;
      XVBA_Create_DecodeBuff_Input  in;
      XVBA_Create_DecodeBuff_Output out;
      PREPARE_SESSION_IN(session,in);
      PREPARE_OUT(out);
      in.buffer_type=type;
      in.num_of_buffers=1;
      
      CHECK_ERROR(ADM_coreXvba::funcs.createDecodeBuffer(&in,&out));
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
void        admXvba::destroyDecodeBuffer(void *session,XVBABufferDescriptor *decodeBuffer)
{
        int xError;
        XVBA_Destroy_Decode_Buffers_Input in;
        CHECK_WORKING();
        PREPARE_SESSION_IN(session,in);
        in.num_of_buffers_in_list=1;
        in.buffer_list=decodeBuffer;
        CHECK_ERROR(ADM_coreXvba::funcs.destroyDecodeBuffer(&in));
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
bool        admXvba::decodeStart(void *session, void *surface)
{
      int xError;
      CHECK_WORKING(false);
      XVBA_Decode_Picture_Start_Input in;
      PREPARE_SESSION_IN(session,in);

      in.target_surface=surface;
      
       CHECK_ERROR(ADM_coreXvba::funcs.startDecodePicture(&in));
      
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
bool        admXvba::decode(void *session,void *picture_desc,void *matrix_desc,bool set,int off0,int size1)
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
      
      if(set)
      {
          desc[0]->data_offset=off0;
          desc[1]->data_size_in_buffer=size1;
      }
      
      CHECK_ERROR(ADM_coreXvba::funcs.decodePicture(&in));
      if(Success!=xError)
      {
          return false;
      }
      return true;
      
}
/**
 *      \fn decodeEnd
 */
bool admXvba::decodeEnd(void *session)
{
    int xError;
      CHECK_WORKING(false);
      XVBA_Decode_Picture_End_Input in;
      PREPARE_SESSION_IN(session,in);
      
       CHECK_ERROR(ADM_coreXvba::funcs.endDecodePicture(&in));
      
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
bool admXvba::syncSurface(void *session, void *surface, bool *ready)
{
    int xError;
    CHECK_WORKING(false);

    XVBA_Surface_Sync_Input  in;
    XVBA_Surface_Sync_Output out;
    PREPARE_SESSION_IN(session,in);
    PREPARE_OUT(out);
    
    in.surface=surface;
    in.query_status=XVBA_GET_SURFACE_STATUS;
    
    CHECK_ERROR(ADM_coreXvba::funcs.syncSurface(&in,&out));

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
bool        admXvba::transfer(void *session, int w, int h,void *surface, ADMImage *img,uint8_t *tmpBuffer)
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

  
    
        
    CHECK_ERROR(ADM_coreXvba::funcs.getSurface(&input));

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

#else 
//******************************************
//******************************************
// Dummy when xvba is not there...
// Dummy when xvba is not there...
//******************************************
//******************************************
static bool                  coreVdpWorking=false;
bool admXvba::init(GUI_WindowInfo *x)
{
          return false;
}
  
/**
    \fn isOperationnal
*/
bool admXvba::isOperationnal(void)
{
    ADM_warning("This binary has no VPDAU support\n");
    return coreVdpWorking;
}
bool admXvba::cleanup(void)
{
    ADM_warning("This binary has no VPDAU support\n");
    return true;
}
#endif
// EOF
