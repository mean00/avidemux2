/***************************************************************************
    \file avi_utils.h
    \author mean, fixounet@free.fr (C) 2012
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
#include "avifmt.h"
#include "avifmt2.h"
class ADM_videoStream;

 void mx_bihFromVideo(ADM_BITMAPINFOHEADER *_bih,ADM_videoStream *video);
 void mx_mainHeaderFromVideoStream(MainAVIHeader  *header,ADM_videoStream *video);
 void mx_streamHeaderFromVideo(AVIStreamHeader *header,ADM_videoStream *video);


// EOF

