/** *************************************************************************
                    \fn       intrinsicTestVideoFilter.cpp  
                    \brief simplest of all video filters, it does nothing

    copyright            : (C) 2009 by mean

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"

#if defined( ADM_CPU_X86_64)
#include <immintrin.h>
#endif

/**
    \class intrinsicTestVideoFilter
*/
class intrinsicTestVideoFilter : public  ADM_coreVideoFilter
{
public:
                    intrinsicTestVideoFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~intrinsicTestVideoFilter();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *frameNumner,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   intrinsicTestVideoFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_MISC,            // Category
                        "intrinsicTest",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("intrinsicTest","Intrinsic Test"),            // Display name
                        QT_TRANSLATE_NOOP("intrinsicTest","Intrinsic Test.") // Description
                    );


intrinsicTestVideoFilter::intrinsicTestVideoFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);
}

intrinsicTestVideoFilter::~intrinsicTestVideoFilter()
{
}

bool intrinsicTestVideoFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if (!previousFilter->getNextFrame(fn,image))
        return false;
#if defined( ADM_CPU_X86_64)
    char text[256];
    __m128 a,b;
    float c = 13;
    float r[4];
    if(CpuCaps::hasSSE())
    {
        a = _mm_xor_ps(a,a);
        b = _mm_load_ps1(&c);
        b = _mm_shuffle_ps(b,b,0);
        a = _mm_add_ps(a,b);
        _mm_storeu_ps(r,a);
        sprintf(text, "SSE intrinsic test: %f %f %f %f\n",r[0],r[1],r[2],r[3]);
        image->printString(1,1,text);
    }
    if(CpuCaps::hasSSE2())
    {
        __m128i x,y;
        uint64_t z[2] = {15,3};
        x = _mm_loadu_si64(z);
        y = _mm_loadu_si64(z+1);
        x = _mm_sad_epu8(x,y);
        _mm_storeu_si64(z,x);
        sprintf(text, "SSE2 intrinsic test: %u, %u\n",(unsigned int)z[0],(unsigned int)z[1]);
        image->printString(1,3,text);
    }
#endif
    return true;
}

bool         intrinsicTestVideoFilter::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0);
    return true;
}

void intrinsicTestVideoFilter::setCoupledConf(CONFcouple *couples)
{
}

const char *intrinsicTestVideoFilter::getConfiguration(void)
{
    return "Intrinsic Test Filter.";
}
