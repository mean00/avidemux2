/**
    \brief VDPAU filters
    \author mean (C) 2010
    This is slow as we copy back and forth data to/from the video cards
    Only 1 filter exposed at the moment : resize...

*/

#include "ADM_default.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/vdpau.h"
}

#include "ADM_coreVideoFilterInternal.h"
#include "ADM_videoFilterCache.h"
#include "DIA_factory.h"
#include "vdpauFilter.h"
#include "vdpauFilter_desc.cpp"
#ifdef USE_VDPAU



#include "ADM_coreVdpau/include/ADM_coreVdpau.h"
//
#define ADM_INVALID_FRAME_NUM 0x80000000
#define ADM_NB_SURFACES 3

/**
    \class vdpauVideoFilter
*/
class vdpauVideoFilter : public  ADM_coreVideoFilter
{
protected:
                    ADMColorScalerSimple *scaler;
                    bool                 passThrough;
                    bool                 setupVdpau(void);
                    bool                 cleanupVdpau(void);

                    uint8_t             *tempBuffer;
                    vdpauFilter          configuration;
                    VdpOutputSurface     outputSurface;
                    VdpVideoSurface      input[ADM_NB_SURFACES];
                    uint32_t             frameDesc[ADM_NB_SURFACES];
                    uint32_t             currentIndex;
                    VdpVideoMixer        mixer;
                    bool                 uploadImage(ADMImage *next,uint32_t surfaceIndex,uint32_t frameNumber) ;

public:
        virtual bool         goToTime(uint64_t usSeek); 
                             vdpauVideoFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                             ~vdpauVideoFilter();

        virtual const char   *getConfiguration(void);                 /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);           /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;                        /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   vdpauVideoFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_GTK+ADM_UI_QT4,     // We need a display for VDPAU; so no cli...
                        VF_TRANSFORM,            // Category
                        "vdpauResize",            // internal name (must be uniq!)
                        "vdpau: Resize",            // Display name
                        "vdpau: Resize image using vdpau." // Description
                    );

//
/**
    \fn goToTime
    \brief called when seeking. Need to cleanup our stuff.
*/
bool         vdpauVideoFilter::goToTime(uint64_t usSeek)
{
    return ADM_coreVideoFilter::goToTime(usSeek);
}

/**
    \fn resetVdpau
*/
bool vdpauVideoFilter::setupVdpau(void)
{
    scaler=NULL;

    info.width=configuration.targetWidth;
    info.height=configuration.targetHeight;
    for(int i=0;i<ADM_NB_SURFACES;i++)             frameDesc[i]=ADM_INVALID_FRAME_NUM;
    currentIndex=0;
    if(!admVdpau::isOperationnal())
    {
        ADM_warning("Vdpau not operationnal\n");
        return false;
    }
    // check if we have something to do
    if(info.width==previousFilter->getInfo()->width &&  info.height==previousFilter->getInfo()->height)
        return false;
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceCreate(VDP_RGBA_FORMAT_B8G8R8A8,
                        info.width,info.height,&outputSurface)) 
    {
        ADM_error("Cannot create outputSurface0\n");
        return false;
    }
    for(int i=0;i<ADM_NB_SURFACES;i++)
    {
        if(VDP_STATUS_OK!=admVdpau::surfaceCreate(   previousFilter->getInfo()->width,
                                                    previousFilter->getInfo()->height,input+i)) 
        {
            ADM_error("Cannot create input Surface %d\n",i);
            goto badInit;
        }
    }
    if(VDP_STATUS_OK!=admVdpau::mixerCreate(previousFilter->getInfo()->width,
                                            previousFilter->getInfo()->height,&mixer)) 
    {
        ADM_error("Cannot create mixer\n");
        goto badInit;
    } 
    tempBuffer=new uint8_t[info.width*info.height*4];
    scaler=new ADMColorScalerSimple( info.width,info.height, ADM_COLOR_RGB32A,ADM_COLOR_YV12);
    ADM_info("VDPAU setup ok\n");
    return true;
badInit:
    cleanupVdpau();
    passThrough=true;
    return false;
}
/**
    \fn cleanupVdpau
*/
bool vdpauVideoFilter::cleanupVdpau(void)
{
    for(int i=0;i<ADM_NB_SURFACES;i++)
        if(input[i]!=VDP_INVALID_HANDLE) admVdpau::surfaceDestroy(input[i]);
    if(outputSurface!=VDP_INVALID_HANDLE)  admVdpau::outputSurfaceDestroy(outputSurface);
    if(mixer!=VDP_INVALID_HANDLE) admVdpau::mixerDestroy(mixer);
    outputSurface=VDP_INVALID_HANDLE;
    for(int i=0;i<ADM_NB_SURFACES;i++)
        input[i]=VDP_INVALID_HANDLE;
    mixer=VDP_INVALID_HANDLE;
    if(tempBuffer) delete [] tempBuffer;
    tempBuffer=NULL;
    if(scaler) delete scaler;
    scaler=NULL;
}

