/**
    \file ADM_imageLoader
*/
//
#ifndef ADM_IMAGE_LOADER
#define ADM_IMAGE_LOADER
#include "ADM_default.h"
#include "ADM_image.h"
typedef enum 
{
		ADM_IMAGE_UNKNOWN=0,
        ADM_IMAGE_JPG=1,
        ADM_IMAGE_PNG=2,
        ADM_IMAGE_BMP=3,
        ADM_IMAGE_BMP2=4
        
} ADM_IMAGE_TYPE;

ADMImage *createImageFromFile(const char *filename);
ADM_IMAGE_TYPE ADM_identidyImageFile(const char *filename,uint32_t *w,uint32_t *h);
#endif
