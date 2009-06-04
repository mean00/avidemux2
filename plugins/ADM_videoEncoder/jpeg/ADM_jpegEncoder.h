/***************************************************************************
                          \fn ADM_VideoEncoders
                          \brief Internal handling of video encoders
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
#ifndef ADM_jpeg_ENCODER_H
#define ADM_jpeg_ENCODER_H
#include "ADM_coreVideoEncoder.h"


/**
        \class ADM_jpegEncoder
        \brief Dummy encoder that does nothing

*/
class ADM_jpegEncoder : public ADM_coreVideoEncoder
{
protected:
               int plane;
public:

                            ADM_jpegEncoder(ADM_coreVideoFilter *src);
                            ~ADM_jpegEncoder();
virtual        bool         encode (ADMBitstream * out);
virtual        const char *getDisplayName(void);       /// E.g. FFmpeg mpeg4      
virtual        const char *getCodecName(void);         /// aka fourcc
virtual        const char *getFCCHandler(void);        /// fourcc 2, needed to build AVI header

virtual        bool        configure(void);           /// Pop-up UI
virtual        bool        getConfiguration(uint32_t *l,uint8_t **d); /// Get current conf to save it
virtual        bool        setConfiguration(uint32_t l,uint8_t *d);   /// Set conf, call it just after creation
};


#endif
