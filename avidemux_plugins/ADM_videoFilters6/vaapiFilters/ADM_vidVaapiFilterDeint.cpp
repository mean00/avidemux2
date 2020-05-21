/**
    \brief vaapi filters
    \author mean (C) 2016
 *  Use VA-API video processing API
 * (c) mean 2016
 * 
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include <list>
#include "ADM_coreLibVA.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_videoFilterCache.h"
#include "ADM_vidMisc.h"
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "vaapiFilterDeint.h"
#include "vaapiFilterDeint_desc.cpp"

/**
    \class vaapiSlot
*/
class vaapiSlot
{
public:
    ADM_vaSurface           *surface;
    bool                    external;
    uint64_t                pts;

                            vaapiSlot();
                            ~vaapiSlot();
    void                    reset(void);
};

vaapiSlot::vaapiSlot()
{
    reset();
}
vaapiSlot::~vaapiSlot() {}

void vaapiSlot::reset(void)
{
    surface=NULL;
    external=false;
    pts=ADM_NO_PTS;
}
#define ADM_VAAPI_DEINT_MAX_REF 8

/**
    \class vaapiVideoFilterDeint
*/
class vaapiVideoFilterDeint : public  ADM_coreVideoFilterCached
{
protected:
    enum
    {
        ADM_VAAPI_DEINT_TOP_FIELD_FIRST=0,
        ADM_VAAPI_DEINT_BOTTOM_FIELD_FIRST=1
    };
    enum
    {
        ADM_VAAPI_DEINT_SEND_FRAME=0,
        ADM_VAAPI_DEINT_SEND_FIELD=1
    };

    vaapiSlot               *inputQueue;
    uint32_t                queueLength;
    std::list <ADM_vaSurface *> freeSurfaces;
    ADM_vaSurface           *surfacePool[ADM_VAAPI_DEINT_MAX_REF];
    ADM_vaSurface           *outputSurface;
    VAConfigID              configId;
    VAContextID             contextId;
    VABufferID              filterBuffer;
    uint32_t                algoCount; // actually supported by hardware and driver
    uint32_t                unsupported; // specified deint method rejected by driver
    VASurfaceID             *forwardRefs, *backwardRefs;
    uint32_t                nbForwardRefs,nbBackwardRefs;

    vaapiFilterDeint        configuration;
    uint64_t                deltaPts;
    bool                    passThrough;
    bool                    preloadCompleted;
    bool                    secondField;

    bool                    setupVaapi(void);
    bool                    cleanupVaapi(void);

    bool                    rotateSlots(void);
    bool                    clearSlots(void);
    bool                    fillSlot(uint32_t slot,ADMImage *image);

    bool                    updateInfo(bool status);

public:
        
