/***************************************************************************
                          ADM_png  -  description
                             -------------------
    copyright            : (C) 2005 by mean
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
#ifndef PNG_H_A
#define PNG_H_A

/**
    \struct memAccess   
    \brief png io wrapper
*/
typedef struct memAccess 
{
  
int size;   
int cur;
uint8_t * data;
} memAccess;

/**
    \class decoderPng
*/
class decoderPng:public decoders 
{

protected:
    memAccess io;      
    ADM_colorspace colorspace;
    void *png_ptr;
    void *info_ptr;
    void *end_info;
    uint8_t ** rows;      
    uint8_t * decoded;
    void Init (void);
    void Cleanup (void);
    void recalc (void);

public:
            decoderPng (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
  
    virtual ~ decoderPng ();
  
    virtual bool uncompress(ADMCompressedImage * in, ADMImage * out);
  
    bool dontcopy (void)
            {
                return true;
            }
};

#endif 
//EOF
