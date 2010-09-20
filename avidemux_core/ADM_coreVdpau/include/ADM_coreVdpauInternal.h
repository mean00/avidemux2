/**

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_CORE_VDPAU_INTERNAL_H
#define ADM_CORE_VDPAU_INTERNAL_H

/**
    \fn VdpFunctions
    
*/
typedef struct 
{
    VdpGetErrorString       *getErrorString;
    VdpGetApiVersion        *getApiVersion;
    VdpGetInformationString *getInformationString;

    VdpDeviceDestroy        *deviceDestroy;

    VdpVideoSurfaceCreate   *createSurface;
    VdpVideoSurfaceDestroy  *destroySurface;
    VdpVideoSurfaceGetBitsYCbCr *getDataSurface;
    VdpVideoSurfacePutBitsYCbCr *surfacePutBitsYCbCr;

 

    VdpOutputSurfaceCreate  *createOutputSurface;
    VdpOutputSurfaceDestroy *destroyOutputSurface;
    VdpOutputSurfacePutBitsYCbCr *putBitsYV12OutputSurface;
    VdpOutputSurfaceQueryPutBitsYCbCrCapabilities *putBitsCapsOutputSurface;
    VdpOutputSurfaceGetBitsNative                 *getBitsNativeOutputSurface;

    VdpDecoderCreate        *decoderCreate;
    VdpDecoderDestroy       *decoderDestroy;
    VdpDecoderRender        *decoderRender;


    VdpPresentationQueueTargetDestroy *presentationQueueDestroy;
    VdpPresentationQueueCreate        *presentationQueueCreate;
    VdpPresentationQueueGetTime       *presentationQueueGetTime;
    VdpPresentationQueueDisplay       *presentationQueueDisplay;

    VdpVideoMixerCreate               *mixerCreate;
    VdpVideoMixerDestroy              *mixerDestroy;
    VdpVideoMixerRender               *mixerRender;
    VdpVideoMixerSetFeatureEnables    *mixerEnableFeatures;
    VdpVideoMixerGetFeatureEnables    *mixerGetFeaturesEnabled;
    VdpVideoMixerQueryFeatureSupport  *mixerQueryFeatureSupported;

    VdpPresentationQueueTargetCreateX11 *presentationQueueDisplayX11Create;
}VdpFunctions;

namespace ADM_coreVdpau
{
 extern VdpFunctions          funcs;
 extern VdpDevice             vdpDevice;
}

#define CHECK(x) if(!isOperationnal()) {ADM_error("vdpau is not operationnal\n");return VDP_STATUS_ERROR;}\
                 VdpStatus r=x;\
                 if(VDP_STATUS_OK!=r) {ADM_warning(#x" call failed with error=%s\n",getErrorString(r));}return r;

#define CHECKBOOL(x) if(!isOperationnal())\
                    {ADM_error("vdpau is not operationnal\n");return false;}\
                 VdpStatus r=x;\
                 if(VDP_STATUS_OK!=r)  \
                    {\
                    ADM_warning(#x" call failed with error=%s\n",getErrorString(r));\
                    return false;};

#endif