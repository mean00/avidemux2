/***************************************************************************
    \file             : ADM_coreVdpau.cpp
    \brief            : Wrapper around vdpau functions
    \author           : (C) 2010 by mean fixounet@free.fr
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
#include "../include/ADM_coreVdpau.h"

#if defined(USE_VDPAU) 
#include "../include/ADM_coreVdpauInternal.h"
#include "ADM_dynamicLoading.h"

#if 0
    #define aprintf ADM_info
#else
    #define aprintf(...) {}
#endif
//GUI_WindowInfo      admVdpau::myWindowInfo;

/**
    \fn    mixerIsFeatureEnabled
    \brief 
*/
bool admVdpau::mixerIsFeatureEnabled( VdpVideoMixer mixer,VdpVideoMixerFeature feature)
{
    VdpBool enabledFeature=true;
    CHECKBOOL(ADM_coreVdpau::funcs.mixerGetFeaturesEnabled(mixer,1,&feature,&enabledFeature));
    if(enabledFeature) return true;
    return false;
}
/**
    \fn    mixerEnableFeature
    \brief enable mixer feature
*/

VdpStatus admVdpau::mixerEnableFeature( VdpVideoMixer mixer,uint32_t nbFeature,VdpVideoMixerFeature *feature,VdpBool *enabledFeature)
{
    aprintf("Enabling %d features\n",nbFeature);
    CHECK(ADM_coreVdpau::funcs.mixerEnableFeatures(mixer,nbFeature,feature,enabledFeature));
}
/**
    \fn mixerFeatureSupported
    \brief Check a feature is supported by VDPAU
*/
bool admVdpau::mixerFeatureSupported(VdpVideoMixerFeature attribute)
{
VdpBool supported=VDP_TRUE;
    CHECKBOOL(ADM_coreVdpau::funcs.mixerQueryFeatureSupported(ADM_coreVdpau::vdpDevice,attribute,&supported));
    if(VDP_TRUE==supported) return true;
    return false;
}
/**
    \fn mixerCreate
*/
VdpStatus admVdpau::mixerCreate(uint32_t width,uint32_t height, VdpVideoMixer *mixer,bool deinterlace)
{
#define MIXER_NB_PARAM 3
#define MIXER_NB_FEATURE_MAX 7

VdpVideoMixerParameter parameters[MIXER_NB_PARAM]=
                                              {VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH,
                                               VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT,
                                               VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE
                                               };
 VdpVideoMixerFeature features[MIXER_NB_FEATURE_MAX];
 VdpBool              enabledFeatures[MIXER_NB_FEATURE_MAX]={VDP_TRUE,VDP_TRUE,VDP_TRUE,VDP_TRUE,VDP_TRUE,VDP_TRUE,VDP_TRUE};

uint32_t color=VDP_CHROMA_TYPE_420;
void    *values[MIXER_NB_PARAM]={&width,&height,&color};
    int nbFeature=0;
    ADM_info("Creating vdpauMixer with width=%d, height=%d color=%d\n",width,height,color);
    //features[nbFeature++]=VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L5;
    if(deinterlace)
    {
        features[nbFeature++]=VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL;
    }
    
    int nbParam=MIXER_NB_PARAM;
    
    VdpStatus e=ADM_coreVdpau::funcs.mixerCreate(ADM_coreVdpau::vdpDevice,
                        nbFeature,features,
                        nbParam,parameters,values,
                        mixer);
    if(VDP_STATUS_OK!=e)
    {
        ADM_warning("MixerCreate  failed :%s\n",getErrorString(e));
        
    }else   
    {
        mixerEnableFeature(*mixer, nbFeature, features, enabledFeatures);
    }
    return e;
}
/**
    \fn mixerDestroy
*/

VdpStatus admVdpau::mixerDestroy(VdpVideoMixer mixer)
{
    CHECK(ADM_coreVdpau::funcs.mixerDestroy(mixer));
}
/**
    \fn mixerRender
*/

