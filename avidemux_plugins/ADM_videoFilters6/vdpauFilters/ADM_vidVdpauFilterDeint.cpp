/**
    \brief VDPAU filters Deinterlacer
    \author mean (C) 2010
    This is slow as we copy back and forth data to/from the video cards
    

*/
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_videoFilterCache.h"
#include "DIA_factory.h"
#include "ADM_vidMisc.h"
#include "vdpauFilterDeint.h"
#include "vdpauFilterDeint_desc.cpp"
#ifdef USE_VDPAU
#include "ADM_coreVdpau/include/ADM_coreVdpau.h"
//
#define ADM_INVALID_FRAME_NUM 0x80000000
#define ADM_NB_SURFACES 3

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

enum
{
    ADM_KEEP_TOP=0,
    ADM_KEEP_BOTTOM=1,
    ADM_KEEP_BOTH=2
};

/**
    \class vdpauVideoFilterDeint
*/
class vdpauVideoFilterDeint : public  ADM_coreVideoFilter
{
protected:
                    bool                 eof;
                    bool                 secondField;
                    uint64_t             nextPts;
                    ADMColorScalerSimple *scaler;
                    bool                 passThrough;
                    bool                 setupVdpau(void);
                    bool                 cleanupVdpau(void);
                    bool                 updateConf(void);
                    uint8_t             *tempBuffer;
                    vdpauFilterDeint     configuration;
                    VdpOutputSurface     surface;
                    VdpVideoSurface      input[ADM_NB_SURFACES];
                    uint32_t             frameDesc[ADM_NB_SURFACES];
                    VdpVideoMixer        mixer;
                    bool                 uploadImage(ADMImage *next,uint32_t surfaceIndex,uint32_t frameNumber) ;
                    bool                 getResult(ADMImage *image);
                    bool                 sendField(bool topField,ADMImage *next);

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
                        "VDPAU deinterlacer (+resize)." // Description
                    );

//

/**
    \fn updateConf
*/
bool vdpauVideoFilterDeint::updateConf(void)
{
    if(passThrough)
    {
        ADM_warning("PassThrough mode\n");
        info=*(previousFilter->getInfo());
        return true;
    }
    if(configuration.resizeToggle)
    {
        info.width=configuration.targetWidth;
        info.height=configuration.targetHeight;
    }else
    {
            info=*(previousFilter->getInfo());
    }
    uint64_t prev=previousFilter->getInfo()->frameIncrement;
    if(configuration.deintMode==ADM_KEEP_BOTH)
        info.frameIncrement=prev/2;
    else
        info.frameIncrement=prev;
    return true;
}
/**
    \fn goToTime
    \brief called when seeking. Need to cleanup our stuff.
*/
bool         vdpauVideoFilterDeint::goToTime(uint64_t usSeek)
{
    secondField=false;
    eof=false;
    for(int i=0;i<ADM_NB_SURFACES;i++)             frameDesc[i]=ADM_INVALID_FRAME_NUM;
    return ADM_coreVideoFilter::goToTime(usSeek);
}

