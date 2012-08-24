/**
    \brief VDPAU filters Deinterlacer
    \author mean (C) 2010
    This is slow as we copy back and forth data to/from the video cards
    
    On a Q6600

    FullHD: 
            Readback ~ 5 ms, RGB 2 YUV ~ 20 ms : 100% CPU
    720
            Readback ~ 2 ms, RGB2YUV ~ 10 ms  : 50% CPU
            


*/
#include "ADM_cpp.h"
#include <list>
#include "ADM_default.h"
#ifdef USE_VDPAU
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/vdpau.h"
}

#include "ADM_coreVideoFilter.h"
#include "ADM_videoFilterCache.h"
#include "DIA_factory.h"
#include "ADM_vidMisc.h"
#include "vdpauFilterDeint.h"
#include "vdpauFilterDeint_desc.cpp"

#include "ADM_coreVdpau/include/ADM_coreVdpau.h"
//
#define ADM_INVALID_FRAME_NUM 0x80000000
#define ADM_NB_SURFACES 5

//#define DO_BENCHMARK
#define NB_BENCH 100

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
    \class VDPSlot
*/
class VDPSlot
{
public:
                              VDPSlot() ;
                             ~VDPSlot();
            VdpVideoSurface   surface;
            bool              isExternal;
            uint64_t          pts;
            uint32_t          frameNumber;
            ADMImage          *image;
};

VDPSlot::VDPSlot()
{
    surface=VDP_INVALID_HANDLE;
    image=NULL;
}
VDPSlot::~VDPSlot()
{
    if(image) delete image;
    image=NULL;
    if(surface!=VDP_INVALID_HANDLE)
    {
        // will be freed by the pool..
    }
    surface=VDP_INVALID_HANDLE;
}

/**
    \class vdpauVideoFilterDeint
*/
class vdpauVideoFilterDeint : public  ADM_coreVideoFilterCached
{
protected:
                    VDPSlot              slots[3];
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
                    VdpOutputSurface     outputSurface;
                    std::list <VdpVideoSurface> freeSurface;
                    VdpVideoSurface      surfacePool[ADM_NB_SURFACES];
                    VdpVideoMixer        mixer;
protected:
                    bool                 rotateSlots(void);
                    bool                 clearSlots(void);
                    bool                 uploadImage(ADMImage *next,const VdpVideoSurface surface) ;
                    bool                 fillSlot(int slot,ADMImage *image);
                    bool                 getResult(ADMImage *image);
                    bool                 sendField(bool topField);
                    bool                 setIdentityCSC(void);

public:
        virtual bool         goToTime(uint64_t usSeek); 
                             vdpauVideoFilterDeint(ADM_coreVideoFilter *previous,CONFcouple *conf);
                             ~vdpauVideoFilterDeint();

        virtual const char   *getConfiguration(void);                 /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);           /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
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
    clearSlots();
    return ADM_coreVideoFilterCached::goToTime(usSeek);
}
/**
    \fn setIdentityCSC
    \brief set the RGB/YUV matrix to identity so that data are still YUV at the end
            Should not work, but it does.
*/
bool vdpauVideoFilterDeint::setIdentityCSC(void)
{
    ADM_info("Setting custom CSC\n");
    const VdpCSCMatrix   matrix={{1.,0,0,0},{0,1.,0,0},{0,0,1.,0}};    
    VdpVideoMixerAttribute attributes_key[]={VDP_VIDEO_MIXER_ATTRIBUTE_CSC_MATRIX};
    const void * attribute_values[] = {&matrix};
    
    VdpStatus st = admVdpau::mixerSetAttributesValue(mixer, 1,attributes_key, (void * const *)attribute_values);
    if(st!=VDP_STATUS_OK)
        ADM_error("Cannot set custom matrix (CSC)\n");
    return true;
}
/**
    \fn resetVdpau
*/
bool vdpauVideoFilterDeint::setupVdpau(void)
{
    scaler=NULL;
    secondField=false;
    nextFrame=0;
    if(!admVdpau::isOperationnal())
    {
        ADM_warning("Vdpau not operationnal\n");
        return false;
    }   
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceCreate(VDP_RGBA_FORMAT_B8G8R8A8,
                        info.width,info.height,&outputSurface)) 
    {
        ADM_error("Cannot create outputSurface0\n");
        return false;
    }
    for(int i=0;i<ADM_NB_SURFACES;i++) surfacePool[i]=VDP_INVALID_HANDLE;
    for(int i=0;i<ADM_NB_SURFACES;i++)
    {
        if(VDP_STATUS_OK!=admVdpau::surfaceCreate(   previousFilter->getInfo()->width,
                                                    previousFilter->getInfo()->height,
                                                    &(surfacePool[i]))) 
        {
            ADM_error("Cannot create input Surface %d\n",i);
            goto badInit;
        }
        aprintf("Created surface %d\n",(int)surfacePool[i]);
    }
    // allocate our (dummy) images
    for(int i=0;i<3;i++)
        slots[i].image=new ADMImageDefault( previousFilter->getInfo()->width, 
                                            previousFilter->getInfo()->height);
                                            
    if(VDP_STATUS_OK!=admVdpau::mixerCreate(previousFilter->getInfo()->width,
                                            previousFilter->getInfo()->height,&mixer,true)) 
    {
        ADM_error("Cannot create mixer\n");
        goto badInit;
    } 
    tempBuffer=new uint8_t[info.width*info.height*4];
    scaler=new ADMColorScalerSimple( info.width,info.height, ADM_COLOR_BGR32A,ADM_COLOR_YV12);

    freeSurface.clear();
    for(int i=0;i<ADM_NB_SURFACES;i++)  
            freeSurface.push_back(surfacePool[i]);
    setIdentityCSC();
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
        if(surfacePool[i]!=VDP_INVALID_HANDLE) admVdpau::surfaceDestroy(surfacePool[i]);
    if(outputSurface!=VDP_INVALID_HANDLE)  admVdpau::outputSurfaceDestroy(outputSurface);
    if(mixer!=VDP_INVALID_HANDLE) admVdpau::mixerDestroy(mixer);
    outputSurface=VDP_INVALID_HANDLE;
    for(int i=0;i<ADM_NB_SURFACES;i++)
        surfacePool[i]=VDP_INVALID_HANDLE;
    mixer=VDP_INVALID_HANDLE;
    if(tempBuffer) delete [] tempBuffer;
    tempBuffer=NULL;
    if(scaler) delete scaler;
    scaler=NULL;
    for(int i=0;i<3;i++)
       if(slots[i].image)
        {
            delete slots[i].image;
            slots[i].image=NULL;
        }

    return true;
}