VdpStatus admVdpau::mixerRender(VdpVideoMixer mixer,
                                VdpVideoSurface sourceSurface,
                                VdpOutputSurface targetOutputSurface, 
                                uint32_t targetWidth, 
                                uint32_t targetHeight )
{
const VdpVideoSurface listOfInvalidSurface[1]={VDP_INVALID_HANDLE};
      VdpStatus e=ADM_coreVdpau::funcs.mixerRender(mixer,
                VDP_INVALID_HANDLE,NULL,    // Background
                VDP_VIDEO_MIXER_PICTURE_STRUCTURE_FRAME,
                
                0,            listOfInvalidSurface, // Past...
                sourceSurface,                      // current
                0,            listOfInvalidSurface, // Future
                NULL,                               // source RECT
                targetOutputSurface,
                NULL,                               // dest Rec
                NULL,                               // dest video Rec
                0,NULL);                            // Layers
                
#if 0      
  ADM_info("Target width=%d height=%d\n", targetWidth,targetHeight);
  VdpRGBAFormat fmt;
  VdpChromaType chroma;
  uint32_t tw,th;
  
  if(VDP_STATUS_OK!=ADM_coreVdpau::funcs.mixerGetOutputSurfaceParameters(targetOutputSurface,&fmt,&tw,&th))
  {
      ADM_warning("VdpOutputSurfaceGetParameters failed\n");
  }
  ADM_info("output surface width=%d height=%d fmt=%d\n", tw,th,fmt);
  if(VDP_STATUS_OK!=ADM_coreVdpau::funcs.mixerGetSurfaceParameters(sourceSurface,&chroma,&tw,&th))
  {
        ADM_warning("mixerGetSurfaceParameters failed\n");
  }
  ADM_info("input surface width=%d height=%d fmt=%d\n", tw,th,chroma);
#endif
  if(VDP_STATUS_OK!=e)
    {
        
        ADM_warning("MixerRender  failed :%s\n",getErrorString(e));
        
    }
    return e;
}
/**
 * \fn mixerRenderWithCropping
 * \brief Same as above but allow to take only top/left of the input image
 * @param mixer
 * @param sourceSurface
 * @param targetOutputSurface
 * @param targetWidth
 * @param targetHeight
 * @param sourceWidth
 * @param sourceHeight
 * @return 
 */
VdpStatus admVdpau::mixerRenderWithCropping(VdpVideoMixer mixer,
                                VdpVideoSurface sourceSurface,
                                VdpOutputSurface targetOutputSurface, 
                                uint32_t targetWidth, 
                                uint32_t targetHeight,uint32_t sourceWidth, uint32_t sourceHeight )
{
const VdpVideoSurface listOfInvalidSurface[1]={VDP_INVALID_HANDLE};

    VdpRect rect;
    rect.x0=rect.y0=0;
    rect.x1=sourceWidth;
    rect.y1=sourceHeight;
#if 0
    VdpChromaType sourceChroma;
    uint32_t sourceW,sourceH;
    ADM_coreVdpau::funcs.mixerGetSurfaceParameters(sourceSurface,&sourceChroma,&sourceW,&sourceH);
    ADM_info("Source is %d %d x %d\n",sourceChroma,sourceW,sourceH);
    VdpRGBAFormat rgb;
    ADM_coreVdpau::funcs.mixerGetOutputSurfaceParameters(targetOutputSurface,&rgb,&sourceW,&sourceH);
    ADM_info("Target is %d %d x %d\n",rgb,sourceW,sourceH);
    ADM_info("Cropped to %d x %d ==> %d x %d\n",sourceWidth,sourceHeight,targetWidth,targetHeight);
    ADM_info("Rect %d %d %d %d\n",rect.x0,rect.y0,rect.x1,rect.y1);
#endif    
      VdpStatus e=ADM_coreVdpau::funcs.mixerRender(mixer,
                VDP_INVALID_HANDLE,NULL,    // Background
                VDP_VIDEO_MIXER_PICTURE_STRUCTURE_FRAME,
                
                0,            listOfInvalidSurface, // Past...
                sourceSurface,                      // current
                0,            listOfInvalidSurface, // Future
                &rect,                               // source RECT
                targetOutputSurface,
                NULL,                               // dest Rec
                NULL,                               // dest video Rec
                0,NULL);                            // Layers
                
            
  if(VDP_STATUS_OK!=e)
    {
        
        ADM_warning("MixerCreate  failed :%s\n",getErrorString(e));
        
    }
    return e;
}

