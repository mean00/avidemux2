/***************************************************************************
                          \fn ADM_videoEncoderApi.h
                          \brief Api to deal with video encoder
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
#ifndef ADM_VIDEO_ENCODER_API_H
#define ADM_VIDEO_ENCODER_API_H
class ADM_coreVideoEncoder;
class ADM_coreVideoFilter;
class CONFcouple;
// Spawn a new encoder using the index from the menu = the index in the vector
//ADM_coreVideoEncoder *createVideoEncoderFromIndex(ADM_coreVideoFilter *chain,int index);

bool                  videoEncoder6SelectByName(const char *name);
bool                  videoEncoder6Configure(void);

int                   videoEncoder6_GetIndexFromName(const char *name);

bool                  videoEncoder6_SetCurrentEncoder(uint32_t index);

const char            *videoEncoder6_GetCurrentEncoderName(void);

bool                  videoEncoder6_SetConfiguration(CONFcouple *c);
bool                  videoEncoder6_GetConfiguration(CONFcouple **c);

ADM_coreVideoEncoder *createVideoEncoderFromIndex(ADM_coreVideoFilter *chain,int index,bool globalHeader);

#endif
