/**********************************************************************
            \file            muxerMp4v2
            \brief           libmp4v2 muxer
                             -------------------
    
    copyright            : (C) 2011 by mean
    email                : fixounet@free.fr
    Strongly inspired by handbrake code

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdarg.h>
#include "ADM_default.h"
#include "ADM_cpp.h"
#include "fourcc.h"
#include "muxerLmkv.h"
#include "ADM_codecType.h"
#include "ADM_imageFlags.h"

#if 1
#define aprintf(...) {}
#define cprintf(...) {}
#else
#define aprintf printf
#endif


/**
    \fn     muxerLmkv
    \brief  Constructor
*/
muxerLmkv::muxerLmkv()
{
        ADM_info("[muxerLmkv] Creating\n");
};
/**
    \fn     muxerLmkv
    \brief  Destructor
*/

muxerLmkv::~muxerLmkv()
{
    ADM_info("[muxerLmkv] Destroying\n");
}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerLmkv::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{

        return false;
}
/**
    \fn save
*/
bool muxerLmkv::save(void)
{
  
    return false;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerLmkv::close(void)
{

    return true;
}
//EOF



