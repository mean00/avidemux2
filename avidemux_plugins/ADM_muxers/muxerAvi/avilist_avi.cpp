/** *************************************************************************
        \file op_aviwrite.cpp
        \brief low level avi muxer

		etc...


    copyright            : (C) 2002 by mean
                           (C) Feb 2005 by GMV: ODML write support
    GPL V2.0
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_cpp.h"
#include "ADM_default.h"
#include <math.h>
#include "ADM_muxer.h"

#include "avifmt.h"
#include "avifmt2.h"
#include "fourcc.h"

#include "avilist_avi.h"
#define w32(x) Write32(hdr.x);
#define w16(x) Write16(hdr.x);
/**
    \fn writeMainHeader
*/
bool  AviListAvi::writeMainHeaderStruct(const MainAVIHeader &hdr)
{
    
    w32(dwMicroSecPerFrame);	// frame display rate (or 0L)
    w32(dwMaxBytesPerSec);	// max. transfer rate
    w32(dwPaddingGranularity);	// pad to multiples of this
					// size; normally 2K.
    w32(dwFlags);		// the ever-present flags
    w32(dwTotalFrames);		// # frames in file
    w32(dwInitialFrames);
    w32(dwStreams);
    w32(dwSuggestedBufferSize);

    w32(dwWidth);
    w32(dwHeight);

    for(int i=0;i<4;i++)
        Write32((uint32_t)0);
    return true;
}
/**
    \fn writeStreamHeader
*/
bool  AviListAvi::writeStreamHeaderStruct(const AVIStreamHeader &hdr)
{

	w32(	fccType);
	w32(	fccHandler);
	w32(	dwFlags);	/* Contains AVITF_* flags */
	w16(	wPriority);	/* dwPriority - splited for audio */
	w16(	wLanguage);
	w32(	dwInitialFrames);
	w32(	dwScale);
	w32(	dwRate);		/* dwRate / dwScale == samples/second */
	w32(	dwStart);
	w32(	dwLength);	/* In units above... */
	w32(	dwSuggestedBufferSize);
	w32(	dwQuality);
	w32(	dwSampleSize);
	w16( rcFrame.left);
    w16( rcFrame.top);
    w16( rcFrame.right);
    w16( rcFrame.bottom);
    return true;
}
/**
     \fn writeStrh
     \brief write a complete strh chunk
*/
bool  AviListAvi::writeStrh(const AVIStreamHeader &hdr)
{
    
    Write32((uint8_t *) "strh");
    Write32(sizeof (AVIStreamHeader));
    writeStreamHeaderStruct(hdr);
    return true;
}
/**
        \fn writeBihStruct
*/
bool AviListAvi::writeBihStruct(const ADM_BITMAPINFOHEADER &hdr)
{
    w32( 	biSize);
    w32(  	biWidth);
    w32(  	biHeight);
    w16( 	biPlanes);
    w16( 	biBitCount);
    w32( 	biCompression);
    w32( 	biSizeImage);
    w32(  	biXPelsPerMeter);
    w32(  	biYPelsPerMeter);
    w32( 	biClrUsed);
    w32( 	biClrImportant);
    return true;
}
/**
        \fn writeWavStruct
*/
bool AviListAvi::writeWavStruct(const WAVHeader &hdr)
{
    w16(	encoding);	
    w16(	channels);					/* 1 = mono, 2 = stereo */
    w32(	frequency);				/* One of 11025, 22050, or 44100 48000 Hz */
    w32(	byterate);					/* Average bytes per second */
    w16(	blockalign);				/* Bytes per sample block */
    w16(	bitspersample);		/* One of 8, 1 */
    return true;
}
/**
    \fn writeStrfBih
*/
bool  AviListAvi::writeStrfBih(const ADM_BITMAPINFOHEADER &bih, int extraLen, uint8_t *extraData)
{

    int toWrite=sizeof(bih)+extraLen;
 
    Write32((uint8_t *) "strf");
    Write32(toWrite);
    writeBihStruct(bih);
    if(extraLen)
        Write(extraData,extraLen);
    return true;
}

/**
    \fn writeStrfBih
*/
bool  AviListAvi::writeStrfWav(const WAVHeader &wav, int extraLen, uint8_t *extraData)
{
    int toWrite=sizeof(wav)+extraLen;
  
    Write32((uint8_t *) "strf");
    Write32(toWrite);
    writeWavStruct(wav);
    if(extraLen)
        Write(extraData,extraLen);
    return true;
}


// EOF

