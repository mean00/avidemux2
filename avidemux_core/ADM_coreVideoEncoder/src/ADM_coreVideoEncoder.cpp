/***************************************************************************
                          \fn ADM_coreVideoEncoder
                          \brief Base class for video encoder plugin
                             -------------------
    
    copyright            : (C) 2002/2009 by mean
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
#include "ADM_coreVideoEncoder.h"

/**
    \fn ADM_coreVideoEncoder
*/                          
ADM_coreVideoEncoder::ADM_coreVideoEncoder(ADM_coreVideoFilter *src)
{
    source=src;
    image=NULL;
    encoderDelay=0;
}

/**
    \fn ADM_coreVideoEncoder
*/                          
ADM_coreVideoEncoder::~ADM_coreVideoEncoder()
{
    if(image) delete image;
    image=NULL;
}
// EOF