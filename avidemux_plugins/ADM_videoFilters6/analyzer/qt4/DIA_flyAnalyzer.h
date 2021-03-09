/***************************************************************************
                          Analyzer filter 
        Copyright 2021 szlldm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_Analyzer_H
#define FLY_Analyzer_H
#include <QGraphicsScene>
#include <QImage>
#include "ADM_default.h"
#include "ADM_byteBuffer.h"
#include "ADM_image.h"
/**
    \class flyAnalyzer
*/
class flyAnalyzer : public ADM_flyDialogYuv
{
  public:
    QGraphicsScene * sceneVectorScope;
    QGraphicsScene * sceneYUVparade;
    QGraphicsScene * sceneRGBparade;
    QGraphicsScene * sceneHistograms;

    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void) { return 1; }
    uint8_t    upload(void) { return 1; }
    uint8_t    update(void);
    void       setTabOrder(void);
               flyAnalyzer (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider, QGraphicsScene * scVectorScope,
                                    QGraphicsScene * scYUVparade, QGraphicsScene * scRGBparade, QGraphicsScene * scHistograms) ;
    virtual    ~flyAnalyzer() ;
  private:
    int                  rgbBufStride;
    ADM_byteBuffer *     rgbBufRaw;
    ADMColorScalerFull * convertYuvToRgb;
    int                  rgbWidth, rgbHeight;
    uint32_t * bufVectorScope;
    QImage   * imgVectorScope;
    uint32_t * bufYUVparade;
    QImage   * imgYUVparade;
    uint32_t * bufRGBparade;
    QImage   * imgRGBparade;
    uint32_t * bufHistograms;
    QImage   * imgHistograms;
};
#endif