                            vaapiVideoFilterDeint(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~vaapiVideoFilterDeint();

    virtual const char      *getConfiguration(void); // Return  current configuration as a human readable string
    virtual bool            getNextFrame(uint32_t *fn,ADMImage *image); // Return the next image
    virtual bool            getCoupledConf(CONFcouple **couples); // Return the current filter configuration
    virtual void            setCoupledConf(CONFcouple *couples);
    virtual bool            configure(void); // Start graphical user interface
    virtual bool            goToTime(uint64_t usSeek);
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(       vaapiVideoFilterDeint, // Class
                            1,0,0, // Version
                            (ADM_UI_QT4+ADM_FEATURE_LIBVA), // We need a display for VA-API; so no cli...
                            VF_INTERLACING, // Category
                            "vaapiDeint", // internal name (must be uniq!)
                            QT_TRANSLATE_NOOP("vaapiDeint","VA-API Deinterlacer"), // Display name
                            QT_TRANSLATE_NOOP("vaapiDeint","Deinterlace and optionally resize video using VA-API.") // Description
)

static const char *deintModeToString(uint32_t mode)
{
    switch(mode)
    {
#define DEINT(algo,name) case VAProcDeinterlacing##algo: return #name;
        DEINT(Bob,Bob)
        DEINT(Weave,Weave)
        DEINT(MotionAdaptive,Motion-Adaptive)
        DEINT(MotionCompensated,Motion-Compensated)
        default:
            return "Invalid";
#undef DEINT
    }
}

/**
    \fn setupVaapi
*/
bool vaapiVideoFilterDeint::setupVaapi(void)
{
    unsupported = 0;

    if(!admLibVA::isOperationnal())
    {
        ADM_warning("HW accel is not available.\n");
        return false;
    }

    configId = admLibVA::createFilterConfig();
    if(configId == VA_INVALID)
    {
        ADM_warning("Cannot create config\n");
        return false;
    }
    // Allocate output surface
    uint32_t outWidth=configuration.targetWidth;
    uint32_t outHeight=configuration.targetHeight;
    FilterInfo *prevInfo=previousFilter->getInfo();
    ADM_assert(prevInfo);

    if(!configuration.resize)
    {
        outWidth=prevInfo->width;
        outHeight=prevInfo->height;
    }

    outputSurface = ADM_vaSurface::allocateWithSurface(outWidth, outHeight);
    if(!outputSurface)
    {
        ADM_warning("Cannot allocate output surface with size %u x %u\n", outWidth, outHeight);
        cleanupVaapi();
        return false;
    }

    VAStatus status = vaCreateContext(admLibVA::getDisplay(), configId,
                        outWidth, outHeight, VA_PROGRESSIVE,
                        &outputSurface->surface, 1, &contextId);

    if(status != VA_STATUS_SUCCESS)
    {
        ADM_warning("Cannot create context: error %d (%s)\n",status,vaErrorStr(status));
        cleanupVaapi();
        return false;
    }
    // Query supported deinterlacing algorithms
    VAProcFilterCapDeinterlacing deintCaps[VAProcDeinterlacingCount];
    algoCount = VAProcDeinterlacingCount;

    status = vaQueryVideoProcFilterCaps(admLibVA::getDisplay(), contextId,
                        VAProcFilterDeinterlacing, &deintCaps, &algoCount);

    if(status != VA_STATUS_SUCCESS)
    {
        ADM_warning("Cannot query deinterlacing capabilities: error %d (%s)\n",status,vaErrorStr(status));
        cleanupVaapi();
        return false;
    }
    if(algoCount)
        ADM_info("Driver reports %u deinterlacing methods as supported.\n",algoCount);
    else
    {
        ADM_error("Driver reports that deinterlacing is not supported.\n");
        cleanupVaapi();
        return false;
    }
    uint32_t i,best=(uint32_t)deintCaps[algoCount-1].type;
    int algo = -1;
    for(i=0; i < algoCount; i++)
    {
        if((uint32_t)deintCaps[i].type != configuration.deintMode)
            continue;
        algo = i;
        break;
    }
    if(algo == -1)
    {
        ADM_warning("Requested deinterlacing mode %s is not supported.\n",deintModeToString(configuration.deintMode));
        ADM_warning("Using %s instead.\n",deintModeToString(best));
        unsupported = configuration.deintMode;
        configuration.deintMode = best;
    }
    // Query required number of reference surfaces
    VAProcFilterParameterBufferDeinterlacing deintParams;

    deintParams.type = VAProcFilterDeinterlacing;
    deintParams.algorithm = (VAProcDeinterlacingType)configuration.deintMode;
    deintParams.flags = 0;

    status = vaCreateBuffer(admLibVA::getDisplay(), contextId,
                        VAProcFilterParameterBufferType, sizeof(deintParams),
                        1, &deintParams, &filterBuffer);
    if(status != VA_STATUS_SUCCESS)
    {
        ADM_warning("Cannot create parameter buffer: error %d (%s)\n",status,vaErrorStr(status));
        cleanupVaapi();
        return false;
    }

    VAProcPipelineCaps caps;

    status = vaQueryVideoProcPipelineCaps(admLibVA::getDisplay(), contextId,
                        &filterBuffer, 1, &caps);
    if(status != VA_STATUS_SUCCESS)
    {
        ADM_warning("Cannot query video pipeline capabilities: error %d (%s)\n",status,vaErrorStr(status));
        cleanupVaapi();
        return false;
    }
    nbForwardRefs = caps.num_forward_references;
    nbBackwardRefs = caps.num_backward_references;
    if(nbForwardRefs)
    {
        forwardRefs = (VASurfaceID *)malloc(nbForwardRefs * sizeof(VASurfaceID));
        if(!forwardRefs)
        {
            cleanupVaapi();
            return false;
        }
    }
    if(nbBackwardRefs)
    {
        backwardRefs = (VASurfaceID *)malloc(nbBackwardRefs * sizeof(VASurfaceID));
        if(!backwardRefs)
        {
            cleanupVaapi();
            return false;
        }
    }
    queueLength = nbForwardRefs + nbBackwardRefs + 1;
    ADM_info("Video processing pipeline for mode %s operates with %u forward and %u backward references.\n",
                        deintModeToString(configuration.deintMode),
                        nbForwardRefs,nbBackwardRefs);
    if(queueLength + 1 > ADM_VAAPI_DEINT_MAX_REF)
    {
        ADM_error("Pipeline requires too many references (%u forward, %u back).\n",nbForwardRefs,nbBackwardRefs);
        cleanupVaapi();
        return false;
    }
    // Allocate source surfaces
    for(i=0; i < queueLength; i++)
    {
        ADM_vaSurface *s = ADM_vaSurface::allocateWithSurface(prevInfo->width,prevInfo->height);
        if(!s)
        {
            ADM_warning("Cannot allocate input surface %d\n",i);
            cleanupVaapi();
            return false;
        }
        surfacePool[i] = s;
    }
    freeSurfaces.clear();
    for(i=0; i < queueLength; i++)  
        freeSurfaces.push_back(surfacePool[i]);
    inputQueue = new vaapiSlot[queueLength];
    return true;    
}
/**
    \fn cleanupVaapi
*/
bool vaapiVideoFilterDeint::cleanupVaapi(void)
{
    for(uint32_t i=0; i < queueLength; i++)
    {
        ADM_vaSurface *s = surfacePool[i];
        if(s)
        {
            delete s;
            surfacePool[i]=NULL;
        }
    }
    if(filterBuffer!=VA_INVALID)
    {
        vaDestroyBuffer(admLibVA::getDisplay(), filterBuffer);
        filterBuffer=VA_INVALID;
    }
    if(outputSurface)
    {
        delete outputSurface;
        outputSurface=NULL;
    }
    if(configId!=VA_INVALID)
    {
        admLibVA::destroyFilterConfig(configId);
        configId=VA_INVALID;
    }
    if(contextId!=VA_INVALID)
    {
        admLibVA::destroyFilterContext(contextId);
        contextId=VA_INVALID;
    }
    if(forwardRefs)
        free(forwardRefs);
    forwardRefs=NULL;
    if(backwardRefs)
        free(backwardRefs);
    backwardRefs=NULL;
    if(inputQueue)
        delete [] inputQueue;
    inputQueue=NULL;
    unsupported=0;
    return true;
}
/**
 * \fn updateInfo
 * @param status
 * @return 
 */
bool vaapiVideoFilterDeint::updateInfo(bool status)
{
    passThrough=!status;
    memcpy(&info,previousFilter->getInfo(),sizeof(info));
    if(passThrough)
    {
        ADM_warning("PassThrough mode\n");
        return true;
    }
    if(configuration.framePerField==ADM_VAAPI_DEINT_SEND_FIELD)
    {
        info.frameIncrement /= 2;
        if(info.timeBaseNum && info.timeBaseDen)
        {
            if(info.timeBaseDen <= 30000 || (info.timeBaseNum & 1))
                info.timeBaseDen *= 2;
            else
                info.timeBaseNum /= 2;
            /* The frame increment passed along the filter chain may be based on
            the average frame rate, but we need the minimum increment here.
            Check whether the time base ~ matches the average increment and derive
            the minimum increment from time base if possible. */
            double f=1000.*1000.;
            f /= info.timeBaseDen;
            f *= info.timeBaseNum;
            f += 0.49;
            if((uint64_t)f > (uint64_t)info.frameIncrement*3/4)
                info.frameIncrement = (uint32_t)f;
        }
        ADM_info("New frame increment: %u us, new time base: %u / %u\n", info.frameIncrement, info.timeBaseNum, info.timeBaseDen);
    }
    if(configuration.resize)
    {
        info.width=configuration.targetWidth;
        info.height=configuration.targetHeight;
    }
    return true;
}
/**
    \fn constructor
*/
vaapiVideoFilterDeint::vaapiVideoFilterDeint(ADM_coreVideoFilter *in, CONFcouple *setup)
        : ADM_coreVideoFilterCached(ADM_VAAPI_DEINT_MAX_REF,in,setup)
{
    preloadCompleted=false;
    secondField=false;
    configId=VA_INVALID;
    contextId=VA_INVALID;
    for(int i=0; i < ADM_VAAPI_DEINT_MAX_REF; i++)
        surfacePool[i]=NULL;
    outputSurface=NULL;
    forwardRefs=NULL;
    backwardRefs=NULL;
    inputQueue=NULL;
    queueLength=0;
    nbForwardRefs=0;
    nbBackwardRefs=0;
    deltaPts=0;
    if(!setup || !ADM_paramLoad(setup,vaapiFilterDeint_param,&configuration))
    {
        // Default value
        configuration.deintMode=4;
        configuration.fieldOrder=0;
        configuration.framePerField=0;
        configuration.targetWidth=info.width;
        configuration.targetHeight=info.height;
        configuration.resize=false;
    }

    myName="vaapiDeint";
    bool status=setupVaapi();
    updateInfo(status);
}
/**
    \fn destructor
*/
vaapiVideoFilterDeint::~vaapiVideoFilterDeint()
{
    cleanupVaapi();
}
/**
    \fn configure
*/
bool vaapiVideoFilterDeint::configure( void) 
{
    diaMenuEntry deintMethod[]={
        { VAProcDeinterlacingBob,               QT_TRANSLATE_NOOP("vaapiDeint","Bob"),NULL },
        { VAProcDeinterlacingWeave,             QT_TRANSLATE_NOOP("vaapiDeint","Weave"),NULL },
        { VAProcDeinterlacingMotionAdaptive,    QT_TRANSLATE_NOOP("vaapiDeint","Motion-Adaptive"),NULL },
        { VAProcDeinterlacingMotionCompensated, QT_TRANSLATE_NOOP("vaapiDeint","Motion-Compensated"),NULL }
    };
    diaMenuEntry fieldOrder[]={
        { ADM_VAAPI_DEINT_TOP_FIELD_FIRST,          QT_TRANSLATE_NOOP("vaapiDeint","Top Field First"),NULL },
        { ADM_VAAPI_DEINT_BOTTOM_FIELD_FIRST,       QT_TRANSLATE_NOOP("vaapiDeint","Bottom Field First"),NULL }
    };
    diaMenuEntry outputPolicy[]={
        { ADM_VAAPI_DEINT_SEND_FRAME,   QT_TRANSLATE_NOOP("vaapiDeint","Frame per Frame"),NULL },
        { ADM_VAAPI_DEINT_SEND_FIELD,   QT_TRANSLATE_NOOP("vaapiDeint","Double Framerate"),NULL }
    };

    diaElemMenu dMode(&configuration.deintMode, QT_TRANSLATE_NOOP("vaapiDeint","_Mode:"), 4, deintMethod);
    diaElemMenu fOrder(&configuration.fieldOrder, QT_TRANSLATE_NOOP("vaapiDeint","_Field Order:"), 2, fieldOrder);
    diaElemMenu outPol(&configuration.framePerField, QT_TRANSLATE_NOOP("vaapiDeint","_Output:"), 2, outputPolicy);

    diaElemFrame frameDeint(QT_TRANSLATE_NOOP("vaapiDeint","Deinterlacing"));
    frameDeint.swallow(&dMode);
    frameDeint.swallow(&fOrder);
    frameDeint.swallow(&outPol);

    diaElemToggle tResize(&configuration.resize, QT_TRANSLATE_NOOP("vaapiDeint","_Resize"));
    diaElemUInteger tWidth(&configuration.targetWidth, QT_TRANSLATE_NOOP("vaapiDeint","Width:"), 16, MAXIMUM_SIZE);
    diaElemUInteger tHeight(&configuration.targetHeight, QT_TRANSLATE_NOOP("vaapiDeint","Height:"), 16, MAXIMUM_SIZE);

    diaElemFrame frameResize(QT_TRANSLATE_NOOP("vaapiDeint","Transformation"));
    frameResize.swallow(&tResize);
    frameResize.swallow(&tWidth);
    frameResize.swallow(&tHeight);

    tResize.link(1,&tWidth);
    tResize.link(1,&tHeight);

    diaElem *elems[]={&frameDeint,&frameResize};
    if(diaFactoryRun(QT_TRANSLATE_NOOP("vaapiDeint","VA-API Deinterlacer and Resizer"),2,elems))
    {
        cleanupVaapi();
        bool status=setupVaapi();
        if(unsupported)
        {
            
            GUI_Info_HIG( ADM_LOG_IMPORTANT,
                          QT_TRANSLATE_NOOP("vaapiDeint","Unsupported Mode"),
                          QT_TRANSLATE_NOOP("vaapiDeint","Specified deinterlacing mode %s is not supported, replaced with %s."),
                              deintModeToString(unsupported), deintModeToString(configuration.deintMode));
            unsupported = 0;
        }
        if(!status)
        {
            GUI_Error_HIG(QT_TRANSLATE_NOOP("vaapiDeint","VA-API Setup Error"),
                          QT_TRANSLATE_NOOP("vaapiDeint","Could not setup VA-API, purely passthrough operation."));
        }
        updateInfo(status);
        return true;
    }
    return false;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool vaapiVideoFilterDeint::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, vaapiFilterDeint_param, &configuration);
}

void vaapiVideoFilterDeint::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, vaapiFilterDeint_param, &configuration);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *vaapiVideoFilterDeint::getConfiguration(void)
{
    static char conf[256];
    sprintf(conf,"VA-API deint. mode: %s, parity: %s, double fps: %s",
                        deintModeToString(configuration.deintMode),
                        (configuration.fieldOrder==ADM_VAAPI_DEINT_TOP_FIELD_FIRST)? "top field first" : "bottom field first",
                        (configuration.framePerField==ADM_VAAPI_DEINT_SEND_FIELD)? "yes" : "no");
    if(configuration.resize)
    {
        char part2[80]={0};
        sprintf(part2,", resize from %dx%d to %dx%d",
                previousFilter->getInfo()->width, previousFilter->getInfo()->height,
                configuration.targetWidth, configuration.targetHeight);
        strcat(conf,part2);
    }
    conf[255]=0;
    return conf;
}

