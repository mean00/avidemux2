/***************************************************************************
    \file             : ADM_coreLibVA.cpp
    \brief            : Wrapper around libVA functions
    \author           : (C) 2013/2016 by mean fixounet@free.fr, derived from xbmc_pvr
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
#include "ADM_imageFlags.h"

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
 VAConfigID             configH264;
 VAConfigID             configH265;
 VAConfigID             configH26510Bits;
 VAConfigID             configVC1;
 VAConfigID             configVP9;
 VAImageFormat          imageFormatNV12;
 VAImageFormat          imageFormatYV12;
 bool                   directOperation;
 bool                   indirectOperationYV12;
 bool                   indirectOperationNV12;
admLibVA::LIBVA_TRANSFER_MODE    transferMode;
 
 namespace decoders
 {
        bool            h264; 
 }
}
namespace ADM_coreLibVAEnc
{
 class vaEncoder
 {
 public:
     bool       enabled;
     VAConfigID configId;
     bool       hasCBR;
     bool       hasVBR;
                vaEncoder()
                {
                    enabled=false;
                    configId=-1;
                    hasCBR=false;
                    hasVBR=false;
                }                    
                        
 };
 namespace encoders
 {
        vaEncoder vaH264,vaH265;
 }
}
static bool                  coreLibVAWorking=false;

#define CLEAR(x) memset(&x,0,sizeof(x))
#define CHECK_ERROR(x) {xError=x;displayXError(#x,ADM_coreLibVA::display,xError);}

#include "ADM_coreLibVA_test.cpp"
#include "ADM_bitstream.h"


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
    ADM_vaSurface    admSurface(640,400);            
    admSurface.surface=VA_INVALID;
    
    if(surface==VA_INVALID)
    {
        ADM_info("Cannot allocate a surface => not working\n");
        return false;
    }    
    admSurface.surface=surface;
    
    // Check direct upload/Download works    
    ADM_info("--Trying direct operations --\n");
    ADM_coreLibVA::directOperation      =tryDirect("direct",admSurface, image1,  image2);
    ADM_info("-- Trying indirect (YV12) --\n");
    ADM_coreLibVA::indirectOperationYV12=tryIndirect(0,admSurface, image1 ,image2);
    ADM_info("-- Trying indirect (NV12) --\nSKIPPED\n");
    ADM_coreLibVA::indirectOperationNV12=false; //tryIndirect(1,admSurface, image1, image2 );
    
    ADM_info("Direct           : %d\n",ADM_coreLibVA::directOperation);
    ADM_info("Indirect NV12    : %d\n",ADM_coreLibVA::indirectOperationNV12);
    ADM_info("Indirect YV12    : %d\n",ADM_coreLibVA::indirectOperationYV12);
    if(ADM_coreLibVA::directOperation)
        ADM_coreLibVA::transferMode=admLibVA::ADM_LIBVA_DIRECT;
    else if(ADM_coreLibVA::indirectOperationYV12)
        ADM_coreLibVA::transferMode=admLibVA::ADM_LIBVA_INDIRECT_YV12;
    else if(ADM_coreLibVA::indirectOperationNV12)
        ADM_coreLibVA::transferMode=admLibVA::ADM_LIBVA_INDIRECT_NV12;
    else
    {
         ADM_warning("Did not find a usable way to transfer images to/from LibVA\n");
         ADM_coreLibVA::transferMode=admLibVA::ADM_LIBVA_NONE;
         return false;
    }
    ADM_info("LibVA: All ok\n");
    return true;
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
 * \fn setupEncodingConfig
 * @return 
 */
bool admLibVA::setupEncodingConfig(void)
{
    VAStatus xError;
    VAEntrypoint entrypoints[5];
    int num_entrypoints;
    VAConfigAttrib attrib[2];
    
 
    CHECK_ERROR(vaQueryConfigEntrypoints(ADM_coreLibVA::display, VAProfileH264Main, entrypoints,        &num_entrypoints));
    
    int found=-1;
    ADM_info("Found %d entry points\n",num_entrypoints);
    for	(int slice_entrypoint = 0; slice_entrypoint < num_entrypoints; slice_entrypoint++) 
    {
            ADM_info("   %d is a %d\n",slice_entrypoint,entrypoints[slice_entrypoint] );
        if (entrypoints[slice_entrypoint] == VAEntrypointEncSlice)
        {
            found=slice_entrypoint;
            break;
        }
            
    }
    if(-1 == found)
    {
        ADM_warning("Cannot find encoder entry point\n");
        return false;
    }
     /* find out the format for the render target, and rate control mode */
    attrib[0].type = VAConfigAttribRTFormat;
    attrib[1].type = VAConfigAttribRateControl;
    CHECK_ERROR(vaGetConfigAttributes(ADM_coreLibVA::display, VAProfileH264Main, VAEntrypointEncSlice,    &attrib[0], 2));
    int check=0;
    for(int i=0;i<2;i++)
    {
           unsigned int value=attrib[i].value;
           int type=attrib[i].type;
           switch(type)
           {
                case VAConfigAttribRTFormat:
                        if(value & VA_RT_FORMAT_YUV420)
                        {
                                ADM_info("YUV420 supported\n");
                                check|=1;
                        }
                        break;
                case VAConfigAttribRateControl:
                        #define MKK(x,y) if(type & x) {ADM_info(#x " is supported\n");y;}
                        MKK(VA_RC_CBR,;);
                        MKK(VA_RC_VBR,check|=2);
                        MKK(VA_RC_CQP,;);
                        MKK(VA_RC_VBR_CONSTRAINED,;);
                        break;
                default:
                        ADM_warning("Unknown attribute %d\n",type);
                        break;

           }

    }
    if(check!=3) 
    {
        ADM_warning("Some configuration are missing, bailing\n");
        return false;
    }
    CHECK_ERROR(vaCreateConfig(ADM_coreLibVA::display, VAProfileH264Main, VAEntrypointEncSlice,
                              &attrib[0], 2,&(ADM_coreLibVAEnc::encoders::vaH264.configId)));
    if(xError)
    {
        ADM_coreLibVAEnc::encoders::vaH264.configId=-1;
        return false;
        
    }else
    {
        ADM_info("H264 Encoding config created\n");
        ADM_coreLibVAEnc::encoders::vaH264.enabled=true;
    }
    return true;
}
/**
 * \fn checkProfile
 * @param profile
 * @param cid
 * @param name
 * @return 
 */
