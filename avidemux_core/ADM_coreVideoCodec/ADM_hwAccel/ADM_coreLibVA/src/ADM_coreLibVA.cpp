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
#include "ADM_imageFlags.h"

#include "fourcc.h"
#include <map>

#define CHECK_WORKING(x)   if(!coreLibVAWorking) {ADM_warning("Libva not operationnal\n");return x;}

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif




/**
 *
 */


static std::map <VASurfaceID, bool>listOfAllocatedSurface;
typedef std::map<VASurfaceID, bool> surfaceList;

static std::map <VAImageID, bool>listOfAllocatedVAImage;
typedef std::map<VAImageID, bool> vaImageList;




GUI_WindowInfo      admLibVA::myWindowInfo;

namespace ADM_coreLibVA
{
typedef struct {
    VAConfigID cid;
    int min_width;
    int min_height;
    int max_width;
    int max_height;
 } decoderConfig;
 void                   *context;
 VADisplay              display;
 decoderConfig          configH264;
 decoderConfig          configMpeg2;
 decoderConfig          configH265;
 decoderConfig          configH26510Bits;
 decoderConfig          configVC1;
 decoderConfig          configVP9;
 VAImageFormat          imageFormatNV12;
 VAImageFormat          imageFormatYV12;
 bool                   directOperation;
 bool                   indirectOperationYV12;
 bool                   indirectOperationNV12;
admLibVA::LIBVA_TRANSFER_MODE    transferMode;
admLibVA::LIBVA_DRIVER_QUIRK     driverQuirks;

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
#define CHECK_ERROR(x) {xError=x;displayXError(#x,ADM_coreLibVA::display,xError);if(xError) printf("%d =<%s>\n",xError,vaErrorStr(xError));}

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
    ADM_info("-- Trying indirect (NV12) --\n");
    ADM_coreLibVA::indirectOperationNV12=tryIndirect(1,admSurface, image1, image2 );

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
    ADM_warning("LibVA Error : <%s:%s:%d>\n",func,vaErrorStr(er),(int)er);


}
/**
 * 
 * @return 
 */
