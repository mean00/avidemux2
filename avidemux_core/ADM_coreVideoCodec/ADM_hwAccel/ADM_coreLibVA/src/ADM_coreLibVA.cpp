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

#ifdef USE_LIBVA
#include "../include/ADM_coreLibVA_internal.h"
#include "ADM_dynamicLoading.h"
#include "ADM_windowInfo.h"
#include "libavcodec/vaapi.h"

#include "fourcc.h"

#define CHECK_WORKING(x)   if(!coreLibVAWorking) {ADM_warning("Libva not operationnal\n");return x;}

#if 1
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
 VAConfigID             config;
 VAImageFormat          imageFormat;
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
 * \fn copyNV12Image
 * \brief copy NV12 to YV12
 * @param yv12
 * @param myImage
 * @param w
 * @param h
 * @param ptr
 */
static void copyNV12Image(uint8_t *yv12,VAImage *tmp,int w,int h,uint8_t *ptr)
{
        // Y
        int dstride=w;
        int sstride=tmp->pitches[0];
        uint8_t *dst=yv12;
        uint8_t *src=ptr+tmp->offsets[0];
        for(int y=0;y<w;y++)
        {
            memcpy(dst,src,w);
            src+=sstride;
            dst+=dstride;
        }
        // U  & V
        src=ptr+tmp->offsets[1]; // NV12
        uint8_t *dstu=yv12+(w*h);
        uint8_t *dstv=yv12+(5*w*h)/4;
        uint8_t *u,*v,*s;
        
        int half=w/2;
        
        for(int y=0;y<half;y++)
        {
            u=dstu;
            v=dstv;
            s=src;
            dstu+=half;
            dstv+=half;
            src+=tmp->pitches[1];
            for(int x=0;x<half;x++)
            {
                *u++=*s++;
                *v++=*s++;
            }
        }
}

/**
 *      \fn setupConfig
 *      \brief verify that h264 main profile is supported
 */
bool admLibVA::setupConfig(void)
{
    VAStatus xError;
    bool r=false;
    int nb=vaMaxNumProfiles(ADM_coreLibVA::display);
    ADM_info("Max config =  %d \n",nb);
    VAProfile *prof=new VAProfile[nb];
    int nbProfiles;
    CHECK_ERROR(vaQueryConfigProfiles (ADM_coreLibVA::display, prof,&nbProfiles));

    // Check supported profiles
    if(!xError)
    {
        ADM_info("Found %d config \n",nbProfiles);
        for(int i=0;i<nbProfiles;i++)
        {
            if(prof[i]==VAProfileH264High)
            {
                r=true;
                ADM_info("H264 high profile found\n");
            }
        }
    }
    // supported ?
    if(r)
    {
        VAConfigAttrib attrib;
                attrib.type = VAConfigAttribRTFormat;
                CHECK_ERROR(vaGetConfigAttributes(ADM_coreLibVA::display, VAProfileH264High, VAEntrypointVLD, &attrib, 1))

                if (!(attrib.value & VA_RT_FORMAT_YUV420) )
                {
                    ADM_warning("YUV420 not supported\n");
                    r=false;
                }else
                {
                    ADM_info("YUV420 supported\n");
                    
                    VAConfigID id;
                    CHECK_ERROR(vaCreateConfig( ADM_coreLibVA::display, VAProfileH264High, VAEntrypointVLD,&attrib, 1,&id));
                    if(xError)
                    {
                        ADM_warning("Cannot create config\n");
                     }
                    else
                    {
                        ADM_info("Config created\n");
                        ADM_coreLibVA::config=id;
                        r=true;
                    }
                    
                }

 
    }
   
    delete [] prof;
    return r;
    
    
}
/**
 * \fn fourCC_tostring
 * @param fourcc
 * @return 
 */
static char *fourCC_tostring(uint32_t fourcc)
{
    static char s[5];
    s[4] = 0;

	s[3]=((fourcc & 0xff000000)>>24)&0xff;
	s[2]=((fourcc & 0xff0000)>>16)&0xff;
	s[1]=((fourcc & 0xff00)>>8)&0xff;
	s[0]=((fourcc & 0xff)>>0)&0xff;

    return s;
}
/**
 * \fn setupImageFormat
 */
bool admLibVA::setupImageFormat()
{
    int xError;
    bool r=false;
        int nbImage=vaMaxNumImageFormats(ADM_coreLibVA::display);
        VAImageFormat *list=new VAImageFormat[nbImage];
        int nb;
        CHECK_ERROR(vaQueryImageFormats( ADM_coreLibVA::display,list,&nb));
        if(xError)
        {
            r=false;
        }else
        {
            r=false;
            for(int i=0;i<nb;i++)
            {
                aprintf("----------");
                aprintf("bpp : %d\n",list[i].bits_per_pixel);
                uint32_t fcc=list[i].fourcc;
                aprintf("fcc : 0x%x:%s\n",fcc,fourCC_tostring(fcc));                
                if( 0x3231564e==fcc) //NV12 0x3231564e, YV12 0x32315659
                {
                    ADM_coreLibVA::imageFormat=list[i];
                    r=true;
                }
            }
            
        }
        if(r==false)
        {
            ADM_warning("Cannot find supported image format : YV12\n");
        }
        delete [] list;
        return r;
}
/**
 * \fn fillContext
 * @param c
 * @return 
 */    
