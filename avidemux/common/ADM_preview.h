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

/**
    class admPreview
*/
class admPreview
{
  protected:
      static ADMImage *rdrImage;
  public:
      static uint8_t nextPicture(void);
      static uint8_t previousPicture(void);
      static uint8_t samePicture(void);
      static bool seekToTime(uint64_t timeFrame);
      static bool seekToIntraPts(uint64_t timeframe);
      static void start(void);
      static void stop(void);
      static void setMainDimension(uint32_t, uint32_t, float);
      static void deferDisplay(bool onoff);
      static void displayNow(void);
      static void cleanUp(void);
      static ADMImage *getBuffer(void);
      static uint64_t getCurrentPts(void);
      static uint64_t getRefPts(void);
      static uint32_t getRefVideo(void);
      static bool nextKeyFrame(void);
      static bool previousKeyFrame(void);
      static bool previousFrame(void);
      static void destroy(void);
      static bool updateImage(void);
      static ADM_HW_IMAGE getPreferedHwImageFormat(void);
      static float getCurrentZoom();
      static void changePreviewZoom(float nzoom);
      static ADM_PREVIEW_MODE getPreviewMode(void);
      static void setPreviewMode(ADM_PREVIEW_MODE preview);
	  static void getFrameFlags(uint32_t *flags, uint32_t *quantiser);
};
#endif
