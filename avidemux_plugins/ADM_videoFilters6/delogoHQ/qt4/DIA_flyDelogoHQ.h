/***************************************************************************
  \fn     DIA_flyDelogoHQ
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for delogoHQ
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_DelogoHQ_H
#define FLY_DelogoHQ_H
#include "delogoHQ.h"
/**
    \class flyDelogoHQ
*/
class flyDelogoHQ : public ADM_flyDialogYuv
{
  public:
    delogoHQ         param;
  private:
    int                    rgbBufStride;
    ADM_byteBuffer *       rgbBufRaw;
    ADMImageRef *          rgbBufImage;
    ADMColorScalerFull *   convertYuvToRgb;
    ADMColorScalerFull *   convertRgbToYuv;
    int *                  mask;
    int                    maskHint[4];
    char * saveFilename;

    void       createBuffers(void);
    void       destroyBuffers(void);
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
    flyDelogoHQ (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO) {
                          saveFilename = NULL;
                          mask=NULL;
                          maskHint[0] = maskHint[1] = maskHint[2] = maskHint[3] = -1;
                          createBuffers();
                };
    virtual ~flyDelogoHQ() {
                          if (mask) free(mask);
                          destroyBuffers();
                };
    void       saveCurrentFrame(char * filename) {saveFilename = filename;};
    bool       setMask(ADMImage * newMask);

};
#endif