bool admLibVA::fillContext(vaapi_context *c)
{
    CHECK_WORKING(false);
    c->config_id=ADM_coreLibVA::config;
    c->display=ADM_coreLibVA::display;
    return true;
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
    
    if(setupConfig() && setupImageFormat())
    {
        coreLibVAWorking=true;
    }

    
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



/**
 * \fn createDecoder
 * @param width
 * @param height
 * @return 
 */

VAContextID        admLibVA::createDecoder(int width, int height,int nbSurface, VASurfaceID *surfaces)
{
    int xError=1;
    CHECK_WORKING(VA_INVALID);
    VAContextID id;
    CHECK_ERROR(vaCreateContext ( ADM_coreLibVA::display, ADM_coreLibVA::config,
                width,    height,    
                VA_PROGRESSIVE, // ?? NOT SURE ??
                surfaces,
                nbSurface,
                &id));
    if(xError)
    {
        ADM_warning("Cannot create decoder\n");
        return VA_INVALID;
    }
    aprintf("Decoder created : %llx\n",(uint64_t)id);
    return id;
}
/**
 * 
 * @param w
 * @param h
 * @return 
 */
VAImage   *admLibVA::allocateYV12Image( int w, int h)
{
    int xError=1;
    CHECK_WORKING(NULL);
    VAImage *image=new VAImage;
    memset(image,0,sizeof(image));
    CHECK_ERROR(vaCreateImage ( ADM_coreLibVA::display, &ADM_coreLibVA::imageFormat,
                w,    h,    
                image));
    if(xError)
    {
        ADM_warning("Cannot allocate yv12 image\n");
        delete image;
        return NULL;
    }
    return image;
}
/**
 * 
 * @param surface
 */
void        admLibVA::destroyImage(  VAImage *image)
{
    int xError=1;
    CHECK_WORKING();
    CHECK_ERROR(vaDestroyImage(ADM_coreLibVA::display, image->image_id));
    delete image;
    if(xError)
    {
        ADM_warning("Cannot destroy image\n");
        return ;
    }
    return ;
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
        aprintf("Error destroying decoder\n");
        return false;
}

/**
 * \fn allocateSurface
 * @param w
 * @param h
 * @return 
 */
VASurfaceID        admLibVA::allocateSurface(int w, int h)
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
void        admLibVA::destroySurface( VASurfaceID surface)
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

/**
 * \fn transfer
 * \brief fetch back a decoded image
 * @param session
 * @param surface
 * @param img
 * @return 
 */
bool        admLibVA:: transfer(VAContextID session, int w, int h,VASurfaceID surface, ADMImage *outImage,VAImage *tmp,uint8_t *yv12)
{
    int xError;
    CHECK_WORKING(false);
    bool r=false;
    VAImage myImage;
#if 0    
    VASurfaceStatus status;
    int count=100;
    while(1)
    {
        CHECK_ERROR(vaQuerySurfaceStatus (ADM_coreLibVA::display,surface,&status));
        if(xError)
        {
            ADM_warning("Questy surface status failed\n");
            break;
        }
        if(status==VASurfaceReady)
            break;
        count--;
        if(!count)
        {
            ADM_warning("Timeout getting surface\n");
            return false;
        }
        ADM_usleep(1000);
        
    }
#endif
    
    
    aprintf("Transfer--->0x%x to 0x%x\n",(int)surface,(int)tmp->image_id);
    CHECK_ERROR(vaDeriveImage (ADM_coreLibVA::display,surface,&myImage));
    if(xError)
    {
        ADM_warning("VaDeriveImage failed\n");
        return false;
    }
     uint8_t *ptr=NULL;
     CHECK_ERROR(vaMapBuffer(ADM_coreLibVA::display, myImage.buf, (void**)&ptr))
     if(xError)
     { 
          ADM_warning("Cannot map image\n");
          goto endIt;         
     }
     copyNV12Image(yv12,&myImage,w,h,ptr);

    CHECK_ERROR(vaUnmapBuffer(ADM_coreLibVA::display, myImage.buf))
     if(xError)
     { 
          ADM_warning("Cannot unmap image\n");
     }
    {
        ADMImageRef *ref=outImage->castToRef();
        ref->_planes[0] = yv12;
        ref->_planes[1] = yv12+(w*h);
        ref->_planes[2] = yv12+(5*w*h)/4;
        ref->_planeStride[0] = w;
        ref->_planeStride[1] = w/2;
        ref->_planeStride[2] = w/2;
        r=true;
    }
     
endIt:
    CHECK_ERROR(vaDestroyImage (ADM_coreLibVA::display,myImage.image_id));
    if(xError)
    {
        ADM_warning("DestroyImage failed\n");
    }
    return r;
}
#endif // ifdef LIBVA
    
#if 0
    #if 0    
    CHECK_ERROR(vaGetImage (ADM_coreLibVA::display,surface,
                                        0,0,
                                        w,h,
                                        tmp->image_id));
    if(xError)
    {
        ADM_warning("Cannot getImage image\n");
        return false;
    }
    aprintf("Get image ok\n");
    // Map image
     uint8_t  *ptr = NULL;
     CHECK_ERROR(vaMapBuffer(ADM_coreLibVA::display, tmp->buf, (void**)&ptr))
     if(xError)
     { 
          ADM_warning("Cannot map image\n");
          goto stopIt;
         
     }
    aprintf("Mapping ok\n");
    // Copy Image
    {
        int dstride=outImage->GetPitch(PLANAR_Y);
        int sstride=tmp->pitches[0];
        uint8_t *dst=outImage->GetWritePtr(PLANAR_Y);
        uint8_t *src=ptr+tmp->offsets[0];
        for(int y=0;y<w;y++)
        {
            memcpy(dst,src,w);
            src+=sstride;
            dst+=dstride;
        }
    }
    r=true;
    CHECK_ERROR(vaUnmapBuffer(ADM_coreLibVA::display, tmp->buf))
    if(xError)
    { 
          ADM_warning("Cannot unmap image\n");
          goto stopIt;
    }
     
stopIt:     
     
    return r;
 }
#endif
 
 
#endif