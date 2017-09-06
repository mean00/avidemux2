
/***************************************************************************
    copyright            : (C) 2017 mean (c)
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
#include "opus_encoder.h"
/** \class AUDMEncoder_Opus
 *  \brief Wrapper around opus encoder library
 * 
 */
class AUDMEncoder_Opus : public ADM_AudioEncoder
{
protected:
        OpusEncoder    *_handle;
        uint32_t        _chunk;
        bool           _globalHeader;
        opus_encoder   _config;
public:
            bool        initialize(void);
    virtual             ~AUDMEncoder_Opus();
                        AUDMEncoder_Opus(AUDMAudioFilter *instream,bool globalHeader,CONFcouple *setup);
    virtual bool	encode(uint8_t *dest, uint32_t *len, uint32_t *samples);
};

// EOF