/**
    \fn constructor
*/
vdpauVideoFilterDeint::vdpauVideoFilterDeint(ADM_coreVideoFilter *in, CONFcouple *setup): 
        ADM_coreVideoFilterCached(5,in,setup)
{
    eof=false;
    for(int i=0;i<ADM_NB_SURFACES;i++)
        surfacePool[i]=VDP_INVALID_HANDLE;
    mixer=VDP_INVALID_HANDLE;
    outputSurface=VDP_INVALID_HANDLE;
    if(!setup || !ADM_paramLoad(setup,vdpauFilterDeint_param,&configuration))
    {
        // Default value
        configuration.resizeToggle=false;
        configuration.deintMode=ADM_KEEP_TOP;
        configuration.targetWidth=info.width;
        configuration.targetHeight=info.height;
    }
    
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
                if(doResize)
                {
                    info.width=configuration.targetWidth;
                    info.height=configuration.targetHeight;
                }else
                {
                    info.width=previousFilter->getInfo()->width;
                    info.height=previousFilter->getInfo()->height;
                }
                ADM_info("New dimension : %d x %d\n",info.width,info.height);
                updateConf();
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
bool         vdpauVideoFilterDeint::getCoupledConf(CONFcouple **couples)
{
   return ADM_paramSave(couples, vdpauFilterDeint_param,&configuration);
}

void vdpauVideoFilterDeint::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, vdpauFilterDeint_param, &configuration);
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
bool vdpauVideoFilterDeint::uploadImage(ADMImage *next,VdpVideoSurface surface) 
{
    if(!next) // empty image
    {
        ADM_warning("VdpauDeint:No image to upload\n");
        return true;
    }
    if(surface==VDP_INVALID_HANDLE)
    {
        ADM_error("Surface provided is invalid\n");
        return false;
    }
  // Blit our image to surface
    uint32_t pitches[3];
    uint8_t *planes[3];
    next->GetPitches(pitches);
    next->GetReadPlanes(planes);

    aprintf("Putting image in surface %d\n",(int)surface);
    // Put out stuff in input...
#if VDP_DEBUG
    printf("Uploading image to surface %d\n",surfaceIndex%ADM_NB_SURFACES);
#endif
    if(VDP_STATUS_OK!=admVdpau::surfacePutBits( 
            surface,
            planes,pitches))
    {
        ADM_warning("[Vdpau] video surface : Cannot putbits\n");
        return false;
    }
    return true;
}
/**
    \fn fillSlot
    \brief upload the image to the slot. 
*/
bool vdpauVideoFilterDeint::fillSlot(int slot,ADMImage *image)
{
    VdpVideoSurface tgt;
    bool external=false;
    if(image->refType!=ADM_HW_VDPAU)
    {   // Need to allocate a surface
        ADM_assert(freeSurface.size());
        tgt=freeSurface.front();
        freeSurface.pop_front();  
        aprintf("FillSlot : Popped %d\n",tgt);
        //
        if(false==uploadImage(image,tgt)) 
        {
            return false;
        }
        external=false;
    }else
    {   // use the provided surface
        aprintf("Deint Image is already vdpau, slot %d \n",slot);
        ADMImage *img=slots[slot].image;
        img->duplicateFull(image); // increment ref count
        // get surface
        img->hwDownloadFromRef();
        vdpau_render_state *render=(vdpau_render_state *)img->refDescriptor.refCookie;
        ADM_assert(render->refCount);
        tgt=render->surface;
        external=true;
    }
    nextPts=image->Pts;
    slots[slot].pts=image->Pts;
    slots[slot].surface=tgt;
    slots[slot].isExternal=external;
    return true;
}

