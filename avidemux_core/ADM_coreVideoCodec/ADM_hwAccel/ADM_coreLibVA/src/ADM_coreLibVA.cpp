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
 VAConfigID             config;
 VAImageFormat          imageFormatNV12;
 VAImageFormat          imageFormatYV12;
 bool                   directUpload;
 bool                   directDownload;
 namespace decoders
 {
        bool            h264; 
 }
}

static bool                  coreLibVAWorking=false;

#define CLEAR(x) memset(&x,0,sizeof(x))
#define CHECK_ERROR(x) {xError=x;displayXError(#x,ADM_coreLibVA::display,xError);}

static bool checkMarkers(uint8_t *mark)
{
    if(mark[0]==0x11 &&    mark[800]==0x22 &&    mark[1600]==0x33) return true;
    ADM_info("Marker do not check\n");
    return false;
}
static bool setMarkers(uint8_t *mark)
{
    mark[0]=0x11;
    mark[800]=0x22;
    mark[1600]=0x33;
    return true;
}
static bool resetMarkers(uint8_t *mark)
{
    mark[0]=0x4;
    mark[800]=0x5;
    mark[1600]=0x6;
    return true;
}

/**
 * \fn tryIndirectUpload
 * @param title
 * @param image
 * @param image1
 * @param image2
 * @return 
 */
static bool tryIndirectUpload(const char *title, ADM_vaSurface &admSurface,VAImage *image, ADMImage &image1)
{
    bool work=false;
            ADM_info("%s indirect upload... \n",title);
            if(!admLibVA::uploadToImage(&image1,image))
            {
                ADM_info("Upload to yv12 image failed \n");
                return false;
            }
            if(!admLibVA::imageToSurface(image,&admSurface))
            {                    
                ADM_info("image to surface failed\n");
                return false;
            }
            return true;
}
/**
 * \fn tryIndirectDownload
 * @param title
 * @param admSurface
 * @param image
 * @param image1
 * @param image2
 * @return 
 */
static bool tryIndirectDownload(const char *title, ADM_vaSurface &admSurface,VAImage *image, ADMImage &image2)
{
    bool work=false;
            ADM_info("%s indirect upload... \n",title);
            if(!admLibVA::surfaceToImage(&admSurface,image))
            {
                ADM_info("Surface to image failed\n");
                return false;
            }
            if(!admLibVA::downloadFromImage(&image2,image))            
            {
                   ADM_info("download from image failed\n");
                   return false;
            }
            uint8_t *ptr=image2.GetReadPtr(PLANAR_Y);
            return checkMarkers(ptr);
}


/**
 * \fn checkSupportedFunctionsAndImageFormat
 * \brief check if operation through vaDeriveImage is supported and if 
 *           YV12 or NV12 is supported
 * @param func
 * @param dis
 * @param er
 */
