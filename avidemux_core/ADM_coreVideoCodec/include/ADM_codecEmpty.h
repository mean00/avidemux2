/***************************************************************************
         \fn ADM_codecEmpty.h
         \brief Empty decoder
         \author mean, fixounet@free.fr (C) 2002-2010
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_codecEmpty_H
#define ADM_codecEmpty_H
/* Dummy decoder in case we don't have the desired one */
class decoderEmpty : public decoders
{
protected:
public:
    decoderEmpty (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
        :decoders (  w,   h,  fcc,   extraDataLen,  extraData,  bpp)
    {

    }
    bool uncompress (ADMCompressedImage * in, ADMImage * out) {return true;}

};
#endif
