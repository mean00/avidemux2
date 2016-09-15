/***************************************************************************

		Put a logon on video

    copyright            : (C) 2007 by mean
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
#include "ADM_imageResizer.h"
#include "ADM_coreVideoFilter.h"
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "ascii_font.h"

#define REDUCE_WIDTH   12
#define REDUCE_HEIGHT  20

/**
    \class AsciiFilter
*/
class AsciiFilter : public  ADM_coreVideoFilter
{
protected:
                ADMImage    *tmpImage;
                bool        init(void);               
                bool        drawGlyphs(ADMImage *source,ADMImage *target);
                bool        drawOne(uint8_t value, ADMImage *target, int x, int y,int luma);
                uint8_t     findBestMatch(ADMImage *source,int col,int raw,int &luma);
                int         reducedWidth,reducedHeight;
public:
                    AsciiFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~AsciiFilter();
                    

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;     /// Return the current filter configuration
		virtual void         setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void);                           /// Start graphical user interface
        
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   AsciiFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "AsciiView",            // internal name (must be uniq!)
                        "Ascii View",            // Display name
                        "Ascii view" // Description
                    );

// Now implements the interesting parts
/**
    \fn AsciiFilter
    \brief constructor
*/
AsciiFilter::AsciiFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
  tmpImage=NULL;
  init();
}
/**

*/
bool AsciiFilter::init(void)
{       
    if(tmpImage) delete tmpImage;
    tmpImage=NULL;
  
    tmpImage=new ADMImageDefault(this->getInfo ()->width,this->getInfo ()->height);
    
    reducedWidth=this->getInfo ()->width / REDUCE_WIDTH;
    reducedHeight=this->getInfo ()->height / REDUCE_HEIGHT;
    return true;
}
/**
    \fn AsciiFilter
    \brief destructor
*/
AsciiFilter::~AsciiFilter()
{
    if(tmpImage) delete  tmpImage;
    tmpImage=NULL;
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool AsciiFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,tmpImage))
    {
        ADM_warning("asciiView : Cannot get frame\n");
        return false;
    }   
    image->blacken ();
    drawGlyphs(tmpImage,image);
    return true;
}
/**
 * 
 * @param value
 * @param target
 * @param x
 * @param y
 * @return 
 */
/**
 * Texture is a 8bits 128*64, each glyph seems to be 8*16
 */
bool AsciiFilter::drawOne(uint8_t  value, ADMImage *target, int x, int y,int luma)
{
    int stride=target->GetPitch (PLANAR_Y);
    uint8_t *src=target->GetReadPtr(PLANAR_Y)+x*REDUCE_WIDTH+y*REDUCE_HEIGHT*stride;
    uint16_t *fnt=font[value];
    for(int y=0;y<REDUCE_HEIGHT;y++)
    {
      uint32_t bit= *fnt;fnt++;
      for(int x=0;x<REDUCE_WIDTH;x++)
      {
          if(bit&0x8000) src[x]=luma;
          else src[x]=0;
          bit<<=1;
      }
      src+=stride;      
    }
    return true;
}
/**
 * 
 * @param car
 * @param src
 * @param stride
 * @return 
 */
static int computeDelta(int car, uint16_t *bitmask)
{
  int sum=0;  
  uint16_t *fnt=font[car-' '];
  for(int y=0;y<REDUCE_HEIGHT;y++)
  {
      
      uint32_t fromFont=fnt[y]>>4;
      uint32_t fromBitmask=bitmask[y];
      sum+=__builtin_popcount(fromFont^fromBitmask);    
  }
  return sum;  
}
static void createBitMask(uint16_t *out, uint8_t *src, int stride, int &luma)
{
  int nbOn=0;
  luma=0;
  int errorDiffusion=0;

  for(int y=0;y<REDUCE_HEIGHT;y++)
    {
      uint32_t bit= 0;
      for(int x=0;x<REDUCE_WIDTH;x++)
      {
          bit=bit<<1;
          int pix=((int)src[x]);
          if(pix+errorDiffusion>128)
          {
              bit+=1;
              nbOn++;
              luma+=src[x];
              errorDiffusion-=255-src[x];
          }
          else
          {
              bit+=0;
              errorDiffusion+=src[x];
          }
      }
      *out=bit&0x7fe,
      out++;
      src+=stride;     
    }
  if(nbOn)
    luma=luma/nbOn; // average
  else
    luma=0;
}
/**
 * 
 * @param col
 * @param raw
 * @return 
 */
uint8_t AsciiFilter::findBestMatch(ADMImage *source,int col,int row,int &luma)
{
  int minDelta=0xfffffff;
  int candidate=-1;
  int stride=source->GetPitch (PLANAR_Y);
  uint8_t *p=source->GetReadPtr(PLANAR_Y)+col*REDUCE_WIDTH+row*REDUCE_HEIGHT*stride;
  // 1- create bitmask
  uint16_t bitMask[REDUCE_WIDTH*REDUCE_HEIGHT];
  createBitMask(bitMask,p,stride,luma);
  
  // 32..127
  for(int tries=32;tries<128;tries++)
    {
      int delta=computeDelta(tries,bitMask);
      if(delta<minDelta)
        {
          minDelta=delta;
          candidate=tries;
        }
    }
  if(candidate==-1) 
    {
      luma=128;
      return '*';
    }
  else 
    return candidate;
}

/**
 * 
 * @param target
 * @return 
 */
bool AsciiFilter::drawGlyphs(ADMImage *source,ADMImage *target)
{
  int luma;
  uint8_t glyph;
  target->blacken ();
  for(int y=0;y<reducedHeight;y++)
    {      
      for(int x=0;x<reducedWidth;x++)
      {
        glyph=findBestMatch(source,x,y,luma);
        drawOne(glyph,target,x,y,luma);
      }
    }
   return true;
}

/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         AsciiFilter::getCoupledConf(CONFcouple **couples)
{
    *couples=NULL;
    return true;
}

void AsciiFilter::setCoupledConf(CONFcouple *couples)
{
   
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *AsciiFilter::getConfiguration(void)
{
    return "Ascii view.";
}

/**
    \fn configure
*/
bool AsciiFilter::configure( void)
{
    return true;
}


/************************************************/
//EOF