/**
    \fn resetVdpau
*/
bool vdpauVideoFilterDeint::setupVdpau(void)
{
    scaler=NULL;
    secondField=false;
    for(int i=0;i<ADM_NB_SURFACES;i++)             frameDesc[i]=ADM_INVALID_FRAME_NUM;
    nextFrame=0;
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
    eof=false;
    for(int i=0;i<ADM_NB_SURFACES;i++)
        input[i]=VDP_INVALID_HANDLE;
    mixer=VDP_INVALID_HANDLE;
    surface=VDP_INVALID_HANDLE;
    if(!setup || !ADM_paramLoad(setup,vdpauFilterDeint_param,&configuration))
    {
        // Default value
        configuration.resizeToggle=false;
        configuration.deintMode=ADM_KEEP_TOP;
        configuration.targetWidth=info.width;
        configuration.targetHeight=info.height;
    }
    vidCache = new VideoCache (5, in);
    myName="vdpauDeint";
    tempBuffer=NULL;
    passThrough=false;
    updateConf();    
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
     
     diaMenuEntry tMode[]={
                             {ADM_KEEP_TOP,      QT_TR_NOOP("Keep Top Field"),NULL},
                             {ADM_KEEP_BOTTOM,   QT_TR_NOOP("Keep Bottom Field"),NULL},
                             {ADM_KEEP_BOTH,      QT_TR_NOOP("Double framerate"),NULL}
                             
          };
     bool doResize=configuration.resizeToggle;
     diaElemToggle    tResize(&(doResize),   QT_TR_NOOP("_Resize:"));
     diaElemMenu      mMode(&(configuration.deintMode),   QT_TR_NOOP("_Deint Mode:"), 3,tMode);
     diaElemUInteger  tWidth(&(configuration.targetWidth),QT_TR_NOOP("Width :"),16,2048);
     diaElemUInteger  tHeight(&(configuration.targetHeight),QT_TR_NOOP("Height :"),16,2048);
     
     diaElem *elems[]={&mMode,&tResize,&tWidth,&tHeight};
     
     if(diaFactoryRun(QT_TR_NOOP("vdpau"),sizeof(elems)/sizeof(diaElem *),elems))
     {
                configuration.resizeToggle=(bool)doResize;
                info.width=configuration.targetWidth;
                info.height=configuration.targetHeight;
                ADM_info("New dimension : %d x %d\n",info.width,info.height);
                updateConf();
                cleanupVdpau();
                passThrough=!setupVdpau();
                
                return 1;
     }
     return 0;
     
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
   return ADM_paramSave(couples, vdpauFilterDeint_param,&configuration);
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *vdpauVideoFilterDeint::getConfiguration(void)
{
    static char conf[80];
    sprintf(conf,"Vdpau Deinterlace mode=%d, %d x %d",configuration.deintMode,info.width,info.height);
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
        ADM_warning("VdpauDeint:No image to upload\n");
        return true;
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
    \fn sendField
    \brief Process a field (top or bottom). If null the next param means there is no successor (next image)
*/
bool vdpauVideoFilterDeint::sendField(bool topField,ADMImage *next)
{
 // Call mixer...
    VdpVideoSurface in[3];
    bool r=true;
        // PREVIOUS
    if(!nextFrame) // First image, we dont have previous
    {
             in[0]=VDP_INVALID_HANDLE;
    }else
    {
             in[0]=input[(nextFrame+ADM_NB_SURFACES-1)%ADM_NB_SURFACES];
    }
        // CURRENT
     in[1]=input[nextFrame%ADM_NB_SURFACES];
        // NEXT
    if(next)
    {
     in[2]=input[(nextFrame+1)%ADM_NB_SURFACES];
    }
    else
    {
      in[2]=VDP_INVALID_HANDLE;
    }

    //
#if VDP_DEBUG
    printf("Current index=%d\n",(int)nextFrame);
    for(int i=0;i<3;i++) printf("Calling with in[%d]=%d\n",i,in[i]);
    for(int i=0;i<3;i++) printf("Desc[%d]=%d\n",i, frameDesc[i]);
#endif
    // ---------- Top field ------------
    if(VDP_STATUS_OK!=admVdpau::mixerRenderWithPastAndFuture(topField, 
                mixer,
                in,
                surface, 
                previousFilter->getInfo()->width,previousFilter->getInfo()->height))

    {
        ADM_warning("[Vdpau] Cannot mixerRender\n");
        r= false;
    }
    vidCache->unlockAll();
    return r;
}
/**
    \fn     getResult
    \brief  Convert the output surface into an ADMImage
*/
bool vdpauVideoFilterDeint::getResult(ADMImage *image)
{
 if(VDP_STATUS_OK!=admVdpau::outputSurfaceGetBitsNative(surface,
                                                            tempBuffer, 
                                                            info.width,info.height))
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
#if 0
    ts=destStride[2];destStride[2]=destStride[1];destStride[1]=ts;
    td=destData[1];destData[2]=destData[2];destData[1]=td;
#endif
    scaler->convertPlanes(  sourceStride,destStride,     
                            sourceData,destData);
    return true;
}
/**
    \fn getNextFrame
    \brief 

*/
bool vdpauVideoFilterDeint::getNextFrame(uint32_t *fn,ADMImage *image)
{
bool r=true;
    if(eof)
    {
        ADM_warning("[VdpauDeint] End of stream\n");
        return false;
    }
#define FAIL {r=false;goto endit;}
     if(passThrough) return previousFilter->getNextFrame(fn,image);
    // top field has already been sent, grab bottom field
    if((secondField)&&(configuration.deintMode==ADM_KEEP_BOTH))
        {
            secondField=false;
            *fn=nextFrame*2+1;
            if(false==getResult(image)) return false;
            if(ADM_NO_PTS==nextPts) image->Pts=nextPts;
                else image->Pts=nextPts-info.frameIncrement;
            aprintf("2ndField : Pts=%s\n",ADM_us2plain(image->Pts));
            return true;
        }
    
    // our first frame, we need to preload one frame
    if(!nextFrame)
    {
            ADMImage *prev= vidCache->getImage( 0);
            // Upload top field
            if(false==uploadImage(prev,nextFrame,0)) 
            {
                vidCache->unlockAll();
                return false;
            }
            nextPts=prev->Pts;
    }
    // regular image, in fact we get the next image here
    
    ADMImage *next= vidCache->getImage(nextFrame+1);
    if(next)
    {
        //printf("VDPAU DEINT : incoming PTS=%"LLU"\n",next->Pts);
    }
    if(false==uploadImage(next,nextFrame+1,nextFrame+1)) 
            {
                vidCache->unlockAll();
                
                FAIL
            }
   if(!next) eof=true; // End of stream
   
    // Now get our image back from surface...
    sendField(true,next); // always send top field
    if(configuration.deintMode==ADM_KEEP_TOP || configuration.deintMode==ADM_KEEP_BOTH)
    {
          if(false==getResult(image)) 
          {
               FAIL
          }
          aprintf("TOP/BOTH : Pts=%s\n",ADM_us2plain(image->Pts));
    }
    // Send 2nd field
    sendField(false,next); 
    if(configuration.deintMode==ADM_KEEP_BOTTOM)
    {
          if(false==getResult(image)) 
          {
               FAIL
          }
          aprintf("BOTTOM : Pts=%s\n",ADM_us2plain(image->Pts));
    }
    // Top Field..
endit:  
    if(configuration.deintMode==ADM_KEEP_BOTH) 
    {
        *fn=nextFrame*2;
        secondField=true;
    }
        else    *fn=nextFrame;
    nextFrame++;
    image->Pts=nextPts;
    if(next) nextPts=next->Pts;
   // printf("VDPAU OUT PTS= %"LLU"\n",image->Pts);
    return r;
}
#endif

//****************
// EOF