/**
    \fn constructor
*/
vdpauVideoFilter::vdpauVideoFilter(ADM_coreVideoFilter *in, CONFcouple *setup): ADM_coreVideoFilter(in,setup)
{
    for(int i=0;i<ADM_NB_SURFACES;i++)
        input[i]=VDP_INVALID_HANDLE;
    mixer=VDP_INVALID_HANDLE;
    outputSurface=VDP_INVALID_HANDLE;
    if(!setup || !ADM_paramLoad(setup,vdpauFilter_param,&configuration))
    {
        // Default value
        configuration.targetWidth=info.width;
        configuration.targetHeight=info.height;
    }
    vidCache = new VideoCache (5, in);
    myName="vdpau";
    tempBuffer=NULL;
    passThrough=!setupVdpau();
    
    
}
/**
    \fn destructor
*/
vdpauVideoFilter::~vdpauVideoFilter()
{
//        delete original;
        delete vidCache;
        vidCache = NULL;
        cleanupVdpau();
}
/**
    \fn updateInfo
*/
bool vdpauVideoFilter::configure( void) 
{
    
     
     diaElemUInteger  tWidth(&(configuration.targetWidth),QT_TR_NOOP("Width :"),16,2048);
     diaElemUInteger  tHeight(&(configuration.targetHeight),QT_TR_NOOP("Height :"),16,2048);
     
     diaElem *elems[]={&tWidth,&tHeight};
     
     if(diaFactoryRun(QT_TR_NOOP("vdpau"),sizeof(elems)/sizeof(diaElem *),elems))
     {
                info.width=configuration.targetWidth;
                info.height=configuration.targetHeight;
                ADM_info("New dimension : %d x %d\n",info.width,info.height);
                cleanupVdpau();
                passThrough=!setupVdpau();
                return 1;
     }
     return 0;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         vdpauVideoFilter::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, vdpauFilter_param,&configuration);
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *vdpauVideoFilter::getConfiguration(void)
{
    static char conf[80];
    char conf2[80];
    conf2[0]=0;
    sprintf(conf,"vdpau:");
    if(1) //configuration.resizeToggle)
    {
        sprintf(conf2,"%d x %d -> %d x %d",
                        previousFilter->getInfo()->width, 
                        previousFilter->getInfo()->height,
                        info.width,info.height);
        strcat(conf,conf2);
    }
    conf[79]=0;
    return conf;
}
/**
    \fn uploadImage
    \brief upload an image to a vdpau surface
*/
bool vdpauVideoFilter::uploadImage(ADMImage *next,uint32_t surfaceIndex,uint32_t frameNumber) 
{
    if(!next) // empty image
    {
        frameDesc[surfaceIndex%ADM_NB_SURFACES]=ADM_INVALID_FRAME_NUM;
        ADM_warning("No image to upload\n");
        return false;
    }
  // Blit our image to surface
    uint32_t pitches[3];
    uint8_t *planes[3];
    next->GetPitches(pitches);
    next->GetReadPlanes(planes);

    // Put out stuff in input...
    printf("Uploading image to surface %d\n",surfaceIndex%ADM_NB_SURFACES);

    if(VDP_STATUS_OK!=admVdpau::surfacePutBits( 
            input[surfaceIndex%ADM_NB_SURFACES],
            planes,pitches))
    {
        ADM_warning("[Vdpau] video surface : Cannot putbits\n");
        return false;
    }
    frameDesc[surfaceIndex%ADM_NB_SURFACES]=frameNumber;
    return true;
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
bool vdpauVideoFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    
     if(passThrough) return previousFilter->getNextFrame(fn,image);
    // regular image, in fact we get the next image here
    VdpVideoSurface tmpSurface=VDP_INVALID_HANDLE;
    ADMImage *next= vidCache->getImageAs(ADM_HW_VDPAU,nextFrame);
    if(next->refType==ADM_HW_VDPAU)
    {
        
        struct vdpau_render_state *rndr = (struct vdpau_render_state *)next->refDescriptor.refCookie;
        tmpSurface=rndr->surface;
        printf("image is already vdpau %d\n",(int)tmpSurface);
    }else
    {
        printf("Uploading image to vdpau\n");
        if(false==uploadImage(next,0,nextFrame)) 
                {
                    vidCache->unlockAll();
                    return false;
                }
        tmpSurface=input[0];
    }
    
    // Call mixer...
    if(VDP_STATUS_OK!=admVdpau::mixerRender( 
                mixer,
                tmpSurface,
                outputSurface, 
                info.width,info.height))

    {
        ADM_warning("[Vdpau] Cannot mixerRender\n");
        vidCache->unlockAll();
        return false;
    }
    // Now get our image back from surface...
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceGetBitsNative(outputSurface,tempBuffer, info.width,info.height))
    {
        ADM_warning("[Vdpau] Cannot copy back data from output surface\n");
        vidCache->unlockAll();
        return false;
    }
    // Convert from VDP_RGBA_FORMAT_B8G8R8A8 to YV12
    uint32_t sourceStride[3]={info.width*4,0,0};
    uint8_t  *sourceData[3]={tempBuffer,NULL,NULL};
    uint32_t destStride[3];
    uint8_t  *destData[3];

    image->GetPitches(destStride);
    image->GetWritePlanes(destData);

    // Invert U&V
    uint32_t ts;
    uint8_t  *td;
#if 0
    ts=destStride[2];destStride[2]=destStride[1];destStride[1]=ts;
    td=destData[2];destData[2]=destData[1];destData[1]=td;
#endif
    scaler->convertPlanes(  sourceStride,destStride,     
                            sourceData,destData);
    nextFrame++;
    currentIndex++;
    vidCache->unlockAll();
    return true;
}
#endif
//****************
// EOF
