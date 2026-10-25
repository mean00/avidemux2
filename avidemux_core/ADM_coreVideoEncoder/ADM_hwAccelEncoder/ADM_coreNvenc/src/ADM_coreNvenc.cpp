/***************************************************************************
    \fn ADM_coreNvenc.cpp
    \brief Probe codecs supported by NVENC interface and hw

    copyright            : (C) 2018 by mean
    email                : fixounet@free.fr
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
#include "ADM_coreNvenc.h"

extern "C" {
#include "dynlink_loader.h"
#include "nvEncodeAPI.h"
}

#define ADM_NVENC_NOT_PROBED 0xFFFF0000

static uint32_t nvEncCodecSupportFlags = ADM_NVENC_NOT_PROBED;

static admNvencCaps nvEncH264Caps;
static admNvencCaps nvEncHEVCCaps;
static admNvencCaps nvEncAV1Caps;

// Code stolen from libavcodec/nvenc.{c,h}

#define LAVC_NVENCAPI_CHECK_VERSION(major, minor) \
    ((major) < NVENCAPI_MAJOR_VERSION || \
    ((major) == NVENCAPI_MAJOR_VERSION && (minor) <= NVENCAPI_MINOR_VERSION))

static void nvencPrintDriverReq(void)
{
#if LAVC_NVENCAPI_CHECK_VERSION(13, 2)
    const char *minVersion = "(unknown)";
#elif LAVC_NVENCAPI_CHECK_VERSION(13, 1)
    const char *minVersion = "610.00";
#elif LAVC_NVENCAPI_CHECK_VERSION(13, 0)
    const char *minVersion = "570.0";
#elif LAVC_NVENCAPI_CHECK_VERSION(12, 2)
# ifdef _WIN32
    const char *minVersion = "551.76";
# else
    const char *minVersion = "550.54.14";
# endif
#elif LAVC_NVENCAPI_CHECK_VERSION(12, 1)
# ifdef _WIN32
    const char *minVersion = "531.61";
# else
    const char *minVersion = "530.41.03";
# endif
#elif LAVC_NVENCAPI_CHECK_VERSION(12, 0)
# ifdef _WIN32
    const char *minVersion = "522.25";
# else
    const char *minVersion = "520.56.06";
# endif
#else
# ifdef _WIN32
    const char *minVersion = "471.41";
# else
    const char *minVersion = "470.57.02";
# endif
#endif
    ADM_error("The minimum required NVIDIA driver for NVENC is %s\n", minVersion);
}

static int checkCap(NV_ENCODE_API_FUNCTION_LIST *list, void *instance, GUID encGuid, NV_ENC_CAPS cap)
{
    NV_ENC_CAPS_PARAM caps;
    int val = 0;
    memset(&caps, 0, sizeof(caps));
    caps.version = NV_ENC_CAPS_PARAM_VER;
    caps.capsToQuery = cap;
    NVENCSTATUS r = list->nvEncGetEncodeCaps(instance, encGuid, &caps, &val);

    return (r == NV_ENC_SUCCESS) ? val : -1;
}

static void checkH264Caps(NV_ENCODE_API_FUNCTION_LIST *list, void *instance)
{
    memset(&nvEncH264Caps, 0, sizeof(admNvencCaps));

    if (!(nvEncCodecSupportFlags & ADM_NVENC_H264_IS_SUPPORTED))
        return;

    nvEncH264Caps.codecId = ADM_NVENC_H264_IS_SUPPORTED;

    int v;

#define QUERY_CAP(x,y) v = checkCap(list, instance, NV_ENC_CODEC_H264_GUID, x); \
    if (v >= 0) \
    { \
        nvEncH264Caps.y = v; \
        ADM_info(#y " = %d\n", v); \
    };

    QUERY_CAP(NV_ENC_CAPS_SUPPORTED_RATECONTROL_MODES, rcMask)
    QUERY_CAP(NV_ENC_CAPS_WIDTH_MAX, maxWidth)
    QUERY_CAP(NV_ENC_CAPS_HEIGHT_MAX, maxHeight)
    QUERY_CAP(NV_ENC_CAPS_NUM_MAX_BFRAMES, maxBframes)

#undef QUERY_CAP
#define QUERY_CAP(x,y) v = checkCap(list, instance, NV_ENC_CODEC_H264_GUID, x); \
    if (v > 0) \
    { \
        nvEncH264Caps.y = true; \
        ADM_info(#y " = true\n"); \
    };

    QUERY_CAP(NV_ENC_CAPS_SUPPORT_LOSSLESS_ENCODE, lossless)
    QUERY_CAP(NV_ENC_CAPS_SUPPORT_TEMPORAL_AQ, temporalAq)
    QUERY_CAP(NV_ENC_CAPS_SUPPORT_WEIGHTED_PREDICTION, weightedPred)
    QUERY_CAP(NV_ENC_CAPS_SUPPORT_10BIT_ENCODE, highBitDepth)

#undef QUERY_CAP

    v = checkCap(list, instance, NV_ENC_CODEC_H264_GUID, NV_ENC_CAPS_SUPPORT_BFRAME_REF_MODE);
    if (v > 0)
    {
        nvEncH264Caps.bframeRef = v & 3;
        ADM_info("bframeRef raw mask = %d\n", v);
        v = checkCap(list, instance, NV_ENC_CODEC_H264_GUID, NV_ENC_CAPS_SUPPORT_HIERARCHICAL_BFRAMES);
        if (v > 0)
        {
            nvEncH264Caps.bframeRef |= (1<<3);
            ADM_info("Hierarchical B-frames are supported.\n");
        }
    }

    v = checkCap(list, instance, NV_ENC_CODEC_H264_GUID, NV_ENC_CAPS_SUPPORT_LOOKAHEAD);
    if (v > 0)
    {
        nvEncH264Caps.lookahead = 1;
        ADM_info("lookahead = true\n");
#if LAVC_NVENCAPI_CHECK_VERSION(12, 2)
        v = checkCap(list, instance, NV_ENC_CODEC_H264_GUID, NV_ENC_CAPS_SUPPORT_LOOKAHEAD_LEVEL);
        if (v > 0)
        {
            switch(v)
            {
                case NV_ENC_LOOKAHEAD_LEVEL_1:
                case NV_ENC_LOOKAHEAD_LEVEL_2:
                case NV_ENC_LOOKAHEAD_LEVEL_3:
                case NV_ENC_LOOKAHEAD_LEVEL_AUTOSELECT:
                    nvEncH264Caps.lookahead |= (v << 1);
                    ADM_info("Raw max lookahead level = %d\n", v);
                default: break; // invalid
            }
        }
#endif
    }
}

static void checkHEVCCaps(NV_ENCODE_API_FUNCTION_LIST *list, void *instance)
{
    memset(&nvEncHEVCCaps, 0, sizeof(admNvencCaps));

    if (!(nvEncCodecSupportFlags & ADM_NVENC_HEVC_IS_SUPPORTED))
        return;

    nvEncHEVCCaps.codecId = ADM_NVENC_HEVC_IS_SUPPORTED;

    int v;

#define QUERY_CAP(x,y) v = checkCap(list, instance, NV_ENC_CODEC_HEVC_GUID, x); \
    if (v >= 0) \
    { \
        nvEncHEVCCaps.y = v; \
        ADM_info(#y " = %d\n", v); \
    };

    QUERY_CAP(NV_ENC_CAPS_SUPPORTED_RATECONTROL_MODES, rcMask)
    QUERY_CAP(NV_ENC_CAPS_WIDTH_MAX, maxWidth)
    QUERY_CAP(NV_ENC_CAPS_HEIGHT_MAX, maxHeight)
    QUERY_CAP(NV_ENC_CAPS_NUM_MAX_BFRAMES, maxBframes)

#undef QUERY_CAP
#define QUERY_CAP(x,y) v = checkCap(list, instance, NV_ENC_CODEC_HEVC_GUID, x); \
    if (v > 0) \
    { \
        nvEncHEVCCaps.y = true; \
        ADM_info(#y " = true\n"); \
    };

    QUERY_CAP(NV_ENC_CAPS_SUPPORT_LOSSLESS_ENCODE, lossless)
    QUERY_CAP(NV_ENC_CAPS_SUPPORT_TEMPORAL_AQ, temporalAq)
    QUERY_CAP(NV_ENC_CAPS_SUPPORT_WEIGHTED_PREDICTION, weightedPred)
    QUERY_CAP(NV_ENC_CAPS_SUPPORT_10BIT_ENCODE, highBitDepth)

#undef QUERY_CAP

    v = checkCap(list, instance, NV_ENC_CODEC_HEVC_GUID, NV_ENC_CAPS_SUPPORT_BFRAME_REF_MODE);
    if (v > 0)
    {
        nvEncHEVCCaps.bframeRef = v & 3;
        ADM_info("bframeRef raw mask = %d\n", v);
        v = checkCap(list, instance, NV_ENC_CODEC_HEVC_GUID, NV_ENC_CAPS_SUPPORT_HIERARCHICAL_BFRAMES);
        if (v > 0)
        {
            nvEncHEVCCaps.bframeRef |= (1<<3);
            ADM_info("Hierarchical B-frames are supported.\n");
        }
    }

    v = checkCap(list, instance, NV_ENC_CODEC_HEVC_GUID, NV_ENC_CAPS_SUPPORT_LOOKAHEAD);
    if (v > 0)
    {
        nvEncHEVCCaps.lookahead = 1;
        ADM_info("lookahead = true\n");
#if LAVC_NVENCAPI_CHECK_VERSION(12, 2)
        v = checkCap(list, instance, NV_ENC_CODEC_HEVC_GUID, NV_ENC_CAPS_SUPPORT_LOOKAHEAD_LEVEL);
        if (v > 0)
        {
            switch(v)
            {
                case NV_ENC_LOOKAHEAD_LEVEL_1:
                case NV_ENC_LOOKAHEAD_LEVEL_2:
                case NV_ENC_LOOKAHEAD_LEVEL_3:
                case NV_ENC_LOOKAHEAD_LEVEL_AUTOSELECT:
                    nvEncHEVCCaps.lookahead |= (v << 1);
                    ADM_info("Raw max lookahead level = %d\n", v);
                default: break; // invalid
            }
        }
#endif
    }
}

static void checkAV1Caps(NV_ENCODE_API_FUNCTION_LIST *list, void *instance)
{
    memset(&nvEncAV1Caps, 0, sizeof(admNvencCaps));

#if LAVC_NVENCAPI_CHECK_VERSION(12, 0)
    if (!(nvEncCodecSupportFlags & ADM_NVENC_AV1_IS_SUPPORTED))
        return;

    nvEncAV1Caps.codecId = ADM_NVENC_AV1_IS_SUPPORTED;

    int v;

#define QUERY_CAP(x,y) v = checkCap(list, instance, NV_ENC_CODEC_AV1_GUID, x); \
    if (v >= 0) \
    { \
        nvEncAV1Caps.y = v; \
        ADM_info(#y " = %d\n", v); \
    };

    QUERY_CAP(NV_ENC_CAPS_SUPPORTED_RATECONTROL_MODES, rcMask)
    QUERY_CAP(NV_ENC_CAPS_WIDTH_MAX, maxWidth)
    QUERY_CAP(NV_ENC_CAPS_HEIGHT_MAX, maxHeight)
    QUERY_CAP(NV_ENC_CAPS_NUM_MAX_BFRAMES, maxBframes)

#undef QUERY_CAP
#define QUERY_CAP(x,y) v = checkCap(list, instance, NV_ENC_CODEC_AV1_GUID, x); \
    if (v > 0) \
    { \
        nvEncAV1Caps.y = true; \
        ADM_info(#y " = true\n"); \
    };

    QUERY_CAP(NV_ENC_CAPS_SUPPORT_LOSSLESS_ENCODE, lossless)
    QUERY_CAP(NV_ENC_CAPS_SUPPORT_TEMPORAL_AQ, temporalAq)
    QUERY_CAP(NV_ENC_CAPS_SUPPORT_WEIGHTED_PREDICTION, weightedPred)
    QUERY_CAP(NV_ENC_CAPS_SUPPORT_10BIT_ENCODE, highBitDepth)

#undef QUERY_CAP

    v = checkCap(list, instance, NV_ENC_CODEC_AV1_GUID, NV_ENC_CAPS_SUPPORT_BFRAME_REF_MODE);
    if (v > 0)
    {
        nvEncAV1Caps.bframeRef = v & 3;
        ADM_info("bframeRef raw mask = %d\n", v);
        v = checkCap(list, instance, NV_ENC_CODEC_AV1_GUID, NV_ENC_CAPS_SUPPORT_HIERARCHICAL_BFRAMES);
        if (v > 0)
        {
            nvEncAV1Caps.bframeRef |= (1<<3);
            ADM_info("Hierarchical B-frames are supported.\n");
        }
    }

    v = checkCap(list, instance, NV_ENC_CODEC_AV1_GUID, NV_ENC_CAPS_SUPPORT_LOOKAHEAD);
    if (v > 0)
    {
        nvEncAV1Caps.lookahead = 1;
        ADM_info("lookahead = true\n");
#if LAVC_NVENCAPI_CHECK_VERSION(12, 2)
        v = checkCap(list, instance, NV_ENC_CODEC_AV1_GUID, NV_ENC_CAPS_SUPPORT_LOOKAHEAD_LEVEL);
        if (v > 0)
        {
            switch(v)
            {
                case NV_ENC_LOOKAHEAD_LEVEL_1:
                case NV_ENC_LOOKAHEAD_LEVEL_2:
                case NV_ENC_LOOKAHEAD_LEVEL_3:
                case NV_ENC_LOOKAHEAD_LEVEL_AUTOSELECT:
                    nvEncAV1Caps.lookahead |= (v << 1);
                    ADM_info("Raw max lookahead level = %d\n", v);
                default: break; // invalid
            }
        }
#endif
    }
#endif
}

static bool checkNvEnc(void)
{
    CudaFunctions *cudaDynl = NULL;
    NvencFunctions *nvencDynl = NULL;
    NV_ENCODE_API_FUNCTION_LIST nvencFunc;
    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS params;

    NVENCSTATUS err;
    CUresult cuerr;
    uint32_t nvenc_max_ver, nbGuids;
    GUID *guids;
    int ret;
    int nbDevices = 0;

    typedef struct { const char *name; GUID guid; } namedGuid;

    namedGuid allGuids[] = {
         { "H264", NV_ENC_CODEC_H264_GUID }
        ,{ "HEVC", NV_ENC_CODEC_HEVC_GUID }
#if LAVC_NVENCAPI_CHECK_VERSION(12, 0)
        ,{ "AV1", NV_ENC_CODEC_AV1_GUID }
#endif
    };

    if (!(nvEncCodecSupportFlags & ADM_NVENC_NOT_PROBED))
    {
        ADM_info("Already probed, skipping check.\n");
        return (nvEncCodecSupportFlags & (ADM_NVENC_H264_IS_SUPPORTED + ADM_NVENC_HEVC_IS_SUPPORTED + ADM_NVENC_AV1_IS_SUPPORTED));
    }
    nvEncCodecSupportFlags &= ~ADM_NVENC_NOT_PROBED;

    ret = cuda_load_functions(&cudaDynl, NULL);
    if (ret < 0)
        return false;

    ret = nvenc_load_functions(&nvencDynl, NULL);
    if (ret < 0)
    {
        nvencPrintDriverReq();
        cuda_free_functions(&cudaDynl);
        return false;
    }

    err = nvencDynl->NvEncodeAPIGetMaxSupportedVersion(&nvenc_max_ver);
    if (err != NV_ENC_SUCCESS)
    {
        ADM_warning("Failed to query NVENC max version.\n");
        goto fail;
    }

    ADM_info("Loaded NVENC version %d.%d\n", nvenc_max_ver >> 4, nvenc_max_ver & 0xf);

    if ((NVENCAPI_MAJOR_VERSION << 4 | NVENCAPI_MINOR_VERSION) > nvenc_max_ver)
    {
        ADM_error("Driver supports NVENC API up to version %d.%d but %d.%d is required.\n",
                nvenc_max_ver >> 4, nvenc_max_ver & 0xF,
                NVENCAPI_MAJOR_VERSION, NVENCAPI_MINOR_VERSION);
        nvencPrintDriverReq();
        goto fail;
    }

    nvencFunc.version = NV_ENCODE_API_FUNCTION_LIST_VER;

    err = nvencDynl->NvEncodeAPICreateInstance(&nvencFunc);
    if (err != NV_ENC_SUCCESS)
    {
        ADM_error("Failed to create NVENC instance.\n");
        goto fail;
    }

    ADM_info("NVENC libraries loaded and instance created successfully.\n");

    cuerr = cudaDynl->cuInit(0);
    if (cuerr != CUDA_SUCCESS)
    {
        ADM_error("Cannot init CUDA, NVENC is not available.\n");
        goto fail;
    }

    cuerr = cudaDynl->cuDeviceGetCount(&nbDevices);
    if (cuerr != CUDA_SUCCESS)
    {
        ADM_error("Cannot query CUDA-capable devices.\n");
        goto fail;
    }

    if (!nbDevices)
    {
        ADM_warning("No CUDA-capable devices found, NVENC is not available.\n");
        goto fail;
    }

    ADM_info("%d CUDA-capable device(s) found.\n", nbDevices);

    for (int i = 0; i < nbDevices; i++)
    {
#define GPU_NAME_BUF_SIZE 128
        char gpu[GPU_NAME_BUF_SIZE] = {0};
        CUdevice dev;
        CUcontext ctx, dummy;
        void *encoder = NULL;
        int major, minor;

        cuerr = cudaDynl->cuDeviceGet(&dev, i);
        if (cuerr != CUDA_SUCCESS)
        {
            ADM_error("Cannot get CUDA device %d.\n", i);
            continue;
        }

        cuerr = cudaDynl->cuDeviceGetName(gpu, GPU_NAME_BUF_SIZE, dev);
        if (cuerr != CUDA_SUCCESS)
        {
            ADM_error("Cannot query CUDA device %d name.\n", i);
            continue;
        }

        gpu[GPU_NAME_BUF_SIZE - 1] = 0;
        ADM_info("Checking CUDA device %d \"%s\"\n", i, gpu);

        cuerr = cudaDynl->cuDeviceComputeCapability(&major, &minor, dev);
        if (cuerr != CUDA_SUCCESS)
        {
            ADM_error("Cannot query CUDA device %d compute capabilities.\n", i);
            continue;
        }

        ADM_info("GPU #%d - \"%s\" streaming multiprocessor version: %d.%d\n", i, gpu, major, minor);
        if (((major << 4) | minor) < 0x30)
        {
            ADM_warning("Version too low for NVENC\n");
            continue;
        }

        cuerr = cudaDynl->cuCtxCreate(&ctx, 0, dev);
        if (cuerr != CUDA_SUCCESS)
        {
            ADM_warning("Cannot create CUDA context.\n");
            continue;
        }

        cuerr = cudaDynl->cuCtxPopCurrent(&dummy);
        if (cuerr != CUDA_SUCCESS)
        {
            ADM_warning("Cannot pop CUDA context.\n");
            cuerr = cudaDynl->cuCtxDestroy(ctx);
            if (cuerr != CUDA_SUCCESS)
            {
                ADM_error("Cannot destroy CUDA context, bailing out.\n");
                goto fail;
            }
            ctx = NULL;
            continue;
        }

        memset(&params, 0, sizeof(params));
        params.version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
        params.apiVersion = NVENCAPI_VERSION;
        params.device = ctx;
        params.deviceType = NV_ENC_DEVICE_TYPE_CUDA;

        err = nvencFunc.nvEncOpenEncodeSessionEx(&params, &encoder);
        if (err != NV_ENC_SUCCESS)
        {
            ADM_warning("Failed to open session.\n");
            err = nvencFunc.nvEncDestroyEncoder(encoder);
            encoder = NULL;
            if (err != NV_ENC_SUCCESS)
            {
                ADM_error("Cannot destroy encoder instance, bailing out.\n");
                goto fail;
            }
            continue;
        }
        nbGuids = 0;
        err = nvencFunc.nvEncGetEncodeGUIDCount(encoder, &nbGuids);
        if (err != NV_ENC_SUCCESS || !nbGuids)
        {
            ADM_warning("Encoder does not support any codecs.\n");
            err = nvencFunc.nvEncDestroyEncoder(encoder);
            encoder = NULL;
            if (err != NV_ENC_SUCCESS)
            {
                ADM_error("Cannot destroy encoder instance, bailing out.\n");
                nvenc_free_functions(&nvencDynl);
                cuda_free_functions(&cudaDynl);
                return false;
            }
            continue;
        }

        guids = (GUID *) ADM_alloc(nbGuids * sizeof(GUID));
        ADM_assert(guids);
        err = nvencFunc.nvEncGetEncodeGUIDs(encoder, guids, nbGuids, &nbGuids);
        if (err != NV_ENC_SUCCESS || !nbGuids)
        {
            ADM_warning("Encoder does not support any codecs.\n");
            ADM_dealloc(guids);
            guids = NULL;
            err = nvencFunc.nvEncDestroyEncoder(encoder);
            encoder = NULL;
            if (err != NV_ENC_SUCCESS)
            {
                ADM_error("Cannot destroy encoder instance, bailing out.\n");
                cudaDynl->cuCtxPopCurrent(&dummy);
                cudaDynl->cuCtxDestroy(ctx);
                goto fail;
            }
            continue;
        }

        for (int j = 0; j < nbGuids; j++)
        {
            for (int k = 0; k < sizeof(allGuids) / sizeof(namedGuid); k++)
            {
                if (!memcmp(&(guids[j]), &(allGuids[k].guid), sizeof(*guids)))
                {
                    ADM_info("%s is supported.\n", allGuids[k].name);
                    if (!strcmp("H264", allGuids[k].name))
                        nvEncCodecSupportFlags |= ADM_NVENC_H264_IS_SUPPORTED;
                    else if (!strcmp("HEVC", allGuids[k].name))
                        nvEncCodecSupportFlags |= ADM_NVENC_HEVC_IS_SUPPORTED;
                    else if (!strcmp("AV1", allGuids[k].name))
                        nvEncCodecSupportFlags |= ADM_NVENC_AV1_IS_SUPPORTED;
                }
            }
        }

        if (nvEncCodecSupportFlags & ADM_NVENC_H264_IS_SUPPORTED)
            checkH264Caps(&nvencFunc, encoder);

        if (nvEncCodecSupportFlags & ADM_NVENC_HEVC_IS_SUPPORTED)
            checkHEVCCaps(&nvencFunc, encoder);

        if (nvEncCodecSupportFlags & ADM_NVENC_AV1_IS_SUPPORTED)
            checkAV1Caps(&nvencFunc, encoder);

        ADM_dealloc(guids);
        guids = NULL;

        err = nvencFunc.nvEncDestroyEncoder(encoder);
        encoder = NULL;

        cudaDynl->cuCtxPopCurrent(&dummy);
        cudaDynl->cuCtxDestroy(ctx);

        if (err != NV_ENC_SUCCESS)
        {
            ADM_warning("Cannot destroy encoder instance!\n");
            goto fail;
        }
    }

fail:
    nvenc_free_functions(&nvencDynl);
    cuda_free_functions(&cudaDynl);

    return (nvEncCodecSupportFlags & (ADM_NVENC_H264_IS_SUPPORTED + ADM_NVENC_HEVC_IS_SUPPORTED + ADM_NVENC_AV1_IS_SUPPORTED));
}

bool ADM_getNvencCodecSupportFlags(admNvencCaps *caps)
{
    bool r = checkNvEnc();
    if (caps)
    {
        if (!r)
        {
            memset(caps, 0, sizeof(admNvencCaps));
            return r;
        }
        switch(caps->codecId)
        {
            case ADM_NVENC_H264_IS_SUPPORTED:
                memcpy(caps, &nvEncH264Caps, sizeof(admNvencCaps));
                break;
            case ADM_NVENC_HEVC_IS_SUPPORTED:
                memcpy(caps, &nvEncHEVCCaps, sizeof(admNvencCaps));
                break;
            case ADM_NVENC_AV1_IS_SUPPORTED:
                memcpy(caps, &nvEncAV1Caps, sizeof(admNvencCaps));
                break;
            default:
                memset(caps, 0, sizeof(admNvencCaps));
                break;
        }
    }

    return r;
}

// EOF