static bool checkSupportedFunctionsAndImageFormat(void)
{
    bool r=false;
    ADMImageDefault  image1(640,400),image2(640,400);
    VASurfaceID      surface=admLibVA::allocateSurface(640,400); // freed with admSurface
    ADM_vaSurface    admSurface(NULL,640,400);        
    VAImage          *image=NULL;
    admSurface.surface=VA_INVALID;
    
    if(surface==VA_INVALID)
    {
        ADM_info("Cannot allocate a surface => not working\n");
        return false;
    }
    
    uint8_t *mark=image1.GetWritePtr(PLANAR_Y);
    mark[0]=0x11;
    mark[800]=0x22;
    mark[1600]=0x33;
    if(surface==VA_INVALID) goto done; // does not work..
    admSurface.surface=surface;
    ADM_info("Direct upload...\n");
    if(true==admLibVA::admImageToSurface(&image1,&admSurface))
    {
        ADM_info("\tworks\n");
        
        // Surface to image now
        ADM_coreLibVA::directUpload=true;
        // read it back
        ADM_info("Direct download ...\n");
        if(true==admLibVA::surfaceToAdmImage(&image2,&admSurface))
        {
            ADM_info("\tCall ok, checking value\n");
            mark=image2.GetWritePtr(PLANAR_Y);
            
            if(checkMarkers(mark))
            {
                ADM_coreLibVA::directDownload=true;
            }else
            {
                ADM_info("Value incorrect\n");
            }
        }
        r=true;
    
    }//else
    {
        //--------------- YV12 -------------
        ADM_info("Trying indirect transfer...\n");
        image=admLibVA::allocateYV12Image(640,400);
        if(!image)
        {
            ADM_info("Cannot allocate YV12 image\n");
        }else
        {
           setMarkers(image1.GetReadPtr(PLANAR_Y));
           resetMarkers(image2.GetReadPtr(PLANAR_Y));
           if(true==tryIndirectUpload("YV12",admSurface,image, image1))
           {
               ADM_info("YV12 upload works\n");
           }
            if(true==tryIndirectDownload("YV12",admSurface,image,image2))   
           {
               ADM_info("YV12 download  works\n");
           }
            admLibVA::destroyImage(image);
            image=NULL;
        }
        //--------------- NV12 -------------
        image=admLibVA::allocateNV12Image(640,400);
        if(!image)
        {
            ADM_info("Cannot allocate NV12 image\n");
        }else
        {
           setMarkers(image1.GetReadPtr(PLANAR_Y));
           resetMarkers(image2.GetReadPtr(PLANAR_Y));
           if(true==tryIndirectUpload("NV12",admSurface,image, image1))
           {
               ADM_info("NV12 upload works\n");
           }
           if(true==tryIndirectDownload("NV12",admSurface,image,image2))
           {
               ADM_info("NV12 download works\n");
           }
            admLibVA::destroyImage(image);
            image=NULL;
        }
        
    }
    
done:    
        ADM_info("Direct upload    : %d\n",ADM_coreLibVA::directUpload);
        ADM_info("Direct download  : %d\n",ADM_coreLibVA::directDownload);
    return r;
}

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
                switch(fcc)
                {
                case VA_FOURCC_NV12:
                    ADM_coreLibVA::imageFormatNV12=list[i];
                    r=true;
                    break;
                case VA_FOURCC_YV12:
                     ADM_coreLibVA::imageFormatYV12=list[i];
                     r=true;
                     break;
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
//    ADM_coreLibVA::imageFormatNV12=VA_INVALID;
//    ADM_coreLibVA::imageFormatYV12=VA_INVALID;
    ADM_coreLibVA::directUpload=false;
    ADM_coreLibVA::directDownload=false;
            
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
    checkSupportedFunctionsAndImageFormat();
    
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
VAImage   *admLibVA::allocateNV12Image( int w, int h)
{
    int xError=1;
    CHECK_WORKING(NULL);
    VAImage *image=new VAImage;
    memset(image,0,sizeof(image));
    CHECK_ERROR(vaCreateImage ( ADM_coreLibVA::display, &ADM_coreLibVA::imageFormatNV12,
                w,    h,    
                image));
    if(xError)
    {
        ADM_warning("Cannot allocate nv12 image\n");
        delete image;
        return NULL;
    }
    return image;
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
    CHECK_ERROR(vaCreateImage ( ADM_coreLibVA::display, &ADM_coreLibVA::imageFormatYV12,
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
 * \fn surfaceToImage
 * @param id
 * @param img
 * @return 
 */
bool        admLibVA::surfaceToAdmImage(ADMImage *dest,ADM_vaSurface *src)
{
    int xError;
    bool r=false;
    VASurfaceStatus status;
    CHECK_WORKING(false);
    uint8_t *ptr=NULL;
    //--------------------------
    // Wait for surface to be ready...
    //--------------------------
    int countDown=20;
    while(1)
    {
     CHECK_ERROR(vaQuerySurfaceStatus ( ADM_coreLibVA::display, src->surface,&status));
     if(xError)
     {
         ADM_warning("QuerySurfacStatus failed\n");
         return false;
     }
     if(status==VASurfaceReady) break;
     countDown--;
     if(!countDown) 
     {
         ADM_warning("Timeout waiting for surface\n");
         return false;
     }
     ADM_usleep(1000);
    }
    //--------------------------
    // Derive Image
    //--------------------------
    VAImage vaImage;
    CHECK_ERROR(vaDeriveImage (ADM_coreLibVA::display, src->surface,&vaImage));
    if(xError)
    {
        ADM_warning("Va GetImage failed\n");
        return false;
    }
    switch(vaImage.format.fourcc)
    {
        case VA_FOURCC_YV12:break;
        case VA_FOURCC_NV12:break;
        default:  
            ADM_warning("Unknown format %s\n",fourCC_tostring(vaImage.format.fourcc));
            goto dropIt;
    }
    // Map image...
     
    CHECK_ERROR(vaMapBuffer(ADM_coreLibVA::display, vaImage.buf, (void**)&ptr))
    if(!xError)        
    {
         switch(vaImage.format.fourcc)
        {
                case VA_FOURCC_YV12:
                {
                    ADMImageRefWrittable ref(dest->_width,dest->_height);
                    for(int i=0;i<3;i++)
                    {
                            ref._planes[i]= ptr+vaImage.offsets[i];
                            ref._planeStride[i]=vaImage.pitches[i];
                    }
                    dest->duplicate(&ref);
                    break;
                }
                case VA_FOURCC_NV12:
                {
                    dest->convertFromNV12(ptr+vaImage.offsets[0],ptr+vaImage.offsets[1], vaImage.pitches[0], vaImage.pitches[1]);
                    break;
                }
                default:  
                        goto dropIt;
    }
         r=true;
        CHECK_ERROR(vaUnmapBuffer(ADM_coreLibVA::display, vaImage.buf))
    }
dropIt:    
    CHECK_ERROR(vaDestroyImage (ADM_coreLibVA::display,vaImage.image_id));
    
    
    return r;
}
/**
 * \fn putX11Surface
 * @param img
 * @param widget
 * @param displayWidth
 * @param displayHeight
 * @return 
 */
bool        admLibVA::putX11Surface(ADM_vaSurface *img,int widget,int displayWidth,int displayHeight)
{
    int xError;
    VASurfaceStatus status;
    CHECK_WORKING(false);
    CHECK_ERROR(vaPutSurface ( ADM_coreLibVA::display, img->surface,(Drawable)widget,0,0,img->w, img->h,0,0,displayWidth,displayHeight,
                              NULL,0 // clip & and num clip
                              ,0));  // flags
    if(xError)
    {
        ADM_warning("putX11Surface failed\n");
        return false;
    }
    return true;
}

/***
 *      \fn imageToSurface
 */
bool   admLibVA::imageToSurface(VAImage *src, ADM_vaSurface *dst)
{
    
    int xError;
    VASurfaceStatus status;
    CHECK_WORKING(false);
    CHECK_ERROR(vaPutImage(ADM_coreLibVA::display,
                           dst->surface,
                           src->image_id,
                           0,0,
                           dst->w,dst->h,
                           0,0,
                           dst->w,dst->h));
    if(xError)
    {
        ADM_warning("[libVa] ImageToSurface failed\n");
        return false;
    }
    return true;
}

/***
 *      \fn imageToSurface
 */
bool   admLibVA::surfaceToImage(ADM_vaSurface *dst,VAImage *src )
{
    
    int xError;
    VASurfaceStatus status;
    CHECK_WORKING(false);
    CHECK_ERROR(vaGetImage(ADM_coreLibVA::display,
                           dst->surface,                           
                           0,0,
                           dst->w,dst->h,
                           src->image_id
                          ));
    if(xError)
    {
        ADM_warning("[libVa] surfaceToImage failed\n");
        return false;
    }
    return true;
}

/**
 * \fn uploadToImage
 * @param dest
 * @param src
 * @return 
 */
bool   admLibVA::uploadToImage( ADMImage *src,VAImage *dest)
{
    int xError;
    VASurfaceStatus status;
    CHECK_WORKING(false);
    uint8_t *ptr=NULL;
    CHECK_ERROR(vaMapBuffer(ADM_coreLibVA::display, dest->buf, (void**)&ptr))
    if(xError)        
    {
        ADM_warning("Cannot map image\n");
        return false;
    }
    int w=src->_width;
    int h=src->_height;
    int dstStride= dest->pitches[0];
    int srcStride= src->GetPitch(PLANAR_Y);
    uint8_t *s=    src->GetReadPtr(PLANAR_Y);
    uint8_t *d=    ptr+dest->offsets[0];
    for(int y=0;y<h;y++)
    {
        memcpy(d,s,w);
        s+=srcStride;
        d+=dstStride;
    }
    
    CHECK_ERROR(vaUnmapBuffer (ADM_coreLibVA::display,dest->buf));    
    return true;
}

/**
 * \fn uploadToImage
 * @param dest
 * @param src
 * @return 
 */
bool   admLibVA::downloadFromImage( ADMImage *src,VAImage *dest)
{
    int xError;
    VASurfaceStatus status;
    CHECK_WORKING(false);
    uint8_t *ptr=NULL;
    CHECK_ERROR(vaMapBuffer(ADM_coreLibVA::display, dest->buf, (void**)&ptr))
    if(xError)        
    {
        ADM_warning("Cannot map image\n");
        return false;
    }
    int w=src->_width;
    int h=src->_height;
    int dstStride= dest->pitches[0];
    int srcStride= src->GetPitch(PLANAR_Y);
    uint8_t *s=    src->GetReadPtr(PLANAR_Y);
    uint8_t *d=    ptr+dest->offsets[0];
    for(int y=0;y<h;y++)
    {
        memcpy(s,d,w);
        s+=srcStride;
        d+=dstStride;
    }
    
    CHECK_ERROR(vaUnmapBuffer (ADM_coreLibVA::display,dest->buf));    
    return true;
}
/**
 * \fn copyNV12
 * @param ptr
 * @param dest
 * @param src
 */
static void  copyNV12(uint8_t *ptr, VAImage *dest, ADMImage *src) 
{
          int w=src->_width;

         int h=src->_height;
         int dstStride= dest->pitches[0];
         int srcStride= src->GetPitch(PLANAR_Y);
         uint8_t *s=    src->GetReadPtr(PLANAR_Y);
         uint8_t *d=    ptr+dest->offsets[0];
         for(int y=0;y<h;y++)
         {
                memcpy(d,s,w);
                s+=srcStride;
                d+=dstStride;
         }
         
        w=w/2;
        h=h/2;
        uint8_t *srcu=src->GetReadPtr(PLANAR_U);
        uint8_t *srcv=src->GetReadPtr(PLANAR_V);
        int     uStride=src->GetPitch(PLANAR_U);
        int     vStride=src->GetPitch(PLANAR_V);
                dstStride=dest->pitches[1];
        uint8_t *dstPtr= ptr+dest->offsets[1];
        for(int y=0;y<h;y++)
        {
                uint8_t *ssrcu=srcu;
                uint8_t *ssrcv=srcv;
                uint8_t *d=dstPtr;
                
                srcu+=uStride;
                srcv+=vStride;
                dstPtr+=dstStride;

                for(int x=0;x<w;x++)
                {
                    d[0]=*ssrcu++;
                    d[1]=*ssrcv++;
                    d+=2;
                }
        }
}
/**
 * \fn uploadToSurface
 * @param src
 * @param dest
 * @return 
 */
bool   admLibVA:: admImageToSurface( ADMImage *src,ADM_vaSurface *dest)
{
    int xError;
    bool r=false;
    VASurfaceStatus status;
    CHECK_WORKING(false);
    uint8_t *ptr=NULL;
    
    VAImage vaImage;
    CHECK_ERROR(vaDeriveImage (ADM_coreLibVA::display, dest->surface,&vaImage));
    if(xError)
    {
        ADM_warning("Va Derive failed\n");
        return false;
    }
    // NV12 or YV12
    switch(vaImage.format.fourcc)
    {
        case VA_FOURCC_YV12:break;
        case VA_FOURCC_NV12:break;
        default:  
            ADM_warning("Unknown format %s\n",fourCC_tostring(vaImage.format.fourcc));
            goto dontTry;
    }

    // Map image...

    CHECK_ERROR(vaMapBuffer(ADM_coreLibVA::display, vaImage.buf, (void**)&ptr))
    if(!xError)        
    {
         r=true;
         // NV12 or YV12
        switch(vaImage.format.fourcc)
        {
            case VA_FOURCC_YV12: 
            {
                ADMImageRefWrittable ref(src->_width,src->_height);
                for(int i=0;i<3;i++)
                {
                        ref._planes[i]= ptr+vaImage.offsets[i];
                        ref._planeStride[i]=vaImage.pitches[i];
                }
                ref.duplicate(src);
            }break;
            case VA_FOURCC_NV12: copyNV12(ptr,&vaImage,src);break;
            default:  
                ADM_warning("Unknown format %s\n",fourCC_tostring(vaImage.format.fourcc));
        }
        CHECK_ERROR(vaUnmapBuffer(ADM_coreLibVA::display, vaImage.buf))
    }
dontTry:    
    CHECK_ERROR(vaDestroyImage (ADM_coreLibVA::display,vaImage.image_id));
    
    return r;
}
#endif