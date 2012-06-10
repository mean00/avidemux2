/** *************************************************************************
                    \fn       rotateFilter.cpp  
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
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "rte.h"
#include "rte_desc.cpp"

static void do_rotate(ADMImage *source,ADMImage *target,uint32_t angle);
/**
    \class rotateFilter
*/
class rotateFilter : public  ADM_coreVideoFilter
{
protected:
        rte                  param;
        bool                 reset(void);
        ADMImage             *src;
public:
                    rotateFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~rotateFilter();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;     /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   rotateFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "rotate",            // internal name (must be uniq!)
                        "Rotate",            // Display name
                        "Rotate the image by 90/180/270 degrees." // Description
                    );

/**
    \fn reset
*/
bool rotateFilter::reset(void)
{
    uint32_t w=previousFilter->getInfo()->width;
    uint32_t h=previousFilter->getInfo()->height;
    switch(param.angle)
    {
            case 0:case 180: info.width=w;info.height=h;break;
            case 90:case 270: info.width=h;info.height=w;break;
            default: ADM_assert(0);
    }
    
    return true;

}
/**
    \fn rotateFilter
    \brief constructor
*/
rotateFilter::rotateFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
    src=NULL;
    if(!setup || !ADM_paramLoad(setup,rte_param,&param))
    {
        // Default value
        param.angle=0;// Bff=0 / 1=tff
    }  	  	
    src=new ADMImageDefault(previousFilter->getInfo()->width,previousFilter->getInfo()->height);		
    reset();
}
/**
    \fn rotateFilter
    \brief destructor
*/
rotateFilter::~rotateFilter()
{
      if(src) delete src;
      src=NULL;
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool rotateFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,src))
    {
        ADM_warning("rotate : Cannot get frame\n");
        return false;
    }
    do_rotate(src,image,param.angle);
    image->copyInfo(src);
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         rotateFilter::getCoupledConf(CONFcouple **couples)
{
   return ADM_paramSave(couples, rte_param,&param);
}

void rotateFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, rte_param, &param);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *rotateFilter::getConfiguration(void)
{
    static char buffer[80];
    snprintf(buffer,80,"Rotate by %"LU" degrees.",param.angle);
    return buffer;
}
/**
    \fn rotatePlane
*/
void rotatePlane(uint32_t angle,uint8_t *src,      uint32_t srcPitch, 
                 uint8_t *dst,  uint32_t dstPitch, uint32_t width,    uint32_t height)
{
    int32_t dstIncPix, dstIncLine;
    
    switch(angle)
    {
        case 0:   BitBlit(dst,dstPitch,src,srcPitch,width,height);return;break;
        case 180: dstIncPix=-1;       dstIncLine=-dstPitch; dst=dst+((height-1)*dstPitch)+width-1;break;

        case 90:  dstIncPix=-dstPitch;dstIncLine=1;         dst=dst+((width-1)*dstPitch);break;
        case 270: dstIncPix=dstPitch; dstIncLine=-1;        dst=dst+height-1;break;
        default:
            break;
    }
    
    uint8_t *lineIn,*lineOut;
    for(int y=0;y<height;y++)
    {
        lineIn=src+srcPitch*y;
        lineOut=dst+dstIncLine*y;
        for(int x=0;x<width;x++)
        {
            *lineOut=*lineIn;
            lineIn++;
            lineOut+=dstIncPix;
        }
    }


}

/**
    \fn do_rotate
*/
void do_rotate(ADMImage *source,ADMImage *target,uint32_t angle)
{
uint8_t *in,*out;
uint32_t width,height;
uint32_t srcPitch,dstPitch;

    for(int i=0;i<3;i++)
    {
         ADM_PLANE plane=(ADM_PLANE)i;
         width=source->_width;
         height=source->_height;
         if(i)
            {
                width>>=1;
                height>>=1;
            }
          in=source->GetReadPtr(plane);
          srcPitch=source->GetPitch(plane);
          dstPitch=target->GetPitch(plane);
          out=target->GetWritePtr(plane);
          rotatePlane(angle,in,  srcPitch, out,  dstPitch,  width,  height);
    }
}

/**
    \fn configure
*/
bool rotateFilter::configure( void)
{
  uint8_t r;
  
  diaMenuEntry rotateValues[]={
      {0,QT_TR_NOOP("None"),QT_TR_NOOP("None")},
      {90,QT_TR_NOOP("90 degrees"),QT_TR_NOOP("90°")},
      {180,QT_TR_NOOP("180 degrees"),QT_TR_NOOP("180°")},
      {270,QT_TR_NOOP("270 degrees"),QT_TR_NOOP("270°")}
  };
  diaElemMenu     rotate(&(param.angle),QT_TR_NOOP("_Angle:"),4,rotateValues,NULL);
  diaElem *allWidgets[]={&rotate};
  if( !diaFactoryRun(QT_TR_NOOP("Rotate"),1,allWidgets)) return false;
  reset();
  return true;
}
//EOF
