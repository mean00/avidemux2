/***************************************************************************
                          ADM_vidAddBorder.cpp  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
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
#include "DIA_coreToolkit.h"
#include "ADM_vidBlackBorder.h"
#include "DIA_factory.h"
#include "blackenBorder_desc.cpp"

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *blackenBorders::getConfiguration(void)
{
    static char conf[100];
    conf[0]=0;
    snprintf(conf,100,"blacken Borders : Left:%" PRIu32" Right:%" PRIu32" Top:%" PRIu32" Bottom:%" PRIu32"\n",
                param.left,param.right,param.top,param.bottom);
    return conf;
}
/**
    \fn ctor
*/
blackenBorders::blackenBorders( ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
	 if(!setup || !ADM_paramLoad(setup,blackenBorder_param,&param))
    {
        // Default value
        param.left=0;
        param.right=0;
        param.top=0;
        param.bottom=0;
    }
}
/**
    \fn dtor
*/
blackenBorders::~blackenBorders()
{

}

/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         blackenBorders::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, blackenBorder_param,&param);
}

void blackenBorders::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, blackenBorder_param, &param);
}

#define Y_BLACK 16
#define UV_BLACK 128
static bool blackenHz(int w,int nbLine,uint8_t *ptr[3],int strides[3])
{
    // y
    uint8_t *p=ptr[0];
    uint32_t s=strides[0];
    for(int y=0;y<nbLine;y++)
    {
        memset(p,Y_BLACK,w);
        p+=s;
    }
    p=ptr[1];
    s=strides[1];
    nbLine/=2;
    w/=2;
    for(int y=0;y<nbLine;y++)
    {
        memset(p,UV_BLACK,w);
        p+=s;
    }
    p=ptr[2];
    s=strides[2];
    for(int y=0;y<nbLine;y++)
    {
        memset(p,UV_BLACK,w);
        p+=s;
    }
    return true;
}

/**
    \fn getNextFrame
*/
bool blackenBorders::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,image))
    {
        ADM_info("[blackenBorder] Cannot get previous image\n");
        return false;
    }


    // Top...
    uint8_t *ptr[3];
    int      stride[3];
    image->GetPitches(stride);
    image->GetWritePlanes(ptr);
    // top
    blackenHz(image->_width,param.top,ptr,stride);
    // Left
    blackenHz(param.left,image->_height,ptr,stride);
    // Right
    uint32_t pWidth=previousFilter->getInfo()->width-param.right;
    ptr[0]+=pWidth;
    ptr[1]+=(pWidth)/2;
    ptr[2]+=(pWidth)/2;
    blackenHz(param.right,image->_height,ptr,stride);
    // Bottom
    image->GetPitches(stride);
    image->GetWritePlanes(ptr);
    uint32_t offsetLine=previousFilter->getInfo()->height-param.bottom;
    ptr[0]+=offsetLine*stride[0];
    ptr[1]+=(offsetLine/2)*stride[1];
    ptr[2]+=(offsetLine/2)*stride[2];
    blackenHz(image->_width,param.bottom,ptr,stride);
    return true;
}

/**
    \fn configure
*/
bool blackenBorders::configure(void)
{
        uint32_t width,height;
#define MAKEME(x) uint32_t x=param.x;
        while(1)
        {
          MAKEME(left);
          MAKEME(right);
          MAKEME(top);
          MAKEME(bottom);

          width=previousFilter->getInfo()->width;
          height=previousFilter->getInfo()->height;

          diaElemUInteger dleft(&left,QT_TRANSLATE_NOOP("blacken","_Left border:"),       0,width/2);
          diaElemUInteger dright(&right,QT_TRANSLATE_NOOP("blacken","_Right border:"),    0,width/2);
          diaElemUInteger dtop(&(top),QT_TRANSLATE_NOOP("blacken","_Top border:"),          0,height/2);
          diaElemUInteger dbottom(&(bottom),QT_TRANSLATE_NOOP("blacken","_Bottom border:"), 0,height/2);

          diaElem *elems[4]={&dleft,&dright,&dtop,&dbottom};
          if(diaFactoryRun(QT_TRANSLATE_NOOP("blacken","Blacken Borders"),4,elems))
          {
            if((left&1) || (right&1)|| (top&1) || (bottom&1))
            {
              GUI_Error_HIG(QT_TRANSLATE_NOOP("blacken","Incorrect parameters"),QT_TRANSLATE_NOOP("blacken","All parameters must be even and within range."));
              continue;
            }
            else
            {
  #undef MAKEME
  #define MAKEME(x) param.x=x;
                MAKEME(left);
                MAKEME(right);
                MAKEME(top);
                MAKEME(bottom);
                return true;
            }
          }
          return false;
      }
}




