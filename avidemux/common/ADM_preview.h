/**
        \file ADM_preview.h
        \brief Handles preview in main program

*/
#ifndef ADM_PREVIEW_H
#define ADM_PREVIEW_H
#include "ADM_render/GUI_render.h"
typedef enum 
{
    ADM_PREVIEW_NONE, 
    ADM_PREVIEW_OUTPUT,
    ADM_PREVIEW_SIDE,
    ADM_PREVIEW_TOP,
    ADM_PREVIEW_SEPARATE
}ADM_PREVIEW_MODE;

ADM_PREVIEW_MODE  getPreviewMode(void);
void             setPreviewMode(ADM_PREVIEW_MODE preview);
void             changePreviewZoom(renderZoom nzoom);

class admPreview
{
  public:
      static uint8_t nextPicture(void);
      static uint8_t previousPicture(void);
      static uint8_t samePicture(void);
//      static uint8_t seekToIntra(uint32_t framenum);
      static bool seekToIntraPts(uint64_t timeframe);
      static void start(void);
      static void stop(void);
      static void setMainDimension(uint32_t, uint32_t );
      static void deferDisplay(uint32_t onoff,uint32_t startat);
      static void displayNow(void);
      static void cleanUp(void);  
      static ADMImage *getBuffer(void);
      static uint64_t getCurrentPts(void);
      static bool nextKeyFrame(void);
      static bool previousKeyFrame(void);
      static bool previousFrame(void);
      static void destroy(void);
};
#endif
