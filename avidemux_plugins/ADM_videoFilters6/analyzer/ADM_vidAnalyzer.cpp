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
#define _USE_MATH_DEFINES // some compilers do not export M_PI etc.. if GNU_SOURCE or that is defined, let's do that
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "ADM_vidAnalyzer.h"


extern uint8_t DIA_getAnalyzer(ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoAnalyzer,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoAnalyzer,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_COLORS,            // Category
                                      "analyzer",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("analyzer","Analyzer"),            // Display name
                                      QT_TRANSLATE_NOOP("analyzer","Null filter. Vectorscope, Waveform scopes and Histograms in Preview.") // Description
                                  );

/**
    \fn configure
*/
bool ADMVideoAnalyzer::configure()
{
    return DIA_getAnalyzer(previousFilter);
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoAnalyzer::getConfiguration(void)
{
    return "Null filter";
}
/**
    \fn ctor
*/
ADMVideoAnalyzer::ADMVideoAnalyzer(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    UNUSED_ARG(couples);
}
/**
    \fn dtor
*/
ADMVideoAnalyzer::~ADMVideoAnalyzer()
{
}
/**
    \fn getCoupledConf
*/
bool ADMVideoAnalyzer::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}

void ADMVideoAnalyzer::setCoupledConf(CONFcouple *couples)
{
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoAnalyzer::getNextFrame(uint32_t *fn,ADMImage *image)
{
    return previousFilter->getNextFrame(fn,image);
}

