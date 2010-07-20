/**
    \brief VDPAU filters Deinterlacer
    \author mean (C) 2010
    This is slow as we copy back and forth data to/from the video cards
    

*/
#include "ADM_default.h"
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
    \class vdpauVideoFilterDeint
*/
class vdpauVideoFilterDeint : public  ADM_coreVideoFilter
{
protected:
                    ADMColorScalerSimple *scaler;
                    bool                 passThrough;
                    bool                 setupVdpau(void);
                    bool                 cleanupVdpau(void);

                    uint8_t             *tempBuffer;
                    vdpauFilter          configuration;
                    VdpOutputSurface     surface;
                    VdpVideoSurface      input[ADM_NB_SURFACES];
                    uint32_t             frameDesc[ADM_NB_SURFACES];
                    uint32_t             currentIndex;
                    VdpVideoMixer        mixer;
                    bool                 uploadImage(ADMImage *next,uint32_t surfaceIndex,uint32_t frameNumber) ;

public:
        virtual bool         goToTime(uint64_t usSeek); 
                             vdpauVideoFilterDeint(ADM_coreVideoFilter *previous,CONFcouple *conf);
                             ~vdpauVideoFilterDeint();

        virtual const char   *getConfiguration(void);                 /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);           /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;                        /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   vdpauVideoFilterDeint,   // Class
                        1,0,0,              // Version
                        ADM_UI_GTK+ADM_UI_QT4,     // We need a display for VDPAU; so no cli...
                        VF_INTERLACING,            // Category
                        "vdpauDeint",            // internal name (must be uniq!)
                        "vdpauDeint",            // Display name
                        "VDPAU deinterlacer." // Description
                    );

//
/**
    \fn goToTime
    \brief called when seeking. Need to cleanup our stuff.
*/
bool         vdpauVideoFilterDeint::goToTime(uint64_t usSeek)
{
    for(int i=0;i<ADM_NB_SURFACES;i++)             frameDesc[i]=ADM_INVALID_FRAME_NUM;
    currentIndex=0;
    return ADM_coreVideoFilter::goToTime(usSeek);
}

/**
    \fn resetVdpau
*/
bool vdpauVideoFilterDeint::setupVdpau(void)
{
    scaler=NULL;
    for(int i=0;i<ADM_NB_SURFACES;i++)             frameDesc[i]=ADM_INVALID_FRAME_NUM;
    currentIndex=0;
    if(!admVdpau::isOperationnal())
    {
        ADM_warning("Vdpau not operationnal\n");
        return false;
    }   
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceCreate(VDP_RGBA_FORMAT_B8G8R8A8,
                        info.width,info.height,&surface)) 
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
                                            previousFilter->getInfo()->height,&mixer,true)) 
    {
        ADM_error("Cannot create mixer\n");
        goto badInit;
    } 
    tempBuffer=new uint8_t[info.width*info.height*4];
    scaler=new ADMColorScalerSimple( info.width,info.height, ADM_COLOR_BGR32A,ADM_COLOR_YV12);
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
bool vdpauVideoFilterDeint::cleanupVdpau(void)
{
    for(int i=0;i<ADM_NB_SURFACES;i++)
        if(input[i]!=VDP_INVALID_HANDLE) admVdpau::surfaceDestroy(input[i]);
    if(surface!=VDP_INVALID_HANDLE)  admVdpau::outputSurfaceDestroy(surface);
    if(mixer!=VDP_INVALID_HANDLE) admVdpau::mixerDestroy(mixer);
    surface=VDP_INVALID_HANDLE;
    for(int i=0;i<ADM_NB_SURFACES;i++)
        input[i]=VDP_INVALID_HANDLE;
    mixer=VDP_INVALID_HANDLE;
    if(tempBuffer) delete [] tempBuffer;
    tempBuffer=NULL;
    if(scaler) delete scaler;
    scaler=NULL;
    return true;
}