/**
    \fn goToTime
    \brief called when seeking. Need to cleanup our stuff.
*/
bool vaapiVideoFilterDeint::goToTime(uint64_t usSeek)
{
    secondField=false;
    preloadCompleted=false;
    clearSlots();
    uint32_t oldFrameIncrement=info.frameIncrement;
    if(!passThrough && configuration.framePerField==ADM_VAAPI_DEINT_SEND_FIELD)
        info.frameIncrement*=2;
    bool r=ADM_coreVideoFilterCached::goToTime(usSeek);
    info.frameIncrement=oldFrameIncrement;
    return r;
}

/**
    \fn fillSlot
    \brief upload the image to the slot. 
*/
bool vaapiVideoFilterDeint::fillSlot(uint32_t slot,ADMImage *image)
{
    ADM_assert(slot<queueLength);
    ADM_vaSurface *target;
    bool external=false;
    if(image->refType!=ADM_HW_LIBVA)
    {
        // provide a surface from our pool
        ADM_assert(freeSurfaces.size());
        target=freeSurfaces.front();
        freeSurfaces.pop_front();
        if(!target->fromAdmImage(image)) 
            return false;
    }else
    {
        // use the provided surface
        target=(ADM_vaSurface *)image->refDescriptor.refHwImage;
        //printf("Source image is already VAAPI, surface %d, slot %d, pts %s\n",(uint32_t)target->surface,slot,ADM_us2plain(image->Pts));
        ADM_assert(target->refCount);
        image->hwIncRefCount();
        external=true;
    }
    inputQueue[slot].pts=image->Pts;
    inputQueue[slot].surface=target;
    inputQueue[slot].external=external;
    return true;
}

