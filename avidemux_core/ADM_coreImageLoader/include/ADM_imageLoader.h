/**
    \file ADM_imageLoader
*/
//
#pragma once
#include "ADM_image.h"
/**
 */
typedef enum 
{
        ADM_PICTURE_UNKNOWN=0,
        ADM_PICTURE_JPG=1,
        ADM_PICTURE_PNG=2,
        ADM_PICTURE_BMP=3,
        ADM_PICTURE_BMP2=4
        
} ADM_PICTURE_TYPE;


ADM_PICTURE_TYPE ADM_identifyImageFile(const char *filename,uint32_t *w,uint32_t *h);

ADMImage *createImageFromFile(const char *filename);
