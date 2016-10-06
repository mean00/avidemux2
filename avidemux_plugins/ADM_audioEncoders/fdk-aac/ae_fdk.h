
/***************************************************************************
    copyright            : (C) 2002-6 by mean
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
#pragma once
#include "fdk_encoder.h"
 /**
  * \class AUDMEncoder_Fdkaac
  */
class AUDMEncoder_Fdkaac : public ADM_AudioEncoder
{
protected:
            AACENC_BufDesc inDesc,outDesc;
            int inAudioId;
            int inAudioSize,inElemSize;
            void *inAudioPtr;
 
            int outAudioId;
            int outAudioSize,outElemSize;
            void *outAudioPtr;
protected:
         HANDLE_AACENCODER  _aacHandle;
         bool           _inited;
         uint32_t       _chunk;
         bool           _globalHeader;
         fdk_encoder    _config;
         float          *ordered;
         bool           setParam(const char *name, int nameAsInt, int value);
         bool           dumpConfiguration();
public:
            bool        initialize(void);
    virtual             ~AUDMEncoder_Fdkaac();
                        AUDMEncoder_Fdkaac(AUDMAudioFilter *instream,bool globalHeader,CONFcouple *setup);
    virtual bool	encode(uint8_t *dest, uint32_t *len, uint32_t *samples);
};

// EOF