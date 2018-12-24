/***************************************************************************
                          ADM_codecPcmFloat.cpp  -  description
                             -------------------    
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
#include <math.h>
#include "ADM_audiocodec.h"
/**
 * 
 * @param fourcc
 * @param info
 */
ADM_AudiocodecPcmFloat::ADM_AudiocodecPcmFloat( uint32_t fourcc,const WAVHeader &info ) : ADM_Audiocodec(fourcc,info)
{
    ADM_info("Creating Float PCM\n");
}
/**
 * 
 */
ADM_AudiocodecPcmFloat::~ADM_AudiocodecPcmFloat()
{

}
/**
 * 
 * @return 
 */
uint8_t ADM_AudiocodecPcmFloat::isCompressed( void )
{
 	return 0;
}
/**
 * 
 * @param inptr
 * @param nbIn
 * @param outptr
 * @param nbOut
 * @return 
 */
uint8_t ADM_AudiocodecPcmFloat::run(uint8_t * inptr, uint32_t nbIn, float * outptr, uint32_t * nbOut)
{
        // Just a passthrough
        memcpy(outptr, inptr,nbIn);
        *nbOut=nbIn/4;
        return true;
}