/**
    \fn rotateSlots
*/
bool vaapiVideoFilterDeint::rotateSlots(void)
{
    ADM_assert(queueLength);
    vaapiSlot *s = &inputQueue[0];
    if(s->surface)
    {
        if(!s->external)
            freeSurfaces.push_back(s->surface);
        else if(s->surface->refCount>0)
            s->surface->refCount--;
    }
    for(int i=0; i < (int)queueLength-1; i++)
        inputQueue[i] = inputQueue[i+1];
    s = &inputQueue[queueLength-1];
    s->reset();
    return true;
}

/**
    \fn clearSlots
*/
bool vaapiVideoFilterDeint::clearSlots(void)
{
    for(int i=0; i < (int)queueLength; i++)
    {
        vaapiSlot *s = &inputQueue[i];  
        if(s->surface)
        {
            if(!s->external)
                freeSurfaces.push_back(s->surface);
            else if(s->surface->refCount>0)
                s->surface->refCount--;
        }
        s->reset();
    }
    return true;
}

/**
    \fn getNextFrame
*/
bool vaapiVideoFilterDeint::getNextFrame(uint32_t *fn,ADMImage *image)
{
    bool r=false;
    uint32_t i;
    if(passThrough) 
    {
        //ADM_info("VA-API deinterlacer: passthrough\n");
        return previousFilter->getNextFrame(fn,image);
    }
    if(!secondField)
    {
        // shift frames;... free slot[0]
        rotateSlots();

        if(!preloadCompleted)
        {
            for(i=0; i < queueLength; i++)
            {
                ADMImage *ref = vidCache->getImageAs(ADM_HW_LIBVA,i);
                if(!ref || false==fillSlot(i,ref))
                {
                    vidCache->unlockAll();
                    ADM_error("Cannot fill the queue, need %u pictures, got %u, aborting.\n",queueLength,i);
                    return false;
                }
            }
            preloadCompleted=true;
            nextFrame+=nbForwardRefs;
            //ADM_info("Preloaded %u pictures, nextFrame = %u\n",queueLength,nextFrame);
        }else
        {
            //printf("[vaapiVideoFilterDeint::getNextFrame] requesting frame %u from cache.\n",nextFrame);
            ADMImage *next = vidCache->getImageAs(ADM_HW_LIBVA,nextFrame);
            if(!next)
            {
                vidCache->unlockAll();
                return false;
            }
            if(false==fillSlot(queueLength-1,next))
            {
                vidCache->unlockAll();
                return false;
            }
        }
    }
    vaapiSlot *src = &inputQueue[nbForwardRefs];
    ADM_assert(src);
    vaapiSlot *prev = NULL;
    if(nbForwardRefs)
        prev = &inputQueue[nbForwardRefs-1];
    if(prev && prev->pts!=ADM_NO_PTS && src->pts!=ADM_NO_PTS && src->pts > prev->pts)
        deltaPts = src->pts - prev->pts;
    image->Pts = src->pts;
    if(secondField && image->Pts != ADM_NO_PTS)
    {
        if(deltaPts < info.frameIncrement*2)
            image->Pts += deltaPts/2;
        else
            image->Pts += info.frameIncrement;
    }

    for(i=0; i < nbForwardRefs; i++)
    {
        forwardRefs[i] = inputQueue[nbForwardRefs-i-1].surface->surface;
        //printf("forward ref %d is VASurfaceID %u\n",i,(uint32_t)forwardReferences[i]);
    }
    for(i=0; i < nbBackwardRefs; i++)
    {
        backwardRefs[i] = inputQueue[nbForwardRefs+i+1].surface->surface;
        //printf("backward ref %d is VASurfaceID %u\n",i,(uint32_t)backwardReferences[i]);
    }

    //-- Perform --
    VAProcPipelineParameterBuffer param;
    VABufferID paramId;
    VAStatus status;

    memset(&param,0,sizeof(param));

    param.surface = src->surface->surface;
    param.surface_region = NULL;
    param.surface_color_standard = VAProcColorStandardBT709 ; // FIXME
    param.output_region = NULL;
    param.output_background_color = 0xff000000;
    param.output_color_standard = VAProcColorStandardBT709; // FIXME

    param.filter_flags = VA_FRAME_PICTURE;
    param.filter_flags |= VA_FILTER_SCALING_HQ;
    param.pipeline_flags = 0;
    param.filters = &filterBuffer;
    param.num_filters = 1;

    param.forward_references = forwardRefs;
    param.num_forward_references = nbForwardRefs;
    param.backward_references = backwardRefs;
    param.num_backward_references = nbBackwardRefs;

    VAProcFilterParameterBufferDeinterlacing *deintParams;
    void *deintParamsPtr;

#define CHECK(x) { status=x; if(status!=VA_STATUS_SUCCESS) { ADM_warning( #x " failed with error %d: %s\n",status,vaErrorStr(status)); goto failed; }}
    CHECK(vaMapBuffer(admLibVA::getDisplay(), filterBuffer, &deintParamsPtr))

    deintParams = (VAProcFilterParameterBufferDeinterlacing *)deintParamsPtr;
    deintParams->flags = 0;
    if(configuration.fieldOrder == ADM_VAAPI_DEINT_BOTTOM_FIELD_FIRST)
        deintParams->flags |= VA_DEINTERLACING_BOTTOM_FIELD_FIRST;
    if(secondField == (configuration.fieldOrder == ADM_VAAPI_DEINT_TOP_FIELD_FIRST))
        deintParams->flags |= VA_DEINTERLACING_BOTTOM_FIELD;
    deintParamsPtr=NULL;
 
    CHECK(vaUnmapBuffer(admLibVA::getDisplay(), filterBuffer))

    CHECK(vaBeginPicture(admLibVA::getDisplay(), contextId, outputSurface->surface))
    CHECK(vaCreateBuffer(admLibVA::getDisplay(), contextId,
                         VAProcPipelineParameterBufferType,
                         sizeof(param), 1,
                         &param, &paramId))
    // Go
    CHECK(vaRenderPicture(admLibVA::getDisplay(), contextId, &paramId, 1))
    CHECK(vaEndPicture(admLibVA::getDisplay(), contextId))

    // Download result to regular ADMImage
    r = outputSurface->toAdmImage(image);
    //printf("Result is %d\n",r);
failed:
    if(paramId!=VA_INVALID)
        vaDestroyBuffer(admLibVA::getDisplay(), paramId);
    if(configuration.framePerField==ADM_VAAPI_DEINT_SEND_FIELD) 
    {
        *fn=(nextFrame-nbForwardRefs)*2+secondField;
        secondField=!secondField;
        //printf("%s, frame number: %u, pts: %s\n",secondField? "First field" : "Second field",*fn,ADM_us2plain(image->Pts));
    }else
        *fn=nextFrame-nbForwardRefs;
    if(!secondField) nextFrame++;
    vidCache->unlockAll();
    return r;
}
// EOF