/**
    \fn mixerGetAttributesValue
*/

VdpStatus admVdpau::mixerGetAttributesValue(VdpVideoMixer mixer,
                                uint32_t attrCount,
                                const  VdpVideoMixerAttribute  *keys,
                                  void * const *         values)
{
                
    VdpStatus e=ADM_coreVdpau::funcs.mixerGetAttributesValue(mixer,attrCount,keys,values);
    if(VDP_STATUS_OK!=e)
    {
        
        ADM_warning("MixerGetAttributes  failed :%s\n",getErrorString(e));
        
    }
    return e;
}
/**
    \fn mixerSetAttributesValue
*/
VdpStatus admVdpau::mixerSetAttributesValue(VdpVideoMixer mixer,
                                uint32_t attrCount,
                                const  VdpVideoMixerAttribute *xkeys,
                                void * const* values)
{
    VdpStatus e=ADM_coreVdpau::funcs.mixerSetAttributesValue(mixer,attrCount,xkeys,values);
    if(VDP_STATUS_OK!=e)
    {
        
        ADM_warning("MixerSetAttributes  failed :%s\n",getErrorString(e));
        
    }
    return e;
}

/**
    \fn mixerRenderWithPastAndFuture
*/

VdpStatus admVdpau::mixerRenderWithPastAndFuture(
                                bool topField,
                                VdpVideoMixer mixer,
                                VdpVideoSurface sourceSurface[3], // Past present future
                                VdpOutputSurface targetOutputSurface, 
                                uint32_t targetWidth, 
                                uint32_t targetHeight )
{
    int nbPrev=2,nbNext=2;
    VdpVideoMixerPictureStructure fieldType=VDP_VIDEO_MIXER_PICTURE_STRUCTURE_TOP_FIELD;
    if(!topField) fieldType=VDP_VIDEO_MIXER_PICTURE_STRUCTURE_BOTTOM_FIELD;


    VdpVideoSurface past[2]={VDP_INVALID_HANDLE,VDP_INVALID_HANDLE};
    VdpVideoSurface future[2]={VDP_INVALID_HANDLE,VDP_INVALID_HANDLE};
    VdpVideoSurface present;

    present=sourceSurface[1];
    int index=0;
    if(!topField) index=1;

    if(VDP_INVALID_HANDLE==sourceSurface[0] ) nbPrev=0;
    else
    {
         
            past[0]=sourceSurface[index];
            past[1]=sourceSurface[0];
    }
    if(VDP_INVALID_HANDLE==sourceSurface[2] ) nbNext=0;
    else
    {
            future[0]=sourceSurface[1+index];
            future[1]=sourceSurface[2];
    }
    // 0 & 1 p
    //ADM_info("Deint : %d\n",(int)mixerIsFeatureEnabled(mixer,VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL));

      VdpStatus e=ADM_coreVdpau::funcs.mixerRender(mixer,
                VDP_INVALID_HANDLE,NULL,    // Background
                fieldType,
                nbPrev,       past, // Past...
                              present, // current
                nbNext,       future, // Future
                NULL,                               // source RECT
                targetOutputSurface,
                NULL,                               // dest Rec
                NULL,                               // dest video Rec
                0,NULL);                            // Layers
                
            
  if(VDP_STATUS_OK!=e)
    {
        
        ADM_warning("mixerRenderWithPastAndFuture  failed :%s\n",getErrorString(e));
        
    }
    return e;
}
#endif
// EOF
