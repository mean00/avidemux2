/** *************************************************************************
    \fn ADM_fileio.h
    \brief Handle file access through a class
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef ADM_FILE_IO
#define ADM_FILE_IO

#include "ADM_coreUtils6_export.h"

class ADM_COREUTILS6_EXPORT ADMFile
{
protected:
        FILE            *_out;
        uint32_t        _fill;
        uint8_t         *_buffer;	  
        uint64_t        _curPos;
public:
                        ADMFile();
                        ~ADMFile();
        uint8_t         open(FILE *in);
        uint8_t         write(const uint8_t *in, uint32_t size);
        uint8_t         flush(void);
        uint8_t         seek(uint64_t where);
        uint64_t        tell(void);
	  


};

#endif