/**
    \fn constructor
*/
vdpauVideoFilterDeint::vdpauVideoFilterDeint(ADM_coreVideoFilter *in, CONFcouple *setup): ADM_coreVideoFilter(in,setup)
{
    for(int i=0;i<ADM_NB_SURFACES;i++)
        input[i]=VDP_INVALID_HANDLE;
    mixer=VDP_INVALID_HANDLE;
    surface=VDP_INVALID_HANDLE;
    if(!setup || !ADM_paramLoad(setup,vdpauFilter_param,&configuration))
    {
        // Default value
        configuration.targetWidth=info.width;
        configuration.targetHeight=info.height;
    }
    vidCache = new VideoCache (5, in);
    myName="vdpauDeint";
    tempBuffer=NULL;
    passThrough=!setupVdpau();
    
    
}
/**
    \fn destructor
*/
vdpauVideoFilterDeint::~vdpauVideoFilterDeint()
{
//        delete original;
        delete vidCache;
        vidCache = NULL;
        cleanupVdpau();
}
/**
    \fn updateInfo
*/
bool vdpauVideoFilterDeint::configure( void) 
{
    
     
        cleanupVdpau();
        passThrough=!setupVdpau();
        return 1;
     
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         vdpauVideoFilterDeint::getCoupledConf(CONFcouple **couples)
{
    *couples=NULL;
    return true;
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *vdpauVideoFilterDeint::getConfiguration(void)
{
    static char conf[80];
    sprintf(conf,"Vdpau Deinterlace.");
    conf[79]=0;
    return conf;
}
/**
    \fn uploadImage
    \brief upload an image to a vdpau surface
*/
bool vdpauVideoFilterDeint::uploadImage(ADMImage *next,uint32_t surfaceIndex,uint32_t frameNumber) 
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
#if VDP_DEBUG
    printf("Uploading image to surface %d\n",surfaceIndex%ADM_NB_SURFACES);
#endif
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
    The "input" arrays contains

        T0 B0 T1 B1 T2 B2
              ^ CurrentIndex
    So in most case we have at least 2 fiels in the past and 2 in the future


*/
bool vdpauVideoFilterDeint::getNextFrame(uint32_t *fn,ADMImage *image)
{
    
     if(passThrough) return previousFilter->getNextFrame(fn,image);
    ADMImage *prev=NULL;
    // our first frame, we need to send it + the next one
    if(!nextFrame)
    {
            ADMImage *prev= vidCache->getImage( 0);
            // Upload top field
            if(false==uploadImage(prev,currentIndex,0)) 
            {
                vidCache->unlockAll();
                return false;
            }

    }
    // regular image, in fact we get the next image here
    ADMImage *next= vidCache->getImage(nextFrame+1);
    if(false==uploadImage(next,currentIndex+1,nextFrame+1)) 
            {
                vidCache->unlockAll();
                return false;
            }
   
    // Call mixer...
    VdpVideoSurface in[3];
        // PREVIOUS
    if(!nextFrame) // First image, we dont have previous
    {
             in[0]=VDP_INVALID_HANDLE;
    }else
    {
             in[0]=input[(currentIndex+ADM_NB_SURFACES-1)%ADM_NB_SURFACES];
    }
        // CURRENT
     in[1]=input[currentIndex%ADM_NB_SURFACES];
        // NEXT
    if(next)
    {
     in[2]=input[(currentIndex+1)%ADM_NB_SURFACES];
    }
    else
    {
      in[2]=VDP_INVALID_HANDLE;
    }

    //
#if VDP_DEBUG
    printf("Current index=%d\n",(int)currentIndex);
    for(int i=0;i<3;i++) printf("Calling with in[%d]=%d\n",i,in[i]);
    for(int i=0;i<3;i++) printf("Desc[%d]=%d\n",i, frameDesc[i]);
#endif
    // ---------- Top field ------------
    if(VDP_STATUS_OK!=admVdpau::mixerRenderWithPastAndFuture(true, 
                mixer,
                in,
                surface, 
                info.width,info.height))

    {
        ADM_warning("[Vdpau] Cannot mixerRender\n");
        vidCache->unlockAll();
        return false;
    }
    // Now get our image back from surface...
    // Top Field..
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceGetBitsNative(surface,
                                                            tempBuffer, 
                                                            info.width,info.height))
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

    ts=destStride[2];destStride[2]=destStride[1];destStride[1]=ts;
    td=destData[2];destData[2]=destData[1];destData[1]=td;

    scaler->convertPlanes(  sourceStride,destStride,     
                            sourceData,destData);
    nextFrame++;
    currentIndex+=2; // Two fields at a time...
    vidCache->unlockAll();
    return true;
}
#endif

//****************
// EOF
