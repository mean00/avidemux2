/***************************************************************************
                         
     Internal Interface for OCR engine
     
    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

#ifndef ADM_OCR_INTERNAL_H
#define ADM_OCR_INTERNAL_H
 
#include "adm_glyph.h"


typedef enum 
{
        ReplyOk=1,
        ReplyClose=0,
        ReplyCalibrate=2,
        ReplySkip=3,
        ReplySkipAll=4
}ReplyType;

// GUI independant part
ReplyType handleGlyph(uint8_t *workArea,uint32_t start, uint32_t end,uint32_t w,uint32_t h,uint32_t base,
							admGlyph *head,char *decodedstring);
ReplyType ocrBitmap(uint8_t *workArea,uint32_t w,uint32_t h,char *decodedString,admGlyph *head);
uint8_t   mergeBitmap(uint8_t *bitin, uint8_t *bitout, uint8_t *maskin,uint32_t w, uint32_t h);
void 	  ocrUpdateMinThreshold(void);

// In GUI dependant part
ReplyType glyphToText(admGlyph *glyph,admGlyph *head,char *decodedString);

/**
 * \class ADM_BitmapSource
 * \brief Front end base class for all OCR'able bitmap source
 */
class ADM_BitmapSource
{
				
public: 
								ADM_BitmapSource(void) {};
			virtual uint8_t     init(ADM_OCR_SOURCE *source)=0;
			virtual 			~ADM_BitmapSource() {};
			virtual uint32_t 	getNbImages(void)=0;
			virtual vobSubBitmap *getBitmap(uint32_t nb,uint32_t *start, uint32_t *end,uint32_t *first,uint32_t *last,
											uint32_t *eos)=0;
};
ADM_BitmapSource *ADM_buildBitmapSource(ADM_OCR_SOURCE *source);

#endif 
