/***************************************************************************
                          \fn     nvEnc
                          \brief  Plugin for nvEnc lav encoder
                             -------------------

    copyright            : (C) 2002/2016 by mean
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
#include "ADM_ffNvEnc.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ffnvenc_desc.cpp"
extern bool            ffNvEncConfigure(void);
extern ffnvenc_encoder NvEncSettings;

void resetConfigurationData()
{
	ffnvenc_encoder defaultConf = NVENC_CONF_DEFAULT;

	memcpy(&NvEncSettings, &defaultConf, sizeof(FFcodecSettings));
}


/**
 *  \fn nvCheckDll
 */

static bool nvCheckDll(const char *zwin64, const char *zwin32, const char *zlinux)
{
        const char *dll;
#ifdef _WIN32
        #ifdef _WIN64
            dll=zwin64;
        #else
            dll=zwin32;
        #endif
#else
        dll=zlinux;
#endif
        
        ADM_LibWrapper wrapper;
        bool r=wrapper.loadLibrary(dll);
        ADM_info("\t checking %s-> %d\n",dll,r);
        return r;
}
/**
 * \fn nvEncProbe
 */
extern "C"
{
    static bool nvEncProbe(void)
    {
        // Step 1 : Try to load cuda
        if(!nvCheckDll("nvcuda.dll","nvcuda.dll","libcuda.so"))
        {
            ADM_warning("Cannot load cuda dll\n");
            return false;
        }
        // Step 2 : Try to load encoder

        if(!nvCheckDll("nvEncodeAPI64.dll","nvEncodeAPI.dll","libnvidia-encode.so.1"))
        {
            ADM_warning("Cannot load nvidia encode dll\n");
            return false;
        }
        return true;
    }
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffNvEncEncoderHEVC);
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX("ffNvEncHEVC",
                               "Nvidia HEVC",
                               "Nvidia hw encoder",
                                ffNvEncConfigure, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                ffnvenc_encoder_param, // conf template
                                &NvEncSettings,NULL,NULL, // conf var
                                nvEncProbe
);
