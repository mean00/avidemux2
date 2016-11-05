/***************************************************************************
    \file             : ADM_coreDxva2.cpp
    \brief            : Wrapper around dxva functions
    \author           : (C) 2016 by mean fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once
#include "ADM_image.h"
#include "ADM_windowInfo.h"
extern "C" 
{
 #include "libavcodec/avcodec.h"
}
/**
 */
class admDx2Surface 
{
public:  
                          admDx2Surface(void *parent);
                          ~admDx2Surface();
    void                 *parent;
    LPDIRECT3DSURFACE9    surface;
    IDirectXVideoDecoder *decoder;
public:    
    bool                  addRef();
    bool                  removeRef();
} ;
/**
    \class admVdpau
*/
class admDxva2
{
public:
        static bool init(GUI_WindowInfo *x);
        static bool isOperationnal(void);
        static bool cleanup(void);
        static bool supported(AVCodecID codec);
        static bool allocateD3D9Surface(int num,int width, int height,void *array);
        static bool destroyD3DSurface(int num,void *surfaces);
        static IDirectXVideoDecoder *createDecoder(AVCodecID codec,int paddedWidth, int paddedHeight,int numSurface, LPDIRECT3DSURFACE9 *surface);
        static bool destroyDecoder(IDirectXVideoDecoderService *decoder);
        static DXVA2_ConfigPictureDecode *getDecoderConfig(AVCodecID codec);
        static bool surfaceToAdmImage(LPDIRECT3DSURFACE9 surface, ADMImage *out);
};
