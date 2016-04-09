/**
    \file  ADM_audioWriteAAC
    \brief Writer

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
#pragma once
#include "ADM_audioWrite.h"
/**
 * 
 * @param stream
 * @return 
 */
class ADM_audioWriteAAC: public ADM_audioWrite
{
protected:
             bool          writeHeader(ADM_audioStream *stream);
             bool          updateHeader(void);

public:
                  ADM_audioWriteAAC();
virtual      bool close(void);
virtual      bool init(ADM_audioStream *stream, const char *fileName);
                  
virtual      bool canBeBuffered() {return false;}; 
virtual      bool write(uint32_t size, uint8_t *buffer);

protected:
            uint8_t  aacHeader[7];
};