VADisplay admLibVA::getVADisplay()
{
    return ADM_coreLibVA::display;
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
static bool checkProfile(const VAProfile &profile, ADM_coreLibVA::decoderConfig *dconf, const char *name)
{
    VAStatus xError;

    dconf->cid = VA_INVALID;
    dconf->min_width = -1;
    dconf->min_height = -1;
    dconf->max_width = -1;
    dconf->max_height = -1;
    VAConfigAttrib attrib;
    attrib.type = VAConfigAttribRTFormat;
    ADM_info("--Probing %s ...\n",name);
    CHECK_ERROR(vaGetConfigAttributes(ADM_coreLibVA::display, profile, VAEntrypointVLD, &attrib, 1));
    if(xError)
    {
         ADM_warning("Cannot get attribute  for VAEntrypointVLD %s \n",name);
         return false;
    }
    ADM_info("RT Format =0x%x\n",attrib.value);
#define CHECK_RT(x)    if(attrib.value & VA_RT_FORMAT_##x ) ADM_info("\t" #x " supported\n");
    CHECK_RT(YUV420);
    CHECK_RT(YUV422);
    CHECK_RT(YUV444);
    CHECK_RT(YUV420_10);
    CHECK_RT(RGB32);
    
    CHECK_ERROR(vaCreateConfig(ADM_coreLibVA::display, profile, VAEntrypointVLD, &attrib, 1, &dconf->cid))
    if(xError)
    {
        ADM_warning("Cannot create config %s\n",name);
        dconf->cid = VA_INVALID;
        return false;
    }
    ADM_info("Config created %s\n",name);

    unsigned int nb_attr = 0;
    CHECK_ERROR(vaQuerySurfaceAttributes(ADM_coreLibVA::display, dconf->cid, 0, &nb_attr))
    if(xError)
    {
        ADM_warning("Failed to query number of surface attributes, destroying config.\n");
        CHECK_ERROR(vaDestroyConfig(ADM_coreLibVA::display, dconf->cid))
        dconf->cid = VA_INVALID;
        return false;
    }
    VASurfaceAttrib *alist = (VASurfaceAttrib *)admAlloca(nb_attr * sizeof(VASurfaceAttrib));
    ADM_assert(alist);

    CHECK_ERROR(vaQuerySurfaceAttributes(ADM_coreLibVA::display, dconf->cid, alist, &nb_attr))
    if(xError)
    {
        ADM_warning("Failed to query surface attributes, destroying config.\n");
        CHECK_ERROR(vaDestroyConfig(ADM_coreLibVA::display, dconf->cid))
        dconf->cid = VA_INVALID;
        return false;
    }

    for (int k = 0; k < nb_attr; k++)
    {
        VASurfaceAttrib *a = &(alist[k]);
        switch (a->type)
        {
            case VASurfaceAttribMinWidth:
                dconf->min_width = a->value.value.i;
                break;
            case VASurfaceAttribMinHeight:
                dconf->min_height = a->value.value.i;
                break;
            case VASurfaceAttribMaxWidth:
                dconf->max_width = a->value.value.i;
                break;
            case VASurfaceAttribMaxHeight:
                dconf->max_height = a->value.value.i;
                break;
            default:break;
        }
    }
    ADM_info("Config %s constraints: %d x %d -- %d x %d\n", name, dconf->min_width, dconf->min_height, dconf->max_width, dconf->max_height);
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
    VAProfile *prof=(VAProfile *)admAlloca(sizeof(VAProfile)*nb);
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

    checkProfile(VAProfileMPEG2Main,    &ADM_coreLibVA::configMpeg2,    "MPEG-2 Main");
    checkProfile(VAProfileH264High,     &ADM_coreLibVA::configH264,     "H264 High");
    checkProfile(VAProfileVC1Advanced,  &ADM_coreLibVA::configVC1 ,     "VC1");
#ifdef LIBVA_HEVC_DEC
    checkProfile(VAProfileHEVCMain,     &ADM_coreLibVA::configH265,     "HEVC Main");
    checkProfile(VAProfileHEVCMain10,   &ADM_coreLibVA::configH26510Bits,"HEVC 10Bits");
#endif

#ifdef LIBVA_VP9_DEC
    checkProfile(VAProfileVP9Profile0,  &ADM_coreLibVA::configVP9 ,     "VP9");
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
 *
 * @return
 */
VADisplay admLibVA::getDisplay()
{
        return ADM_coreLibVA::display;
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
                    ADM_info("NV12 is supported\n");
                    break;
                case VA_FOURCC_YV12:
                    ADM_coreLibVA::imageFormatYV12=list[i];
                    ADM_info("YV12 is supported\n");
                    r=true;
                    break;
                default:break;
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
    \fn     init
    \brief
*/
bool admLibVA::init(GUI_WindowInfo *x)
{
    Display *dis=(Display *)x->display;
    ADM_coreLibVA::display=vaGetDisplay(dis);
    ADM_info("[LIBVA] Initializing LibVA library ...\n");


    ADM_coreLibVA::context=NULL;
    ADM_coreLibVA::decoders::h264=false;
    ADM_coreLibVA::directOperation=true;
    ADM_coreLibVA::transferMode=ADM_LIBVA_NONE;
    ADM_coreLibVA::driverQuirks=ADM_LIBVA_DRIVER_QUIRK_NONE;
    ADM_coreLibVA::decoderConfig *c;

#define INVALIDATE(x) { c=&ADM_coreLibVA::x; c->cid=VA_INVALID; c->min_width=-1; c->min_height=-1; c->max_width=-1; c->max_height=-1; }
    INVALIDATE(configH264)
    INVALIDATE(configMpeg2)
    INVALIDATE(configH265)
    INVALIDATE(configH26510Bits)
    INVALIDATE(configVP9)
    INVALIDATE(configVC1)
#undef INVALIDATE
    myWindowInfo=*x;
    VAStatus xError;
    int majv,minv;
    CHECK_ERROR(vaInitialize(ADM_coreLibVA::display,&majv,&minv));
    if(xError)
    {
        ADM_warning("VA: init failed\n");
        return false;
    }

    const char *vendorString = vaQueryVendorString(ADM_coreLibVA::display);
    ADM_info("VA %d.%d, Vendor = %s\n",majv,minv,vendorString);
    if(vendorString)
    {
        if(strstr(vendorString, "ubit"))
        {
            ADM_coreLibVA::driverQuirks = ADM_LIBVA_DRIVER_QUIRK_ATTRIB_MEMTYPE;
            ADM_info("Not setting VASurfaceAttribMemoryType attribute when allocating surfaces with this driver.\n");
        }else if(strstr(vendorString, "Splitted-Desktop Systems VDPAU backend for VA-API"))
        {
            ADM_coreLibVA::driverQuirks = ADM_LIBVA_DRIVER_QUIRK_SURFACE_ATTRIBUTES;
            ADM_info("Not setting any surface attributes with this driver.\n");
        }else
        {
            ADM_info("Using standard behavior with this driver.\n");
        }
    }

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
bool admLibVA::supported(VAProfile profile, int width, int height)
{
    bool result = false;
    ADM_coreLibVA::decoderConfig *c = NULL;
#define SUPSUP(a,b) case a: c = &ADM_coreLibVA::b; if(c->cid != VA_INVALID) result = true; break;
    switch(profile)
    {
        SUPSUP(VAProfileMPEG2Main,configMpeg2)
        SUPSUP(VAProfileH264High,configH264)
        SUPSUP(VAProfileVC1Advanced,configVC1)
#ifdef LIBVA_HEVC_DEC
        SUPSUP(VAProfileHEVCMain,configH265)
        SUPSUP(VAProfileHEVCMain10,configH26510Bits)
#endif

#ifdef LIBVA_VP9_DEC
        SUPSUP(VAProfileVP9Profile0,configVP9)
#endif
        default:
            ADM_info("Unknown libva profile ID %d\n", profile);
            break;
    }
    if(result)
    {
        ADM_assert(c);
        if((c->min_width > 0 && width > 0 && width < c->min_width) ||
           (c->min_height > 0 && height > 0 && height < c->min_height) ||
           (c->max_width > 0 && width > c->max_width) ||
           (c->max_height > 0 && height > c->max_height))
        {
            ADM_info("Dimensions %d x %d not supported by hw decoder for this profile.\n", width, height);
            return false;
        }
    }
    return result;
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
    ADM_coreLibVA::decoderConfig *cfg = NULL;

    switch(profile)
    {
       case VAProfileMPEG2Main:   cfg = &ADM_coreLibVA::configMpeg2;break;
       case VAProfileH264High:    cfg = &ADM_coreLibVA::configH264;break;
       case VAProfileVC1Advanced: cfg = &ADM_coreLibVA::configVC1;break;
#ifdef LIBVA_HEVC_DEC
       case VAProfileHEVCMain:    cfg = &ADM_coreLibVA::configH265;break;
       case VAProfileHEVCMain10:  cfg = &ADM_coreLibVA::configH26510Bits;break;
#endif
#ifdef LIBVA_VP9_DEC
       case VAProfileVP9Profile0: cfg = &ADM_coreLibVA::configVP9;break;
#endif
       default:
                ADM_assert(0);
                break;

    }

    ADM_assert(cfg);

    if(cfg->cid == VA_INVALID)
    {
        ADM_warning("No VA support for that\n");
        return VA_INVALID;
    }
    bool failure = false;
    if(cfg->min_width > 0 && width < cfg->min_width)
    {
        ADM_warning("Width %d less than minimum width %d supported by VA-API hw decoder.\n", width, cfg->min_width);
        failure = true;
    }
    if(cfg->min_height > 0 && height < cfg->min_height)
    {
        ADM_warning("Height %d less than minimum height %d supported by VA-API hw decoder.\n", height, cfg->min_height);
        failure = true;
    }
    if(cfg->max_width > 0 && width > cfg->max_width)
    {
        ADM_warning("Width %d exceeds maximum width %d supported by VA-API hw decoder.\n", width, cfg->max_width);
        failure = true;
    }
    if(cfg->max_height > 0 && height < cfg->max_height)
    {
        ADM_warning("Height %d exceeds maximum height %d supported by VA-API hw decoder.\n", height, cfg->max_height);
        failure = true;
    }
    if(failure)
        return VA_INVALID;

    CHECK_ERROR(vaCreateContext ( ADM_coreLibVA::display, cfg->cid,
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
    listOfAllocatedVAImage[image->image_id]=true;
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
    listOfAllocatedVAImage[image->image_id]=true;
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
    if(listOfAllocatedVAImage.end()==listOfAllocatedVAImage.find(image->image_id))
    {
        ADM_warning("Trying to destroy an unallocated VAImage\n");
        ADM_assert(0);
    }
    listOfAllocatedVAImage.erase(image->image_id);

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
VASurfaceID        admLibVA::allocateSurface(int w, int h, int fmt)
{
    int xError;
    CHECK_WORKING(VA_INVALID)

    aprintf("Creating surface %d x %d (fmt=%d)\n",w,h,fmt);
    VASurfaceID s;
    VASurfaceAttrib *attr = NULL;
    unsigned int nbAttr = 2;
    switch(ADM_coreLibVA::driverQuirks)
    {
        case ADM_LIBVA_DRIVER_QUIRK_ATTRIB_MEMTYPE:
            nbAttr = 1;
            break;
        case ADM_LIBVA_DRIVER_QUIRK_SURFACE_ATTRIBUTES:
            nbAttr = 0;
            break;
        case ADM_LIBVA_DRIVER_QUIRK_NONE:
        default:break;
    }
    if(nbAttr)
    {
        int fcc;
        switch(fmt)
        {
            case VA_RT_FORMAT_YUV420:    fcc = VA_FOURCC_NV12; break;
            case VA_RT_FORMAT_YUV420_10: fcc = VA_FOURCC_I010; break; // UV swapped?
            case VA_RT_FORMAT_YUV422:    fcc = VA_FOURCC_422H; break;
            case VA_RT_FORMAT_YUV444:    fcc = VA_FOURCC_444P; break;
            case VA_RT_FORMAT_RGB32:     fcc = VA_FOURCC_BGRX; break;
            default:
                ADM_warning("Unsupported format 0x%08x requested\n",fmt);
                return VA_INVALID;
        }
        attr = (VASurfaceAttrib *)admAlloca(sizeof(VASurfaceAttrib) * nbAttr);
        attr->type = VASurfaceAttribPixelFormat;
        attr->flags = VA_SURFACE_ATTRIB_SETTABLE;
        attr->value.type = VAGenericValueTypeInteger;
        attr->value.value.i = fcc;

        if(ADM_coreLibVA::driverQuirks != ADM_LIBVA_DRIVER_QUIRK_ATTRIB_MEMTYPE)
        {
            attr++;
            attr->type = VASurfaceAttribMemoryType;
            attr->flags = VA_SURFACE_ATTRIB_SETTABLE;
            attr->value.type = VAGenericValueTypeInteger;
            attr->value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA;
            attr--;
        }
    }

    CHECK_ERROR(vaCreateSurfaces(ADM_coreLibVA::display, fmt, w, h, &s, 1, attr, nbAttr))

    if(!xError)
    {
        surfaceList::iterator already;
        already=listOfAllocatedSurface.find(s);
        if(already!=listOfAllocatedSurface.end())
        {
            ADM_warning("Doubly allocated va surface\n");
            ADM_assert(0);
        }
        listOfAllocatedSurface[s]=true;
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

      surfaceList::iterator already;
      already=listOfAllocatedSurface.find(surface);
      if(already==listOfAllocatedSurface.end())
      {
          ADM_warning("Trying to destroy an unallocated surface\n");
          ADM_assert(0);
      }
      listOfAllocatedSurface.erase(surface);
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
    int countDown=50;
    bool end=false;
    while(!end)
    {
     CHECK_ERROR(vaQuerySurfaceStatus ( ADM_coreLibVA::display, src->surface,&status));
     if(xError)
     {
         ADM_warning("QuerySurfacStatus failed\n");
         return false;
     }
     aprintf("surface status = %d\n",status);
     switch(status)
     {
        case VASurfaceReady:
                end=true;
                continue;
                break;
        case VASurfaceSkipped:
                end=true;
                continue;
                break;
      default:
          countDown--;
          if(!countDown)
          {
              ADM_warning("Timeout waiting for surface\n");
              end=true;
              continue;
          }
          ADM_usleep(1000);
          break;
    }
   }
    if(status!=VASurfaceReady)
    {
      ADM_warning("Error getting surface within timeout = %d\n",(int)status);
      dest->_noPicture=true;
      return true;
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
        case VA_FOURCC_P010:break;
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
#ifdef VA_10BITS_IS_ACTUALL_8BITS                    
                case VA_FOURCC_P010:
#endif                    
                {
#if 0
                    dest->convertFromNV12(ptr+vaImage.offsets[0],ptr+vaImage.offsets[1], vaImage.pitches[0], vaImage.pitches[1]);
#else
                    ADMColorScalerSimple *color=src->fromNv12ToYv12;
                    if(!color)
                        color = new ADMColorScalerSimple(src->w, src->h, ADM_PIXFRMT_NV12, ADM_PIXFRMT_YV12);
                    ADMImageRef ref(dest->_width,dest->_height);
                    for(int i=0;i<2;i++)
                    {
                            ref._planes[i]= ptr+vaImage.offsets[i];
                            ref._planeStride[i]=vaImage.pitches[i];
                    }
                    ref._planes[2]=NULL;
                    ref._planeStride[2]=0;
                    color->convertImage(&ref,dest);
                    src->fromNv12ToYv12=color;
#endif
                    break;
                }
#ifndef VA_10BITS_IS_ACTUALL_8BITS                
                case VA_FOURCC_P010: // It is actually NV12 style All Y, then U/V interleaved
                {
                    ADMColorScalerSimple *color=src->color10bits;
                    if(!color)
                        color = new ADMColorScalerSimple(src->w, src->h, ADM_PIXFRMT_NV12_10BITS, ADM_PIXFRMT_YV12);
                    ADMImageRef ref(dest->_width,dest->_height);
                    for(int i=0;i<2;i++)
                    {
                            ref._planes[i]= ptr+vaImage.offsets[i];
                            ref._planeStride[i]=vaImage.pitches[i];
                    }
                    ref._planes[2]=NULL;
                    ref._planeStride[2]=0;
                    color->convertImage(&ref,dest);
                    src->color10bits=color;
                    break;
                }
#endif                
                default:
                    goto dropIt;
                    break;
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
bool        admLibVA::putX11Surface(ADM_vaSurface *img,int widget,int sourceWidth,int sourceHeight,int displayWidth,int displayHeight)
{
    int xError;
    CHECK_WORKING(false);
    CHECK_ERROR(vaPutSurface ( ADM_coreLibVA::display, img->surface, (Drawable)widget, 0, 0,
                              sourceWidth, sourceHeight, 0, 0, displayWidth, displayHeight,
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
 * \fn downloadFromImage
 * @param dest
 * @param src
 * @return
 */
bool   admLibVA::downloadFromImage( ADMImage *src,VAImage *dest,ADM_vaSurface *face)
{
    int xError;
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
        case VA_FOURCC_P010: // It is actually NV12 style All Y, then U/V interleaved
                {
                    ADMColorScalerSimple *color = face ? face->color10bits : NULL;
                    if(!color)
                        color = new ADMColorScalerSimple(src->_width, src->_height, ADM_PIXFRMT_NV12_10BITS, ADM_PIXFRMT_YV12);
                    ADMImageRef ref(src->_width,src->_height);
                    for(int i=0;i<2;i++)
                    {
                            ref._planes[i]= ptr+dest->offsets[i];
                            ref._planeStride[i]=dest->pitches[i];
                    }
                    ref._planes[2]=NULL;
                    ref._planeStride[2]=0;
                    color->convertImage(&ref,src);
                    if(face)
                        face->color10bits=color;
                    else
                        delete color;
                    color = NULL;
                    break;
                }
        case VA_FOURCC_NV12:
                {
#if 0
                    src->convertFromNV12(  ptr+dest->offsets[0], ptr+dest->offsets[1],dest->pitches[0],dest->pitches[1]);
#else
                    ADMColorScalerSimple *color = face ? face->fromNv12ToYv12 : NULL;
                    if(!color)
                        color = new ADMColorScalerSimple(src->_width, src->_height, ADM_PIXFRMT_NV12, ADM_PIXFRMT_YV12);
                    ADMImageRef ref(src->_width,src->_height);
                    for(int i=0;i<2;i++)
                    {
                            ref._planes[i]= ptr+dest->offsets[i];
                            ref._planeStride[i]=dest->pitches[i];
                    }
                    ref._planes[2]=NULL;
                    ref._planeStride[2]=0;
                    color->convertImage(&ref,src);
                    if(face)
                        face->fromNv12ToYv12=color;
                    else
                        delete color;
                    color = NULL;
#endif
                    break;
                }
        default: ADM_assert(0);
    }
    CHECK_ERROR(vaUnmapBuffer (ADM_coreLibVA::display,dest->buf));
    return true;
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
                }
                break;
            case VA_FOURCC_NV12:
                src->convertToNV12(ptr+vaImage.offsets[0], ptr+vaImage.offsets[1], vaImage.pitches[0], vaImage.pitches[1]);
                break;
            default:
                ADM_warning("Unknown format %s\n",fourCC_tostring(vaImage.format.fourcc));
                break;
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
 * allocateWithSurface
 * @param w
 * @param h
 * @return
 */
ADM_vaSurface *ADM_vaSurface::allocateWithSurface(int w,int h,int fmt)
{
    ADM_vaSurface *s=new ADM_vaSurface(w,h);
    s->surface=admLibVA::allocateSurface(w,h,fmt);
    if(!s->hasValidSurface())
    {
        delete s;
        s=NULL;
        ADM_warning("Cannot allocate va surface\n");
        return NULL;
    }
    return s;
}

/**
 *
 * @return
 */
bool ADM_vaSurface_cleanupCheck(void)
{
    int n=listOfAllocatedSurface.size();
    if(!n) return true;

    ADM_warning("Some allocated va surface are still in use (%d), clearing them\n",n);
    return true;

}
/**
 *
 * @return
 */
bool ADM_vaImage_cleanupCheck(void)
{
    int n=listOfAllocatedVAImage.size();
    if(!n) return true;

    ADM_warning("Some allocated va images are still in use (%d), clearing them\n",n);
    return true;

}
/**
 * 
 * @param w
 * @param h
 */
 ADM_vaSurface::ADM_vaSurface(int w, int h)
{
    surface=VA_INVALID;
    refCount=0;
    this->w=w;
    this->h=h;
    image=admLibVA::allocateImage(w,h);
    fromNv12ToYv12=NULL;
    color10bits=NULL;
}
/**
 * 
 */ 
ADM_vaSurface:: ~ADM_vaSurface()
 {
     if(surface!=VA_INVALID)
     {
        admLibVA::destroySurface(surface);
        surface=VA_INVALID;
     }
     if(image)
     {
         admLibVA::destroyImage(image);
         image=NULL;
     }
     if(fromNv12ToYv12)
     {
         delete fromNv12ToYv12;
         fromNv12ToYv12=NULL;
     }
     if(color10bits)
     {
         delete color10bits;
         color10bits=NULL;
     }
 }

/**
 * \fn admLibVa_exitCleanup
 */
bool admLibVa_exitCleanup()
{
    ADM_info("VA cleanup begin\n");
    ADM_vaSurface_cleanupCheck();
    ADM_vaImage_cleanupCheck();
    admLibVA::cleanup();
    ADM_info("VA cleanup end\n");
    return true;
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
                        return  admLibVA::downloadFromImage(dest,this->image,this);
                return false;
                break;
    default:ADM_assert(0);
    }
    return false;
}
/**
 *
 * @return
 */
VAConfigID  admLibVA::createFilterConfig()
{
      VAStatus xError;
      VAConfigID id=VA_INVALID;

      if(!coreLibVAWorking) {ADM_warning("Libva not operationnal\n");return VA_INVALID;}

      CHECK_ERROR(vaCreateConfig(ADM_coreLibVA::display, VAProfileNone, VAEntrypointVideoProc, 0, 0, &id));
      if(xError!=VA_STATUS_SUCCESS)
          return VA_INVALID;
      return id;
}

/**
 *
 * @return
 */
VAContextID admLibVA::createFilterContext()
{
    return VA_INVALID;
}
/**
 *
 * @param id
 * @return
 */
bool        admLibVA::destroyFilterContext(VAContextID &id)
{
     VAStatus xError;
     if(!coreLibVAWorking) {ADM_warning("Libva not operationnal\n");return false;}

    CHECK_ERROR( vaDestroyContext(ADM_coreLibVA::display, id));
    id=VA_INVALID;
    return true;
}

/**
 *
 * @return
 */
bool        admLibVA::destroyFilterConfig(VAConfigID &id)
{
    VAStatus xError;
     if(!coreLibVAWorking) {ADM_warning("Libva not operationnal\n");return false;}

    CHECK_ERROR( vaDestroyConfig(ADM_coreLibVA::display, id));
    id=VA_INVALID;
    return true;
}


#endif
