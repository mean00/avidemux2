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

#include "fourcc.h"
#include "avilist.h"

uint32_t ADM_UsecFromFps1000(uint32_t fps1000);


/**
    \fn mx_bihFromVideo
    \brief build a bih from video
*/
void mx_bihFromVideo(ADM_BITMAPINFOHEADER *bih,ADM_videoStream *video)
{
        memset(bih,0,sizeof(*bih));
        //
         bih->biSize=sizeof(ADM_BITMAPINFOHEADER); //uint32_t 	biSize;
         bih->biWidth=video->getWidth(); //uint32_t  	biWidth;
         bih->biHeight=video->getHeight(); //uint32_t  	biHeight;
         bih->biPlanes=1; //    uint16_t 	biPlanes;
         bih->biBitCount=24; //
         bih->biCompression=video->getFCC(); //    uint32_t 	biCompression;
         bih->biSizeImage=(bih->biWidth*bih->biHeight*3)>>1;//    uint32_t 	biSizeImage;
         bih->biXPelsPerMeter=0;
         bih->biYPelsPerMeter=0;
         bih->biClrUsed=0;
         bih->biClrImportant=0;
        // Recompute image size
        uint32_t is;
            is=bih->biWidth*bih->biHeight;
            is*=(bih->biBitCount+7)/8;
            bih->biSizeImage=is;
}
/**
        \fn mx_mainHeaderFromVideoStream
        \brief Write MainAVIHeader from video
*/
void mx_mainHeaderFromVideoStream(MainAVIHeader  *header,ADM_videoStream *video)
{
    memset(header,0,sizeof(*header));
    header->dwMicroSecPerFrame= ADM_UsecFromFps1000(video->getAvgFps1000()); //int32_t	dwMicroSecPerFrame;	// frame display rate (or 0L)
    header->dwMaxBytesPerSec=8*1000*1000; //int32_t	dwMaxBytesPerSec;	// max. transfer rate
    header->dwPaddingGranularity=0; //int32_t	dwPaddingGranularity;	// pad to multiples of this
					// size; normally 2K.
    header->dwFlags= AVIF_HASINDEX + AVIF_ISINTERLEAVED; // FIXME HAS INDEX //int32_t	dwFlags;		// the ever-present flags
    //header->dwTotalFrames=0; //int32_t	dwTotalFrames;		// # frames in file
    header->dwInitialFrames=0; //int32_t	dwInitialFrames;
   // Must be set by caller  header->dwStreams=int32_t	dwStreams;
    header->dwSuggestedBufferSize=64*1024;// int32_t	dwSuggestedBufferSize;

    header->dwWidth=video->getWidth();//int32_t	dwWidth;
    header->dwHeight=video->getHeight();//int32_t	dwHeight;
}
/**
    \fn mx_streamHeaderFromVideo
    \fill in AVIStreamHeader from video. Only for video stream header of course.

*/
 void mx_streamHeaderFromVideo(AVIStreamHeader *header,ADM_videoStream *video)
{
    memset(header,0,sizeof(*header));
	header->fccType=fourCC::get((uint8_t *)"vids");  //uint32_t	fccType;
	header->fccHandler=video->getFCC(); //uint32_t	fccHandler;
	header->dwFlags=0; //int32_t	dwFlags;	/* Contains AVITF_* flags */
	header->wPriority=0; //int16_t	wPriority;	/* dwPriority - splited for audio */
	header->wLanguage=0; //int16_t	wLanguage;
	header->dwInitialFrames=0;//int32_t	dwInitialFrames;
	header->dwScale=1000;//  int32_t	dwScale;
	header->dwRate=video->getAvgFps1000();// int32_t	dwRate;		/* dwRate / dwScale == samples/second */
	header->dwStart=0;// int32_t	dwStart;
	header->dwLength=0; // int32_t	dwLength;	/* In units above... */
	header->dwSuggestedBufferSize=1000000;// int32_t	dwSuggestedBufferSize;
	header->dwQuality=0;// int32_t	dwQuality;
	header->dwSampleSize=0;// int32_t	dwSampleSize;
/*
	struct {
		int16_t left;
		int16_t top;
		int16_t right;
		int16_t bottom;
	} rcFrame;
*/
}

// EOF