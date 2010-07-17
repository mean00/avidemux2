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

#ifdef USE_VDPAU
#include "../include/ADM_coreVdpauInternal.h"
#include "ADM_dynamicLoading.h"


//GUI_WindowInfo      admVdpau::myWindowInfo;

/**
    \fn    mixerEnableFeature
    \brief enable mixer feature
*/

VdpStatus admVdpau::mixerEnableFeature( VdpVideoMixer mixer,uint32_t nbFeature,VdpVideoMixerFeature *feature,VdpBool *enabledFeature)
{
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
const VdpVideoSurface listOfSurface[1]={sourceSurface};
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
                
            
  if(VDP_STATUS_OK!=e)
    {
        
        ADM_warning("MixerCreate  failed :%s\n",getErrorString(e));
        
    }
    return e;
}

/**
    \fn mixerRenderWithPastAndFuture
*/

VdpStatus admVdpau::mixerRenderWithPastAndFuture(VdpVideoMixer mixer,
                                VdpVideoSurface sourceSurface[3], // Past present future
                                VdpOutputSurface targetOutputSurface, 
                                uint32_t targetWidth, 
                                uint32_t targetHeight )
{
      VdpStatus e=ADM_coreVdpau::funcs.mixerRender(mixer,
                VDP_INVALID_HANDLE,NULL,    // Background
                VDP_VIDEO_MIXER_PICTURE_STRUCTURE_FRAME,
                1,            sourceSurface+0, // Past...
                              sourceSurface[1], // current
                1,            sourceSurface+2, // Future
                NULL,                               // source RECT
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
#endif
// EOF