/**
    \fn rotateSlots
*/
bool vdpauVideoFilterDeint::rotateSlots(void)
{
    VDPSlot *s=&(slots[0]);
    ADMImage *img=slots[0].image;
    if(s->surface!=VDP_INVALID_HANDLE)
        if(!s->isExternal)
        {
            freeSurface.push_back(s->surface);
            s->surface=VDP_INVALID_HANDLE;
        }else
        {
            // Ref couting dec..
            s->image->hwDecRefCount();
            s->surface=VDP_INVALID_HANDLE;
        }
    slots[0]=slots[1];
    slots[1]=slots[2];
    slots[2].surface=VDP_INVALID_HANDLE;
    slots[2].image=img;
    return true;
}
/**
    \fn clearSlots
*/
bool vdpauVideoFilterDeint::clearSlots(void)
{
    for(int i=0;i<3;i++)
    {
           VDPSlot *s=&(slots[i]);  
           if(s->surface!=VDP_INVALID_HANDLE)
            {
                if(s->isExternal)
                {
                    s->image->hwDecRefCount();
                }else
                {
                    freeSurface.push_back(s->surface);
                }
            }
            s->surface=VDP_INVALID_HANDLE;
    }
    return true;
}
/**
    \fn sendField
    \brief Process a field (top or bottom). If null the next param means there is no successor (next image)
*/
bool vdpauVideoFilterDeint::sendField(bool topField)
{
 // Call mixer...
    VdpVideoSurface in[3];
    bool r=true;
    // PREVIOUS
    for(int i=0;i<3;i++)
    {
        in[i]=slots[i].surface;
        aprintf("Mixing %d %d\n",i,(int)in[i]);
    }
    if(in[0]==VDP_INVALID_HANDLE)
            in[0]=in[1];
    //

#ifdef DO_BENCHMARK
    ADMBenchmark bmark;
    for(int i=0;i<NB_BENCH;i++)
    {
        bmark.start();
#endif
    
       
    // ---------- Top field ------------
    if(VDP_STATUS_OK!=admVdpau::mixerRenderWithPastAndFuture(topField, 
                mixer,
                in,
                outputSurface, 
                previousFilter->getInfo()->width,previousFilter->getInfo()->height))

    {
        ADM_warning("[Vdpau] Cannot mixerRender\n");
        r= false;
    }   
                     
#ifdef DO_BENCHMARK
        bmark.end();
    }
    ADM_warning("Mixer Benchmark\n");
    bmark.printResult();
#endif 
    return r;
}
/**
    \fn     getResult
    \brief  Convert the output surface into an ADMImage
*/
bool vdpauVideoFilterDeint::getResult(ADMImage *image)
{

    ADM_assert(image->GetWidth(PLANAR_Y)==info.width);
    ADM_assert(image->GetHeight(PLANAR_Y)==info.height);
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceGetBitsNative(outputSurface,
                                                            tempBuffer, 
                                                            info.width,info.height))
    {
        ADM_warning("[Vdpau] Cannot copy back data from output surface\n");
        return false;
    }
    return image->convertFromYUV444(tempBuffer);
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
     // shift frames;... free slot[0]
    rotateSlots();

    // our first frame, we need to preload one frame
    if(!nextFrame)
    {
            aprintf("This is our first image, filling slot 1\n");
            ADMImage *prev= vidCache->getImageAs(ADM_HW_VDPAU,0);
            if(false==fillSlot(1,prev))
            {
                    vidCache->unlockAll();
                    return false;
            }
            
    }
    // regular image, in fact we get the next image here
    ADMImage *next= vidCache->getImageAs(ADM_HW_VDPAU,nextFrame+1);
    if(next)
    {
            if(false==fillSlot(2,next))
            {
                vidCache->unlockAll();
                FAIL
            }
    }
    if(!next) eof=true; // End of stream

    // now we have slot 0 : prev Image, slot 1=current image, slot 2=next image

    // Now get our image back from surface...
    sendField(true); // always send top field
    if(configuration.deintMode==ADM_KEEP_TOP || configuration.deintMode==ADM_KEEP_BOTH)
    {
          if(false==getResult(image)) 
          {
               FAIL
          }
          aprintf("TOP/BOTH : Pts=%s\n",ADM_us2plain(image->Pts));
    }
    // Send 2nd field
    sendField(false); 
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
    vidCache->unlockAll();
    if(configuration.deintMode==ADM_KEEP_BOTH) 
    {
        *fn=nextFrame*2;
        secondField=true;
    }
        else    *fn=nextFrame;
    nextFrame++;
    image->Pts=nextPts;
    if(next) nextPts=next->Pts;
   // printf("VDPAU OUT PTS= %"PRIu64"\n",image->Pts);
    return r;
}
#else // USE_VDPAU
static void dummy_fun(void)
{
    return ;
}
#endif // use VDPAU

//****************
// EOF
