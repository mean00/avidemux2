/***************************************************************************
                          ColorBalance filter 
    Algorithm:
        Copyright 2009 Maksim Golovkin (m4ks1k@gmail.com)
        Copyright 2021 szlldm
    Ported to Avidemux:
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
#pragma once

/**
    \class ADMVideoColorBalance
*/
class  ADMVideoColorBalance:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    colorBalance     _param;
    colorBalance     _internalparam;
  public:
    ADMVideoColorBalance(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoColorBalance();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static  void         ColorBalanceProcess_C(ADMImage *img, colorBalance cfg);
    static  void         ColorBalanceRanges_C(ADMImage *img);
    static  void         reset(colorBalance *cfg);

  private:
    static float         valueLimit(float val, float min, float max);
    static int32_t       valueLimit(int32_t val, int32_t min, int32_t max);
    static void          gaussSLESolve(size_t size, double* A, double *solution);
    static void          calcParabolaCoeffs(double* points, double *coeffs);
    static float         parabola(double x, double* coeffs);
    static void          quadraticCurve(int * map, float loPoint, float mdPoint, float hiPoint, float curveMin, float curveMax, bool limitedRange, float frMul, float lrMul, float lrOffset);
};


