/**
    \brief VDPAU filters
    \author mean (C) 2010
    This is slow as we copy back and forth data to/from the video cards
    Only 1 filter exposed at the moment : resize...

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
/**
    \class vdpauVideoFilter
*/
class vdpauVideoFilter : public  ADM_coreVideoFilter
{
protected:
                    ADMColorScalerSimple *scaler;
                    bool passThrough;
                    bool setupVdpau(void);
                    bool cleanupVdpau(void);

                    uint8_t   *tempBuffer;
                    ADMImage *original;
                    vdpauFilter configuration;
                    VdpOutputSurface     surface;
                    VdpVideoSurface      input;
                    VdpVideoMixer        mixer;


public:
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
                        VF_INTERLACING,            // Category
                        "vdpau",            // internal name (must be uniq!)
                        "vdpau",            // Display name
                        "vdpau, vdpau filters." // Description
                    );

//
/**
    \fn resetVdpau
*/
bool vdpauVideoFilter::setupVdpau(void)
{
    scaler=NULL;
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
   if(VDP_STATUS_OK!=admVdpau::surfaceCreate(   previousFilter->getInfo()->width,
                                                previousFilter->getInfo()->height,&input)) 
    {
        ADM_error("Cannot create input Surface\n");
        goto badInit;
    }
    if(VDP_STATUS_OK!=admVdpau::mixerCreate(previousFilter->getInfo()->width,
                                            previousFilter->getInfo()->height,&mixer)) 
    {
        ADM_error("Cannot create mixer\n");
        goto badInit;
    } 
    tempBuffer=new uint8_t[info.width*info.height*4];
    scaler=new ADMColorScalerSimple( info.width,info.height, ADM_COLOR_BGR32A,ADM_COLOR_YV12);
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
    if(input!=VDP_INVALID_HANDLE) admVdpau::surfaceDestroy(input);
    if(surface!=VDP_INVALID_HANDLE)  admVdpau::outputSurfaceDestroy(surface);
    if(mixer!=VDP_INVALID_HANDLE) admVdpau::mixerDestroy(mixer);
    surface=VDP_INVALID_HANDLE;
    input=VDP_INVALID_HANDLE;
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
    input=VDP_INVALID_HANDLE;
    mixer=VDP_INVALID_HANDLE;
    surface=VDP_INVALID_HANDLE;
    if(!setup || !ADM_paramLoad(setup,vdpauFilter_param,&configuration))
    {
        // Default value
        configuration.resizeToggle=false;
        configuration.deinterlace=false;
        configuration.targetWidth=info.width;
        configuration.targetHeight=info.height;
    }
    vidCache = new VideoCache (5, in);
    original=new ADMImageDefault(previousFilter->getInfo()->width,previousFilter->getInfo()->height);
    myName="vdpau";
    tempBuffer=NULL;
    passThrough=!setupVdpau();
    
    
}
/**
    \fn destructor
*/
vdpauVideoFilter::~vdpauVideoFilter()
{
        delete original;
        delete vidCache;
        vidCache = NULL;
        cleanupVdpau();
}
/**
    \fn updateInfo
*/
bool vdpauVideoFilter::configure( void) 
{
    
     uint32_t resize=configuration.resizeToggle;
     uint32_t deinterlace=configuration.deinterlace;
  
     diaElemToggle resizeT(&(resize),   QT_TR_NOOP("Resize:"));
     diaElemToggle deinterlaceT(&(deinterlace),   QT_TR_NOOP("Deinterlace:"));
     
     diaElemUInteger  tWidth(&(configuration.targetWidth),QT_TR_NOOP("Width :"),16,2048);
     diaElemUInteger  tHeight(&(configuration.targetHeight),QT_TR_NOOP("Height :"),16,2048);
     
     diaElem *elems[]={&resizeT,&deinterlaceT,&tWidth,&tHeight};
     
     if(diaFactoryRun(QT_TR_NOOP("vdpau"),sizeof(elems)/sizeof(diaElem *),elems))
     {
         configuration.resizeToggle=resize;
         configuration.deinterlace=deinterlace;
         if(resize)
         {
                info.width=configuration.targetWidth;
                info.height=configuration.targetHeight;
         }
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
    if(configuration.resizeToggle)
    {
        sprintf(conf2,"%d x %d -> %d x %d",
                        previousFilter->getInfo()->width, 
                        previousFilter->getInfo()->height,
                        info.width,info.height);
        strcat(conf,conf2);
    }
    if(configuration.deinterlace)
    {
        sprintf(conf2," deinterlace");
        strcat(conf,conf2);

    }
    conf[79]=0;
    return conf;
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
bool vdpauVideoFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // 1- Get image...
      if(passThrough) return previousFilter->getNextFrame(fn,image);
    // 
     if(false==previousFilter->getNextFrame(fn,original)) return false;
     // Blit our image to surface
    uint32_t pitches[3];
    uint8_t *planes[3];
    original->GetPitches(pitches);
    original->GetReadPlanes(planes);

    // Put out stuff in input...

    if(VDP_STATUS_OK!=admVdpau::surfacePutBits( 
            input,
            planes,pitches))
    {
        ADM_warning("[Vdpau] video surface : Cannot putbits\n");
        return false;
    }

    // Call mixer...
    if(VDP_STATUS_OK!=admVdpau::mixerRender( mixer,input,surface, info.width,info.height))

    {
        ADM_warning("[Vdpau] Cannot mixerRender\n");
        return false;
    }
    // Now get our image back from surface...
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceGetBitsNative(surface,tempBuffer, info.width,info.height))
    {
        ADM_warning("[Vdpau] Cannot copy back data from output surface\n");
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
      return true;
}
#endif
//****************
// EOF
