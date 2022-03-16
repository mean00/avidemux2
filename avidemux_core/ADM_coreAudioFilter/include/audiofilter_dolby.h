//
// C++ Interface: audiofilter_dolby
//
// Description: 
//
//
// Author: Mihail Zenkov <kreator@tut.by>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef AUDM_DOLBY_H
#define AUDM_DOLBY_H


#define NZEROS 500
#define GAIN 1.571116176e+00

/**
 * \class  ADMDolbyContext
 */
class ADMDolbyContext
{
public:    
        ADMDolbyContext();
        ~ADMDolbyContext();
        float *xv[8][4];
        int    pos[8];
        static bool  skip;
        
        float DolbyShift90(unsigned int sel, float isamp);
        void  DolbyInit();
        void  reset();
        
static   bool setValue(  float **target,int offset, float value);
static   bool setOneValue(float *target,int offset, float value);
        
static   void  DolbySkip(bool on);
        
static  float DolbyShift_simple(int pos, float *oldie, float *coef);        
static  float DolbyShift_convolution(int pos, float *oldie, float *coef);
static  float DolbyShift_convolutionAlign1(int pos, float *oldie, float *coef);
static  float DolbyShift_convolutionAlign2(float *oldie, float *coef);
static  float DolbyShift_convolutionAlign3(float *oldie, float *coef);
static  float DolbyShift_convolutionAlignSSE(float *oldie, float *coef);
};

#endif