static bool checkProfile(const VAProfile &profile,VAConfigID *cid,const char *name)
{
    VAStatus xError;
    
    *cid=-1;
    VAConfigAttrib attrib;
    attrib.type = VAConfigAttribRTFormat;
    CHECK_ERROR(vaGetConfigAttributes(ADM_coreLibVA::display, profile, VAEntrypointVLD, &attrib, 1));
    if(xError)
    {
         ADM_warning("Cannot get attribute for %s \n",name);
         return false;
    }
    CHECK_ERROR(vaCreateConfig( ADM_coreLibVA::display, profile, VAEntrypointVLD,&attrib, 1,cid));
    if(xError)
    {
        ADM_warning("Cannot create config %s\n",name);
        *cid=-1;
        return false;
     }
    ADM_info("Config created %s \n",name);
    return true;
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
    VAProfile *prof=(VAProfile *)alloca(sizeof(VAProfile)*nb);
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
    // if H264 is not supported, no need to go further
    if(!r)
        return false;
    
    
    checkProfile(VAProfileH264High,     &ADM_coreLibVA::configH264,"H264 Hight");
    checkProfile(VAProfileHEVCMain,     &ADM_coreLibVA::configH265,"HEVC Main");
    checkProfile(VAProfileVC1Advanced,  &ADM_coreLibVA::configVC1 ,"VC1");
    checkProfile(VAProfileHEVCMain10,   &ADM_coreLibVA::configH26510Bits ,"H265 10Bits");
    
    
#ifdef ADM_VA_HAS_VP9
    checkProfile(VAProfileVP9Profile3,  &ADM_coreLibVA::configVP9 ,"VP9");
#endif
    return true;
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
bool admLibVA::fillContext(VAProfile profile ,vaapi_context *c)
{
    CHECK_WORKING(false);
    VAConfigID cid;
    switch(profile)
    {
       case VAProfileH264High: cid=ADM_coreLibVA::configH264;break;
       case VAProfileHEVCMain: cid=ADM_coreLibVA::configH265;break;
        case VAProfileHEVCMain10: cid=ADM_coreLibVA::configH26510Bits;break;
       case VAProfileVC1Advanced: cid=ADM_coreLibVA::configVC1;break;
#ifdef ADM_VA_HAS_VP9
       case VAProfileVP9Profile3: cid=ADM_coreLibVA::configVP9;break;
#endif
       default:
                ADM_assert(0);

    }
    c->config_id=cid;
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
    ADM_coreLibVA::directOperation=true;
    ADM_coreLibVA::transferMode=ADM_LIBVA_NONE;
            
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
    
    if(setupEncodingConfig())
    {
        ADM_info("VA: Encoding supported\n");
    }else
    {
        ADM_warning("VA: Encoding not supported\n");
    }
    return checkSupportedFunctionsAndImageFormat();
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
 * 
 * @param profile
 * @return 
 */
bool        admLibVA::supported(VAProfile profile)
{
#define SUPSUP(a,b) case a: if(ADM_coreLibVA::b!=-1) return true;break;
    switch(profile)
    {
        SUPSUP(VAProfileH264High,configH264)
        SUPSUP(VAProfileHEVCMain,configH265)
        SUPSUP(VAProfileHEVCMain10,configH26510Bits)
        SUPSUP(VAProfileVC1Advanced,configVC1)
#ifdef ADM_VA_HAS_VP9
        SUPSUP(VAProfileVP9Profile3,configVP9)
#endif
        default:
            break;
    } 
    return false;
}

/**
 * \fn createDecoder
 * @param width
 * @param height
 * @return 
 */

VAContextID        admLibVA::createDecoder(VAProfile profile,int width, int height,int nbSurface, VASurfaceID *surfaces)
{
    int xError=1;
    CHECK_WORKING(VA_INVALID);
    VAContextID id;
    VAConfigID cid;

    switch(profile)
    {
       case VAProfileH264High:    cid=ADM_coreLibVA::configH264;break;
       case VAProfileHEVCMain:    cid=ADM_coreLibVA::configH265;break;
       case VAProfileHEVCMain10:  cid=ADM_coreLibVA::configH26510Bits;break;       
       case VAProfileVC1Advanced: cid=ADM_coreLibVA::configVC1;break;
#ifdef ADM_VA_HAS_VP9
       case VAProfileVP9Profile3: cid=ADM_coreLibVA::configVP9;break;       
#endif
       default:
                ADM_assert(0);

    }

    CHECK_ERROR(vaCreateContext ( ADM_coreLibVA::display, cid,
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
 * \fn allocateImage
 * \brief allocate the correct image type for transfer : None, YV12 or NV12
 * @param w
 * @param h
 * @return 
 */
 VAImage    *admLibVA::allocateImage( int w, int h)
 {
    switch(ADM_coreLibVA::transferMode)
    {
    case   admLibVA::ADM_LIBVA_NONE: ADM_warning("No transfer supported\n");
    case   admLibVA::ADM_LIBVA_DIRECT:        
                return NULL;break;
    case   admLibVA::ADM_LIBVA_INDIRECT_NV12:
                return admLibVA::allocateNV12Image(w,h);break;
    case   admLibVA::ADM_LIBVA_INDIRECT_YV12:
                return admLibVA::allocateYV12Image(w,h);break;
    default:ADM_assert(0);
    }
    return NULL;
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
    memset(image,0,sizeof(*image));
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
    memset(image,0,sizeof(*image));
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
 
        CHECK_ERROR(vaCreateSurfaces(ADM_coreLibVA::display,
                        VA_RT_FORMAT_YUV420,
                        w,h,
                        &s,1,
                        NULL,0));

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
    switch(dest->format.fourcc)
    {
        case VA_FOURCC_YV12:
                {
                        ADMImageRefWrittable ref(src->_width,src->_height);
                        for(int i=0;i<3;i++)
                        {
                                ref._planes[i]= ptr+dest->offsets[i];
                                ref._planeStride[i]=dest->pitches[i];
                        }
                        ref.duplicate(src);
                }
                break;
        case VA_FOURCC_NV12:src->convertToNV12(  ptr+dest->offsets[0], ptr+dest->offsets[1],dest->pitches[0],dest->pitches[1]);break;
        default: ADM_assert(0);
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
     switch(dest->format.fourcc)
    {
        case VA_FOURCC_YV12:
                {
                        ADMImageRef ref(src->_width,src->_height);
                        ref.copyInfo(src);
                        for(int i=0;i<3;i++)
                        {
                                ref._planes[i]= ptr+dest->offsets[i];
                                ref._planeStride[i]=dest->pitches[i];
                        }
                        src->duplicate(&ref);
                }
                break;
        case VA_FOURCC_NV12:
                        src->convertFromNV12(  ptr+dest->offsets[0], ptr+dest->offsets[1],dest->pitches[0],dest->pitches[1]);
                        break;
        default: ADM_assert(0);
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
/**
 * 
 * @param image
 * @return 
 */
bool ADM_vaSurface::fromAdmImage (ADMImage *dest)
{
    switch(ADM_coreLibVA::transferMode)
    {
    case   admLibVA::ADM_LIBVA_NONE: ADM_warning("No transfer supported\n");return false;break;
    case   admLibVA::ADM_LIBVA_DIRECT:   
                //printf("Direct\n");
                return admLibVA::admImageToSurface (dest,this);
    case   admLibVA::ADM_LIBVA_INDIRECT_NV12:
    case   admLibVA::ADM_LIBVA_INDIRECT_YV12:
                ADM_assert(this->image);
                //printf("InDirect\n");
                if(  admLibVA::uploadToImage(dest,this->image))
                    return  admLibVA::imageToSurface(this->image,this);
                return false;
                break;
    default:ADM_assert(0);
    }
    return false;
}
/**
 * 
 * @param image
 * @return 
 */
bool ADM_vaSurface::toAdmImage(ADMImage *dest)
{
    switch(ADM_coreLibVA::transferMode)
    {
    case   admLibVA::ADM_LIBVA_NONE: ADM_warning("No transfer supported\n");return false;break;
    case   admLibVA::ADM_LIBVA_DIRECT:   
                //printf("Direct\n");
                return admLibVA::surfaceToAdmImage(dest,this);
    case   admLibVA::ADM_LIBVA_INDIRECT_NV12:
    case   admLibVA::ADM_LIBVA_INDIRECT_YV12:
                //printf("InDirect\n");
                ADM_assert(this->image);
                if(admLibVA::surfaceToImage(this,this->image))
                        return  admLibVA::downloadFromImage(dest,this->image);
                return false;
                break;
    default:ADM_assert(0);
    }
    return false;
}
#include "ADM_coreLibVA_encoder.cpp"
#endif


