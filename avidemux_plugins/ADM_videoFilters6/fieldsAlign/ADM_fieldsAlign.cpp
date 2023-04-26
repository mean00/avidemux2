/** *************************************************************************
                    \file     fieldsAlign.cpp  
                    \brief    align fields
    copyright            : (C) 2022 by szlldm

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
#include "DIA_factory.h"
#include "fieldsAlign.h"
#include "fieldsAlign_desc.cpp"
#include "ADM_byteBuffer.h"
/**
    \class fieldsAlignFilter
*/
class fieldsAlignFilter : public  ADM_coreVideoFilter
{
private:
                uint8_t *   lineBuf;
                void        shiftLine(uint8_t * line, int width, int amount);
                ADM_byteBuffer  data444;
                uint8_t *   yuv444[3];
                int         yuv444stride[3];
                ADMColorScalerSimple * forwScale;
                ADMColorScalerSimple * backScale;
public:
                    fieldsAlignFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~fieldsAlignFilter();
                fieldsAlign  config;
        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   fieldsAlignFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,     // Category
                        "fieldsAlign",         // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("fieldsAlign","Fields Align"),            // Display name
                        QT_TRANSLATE_NOOP("fieldsAlign","Adjust fields alignment.") // Description
                    );

// Now implements the interesting parts
/**
    \fn fieldsAlignFilter
    \brief constructor
*/
fieldsAlignFilter::fieldsAlignFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
    if(!setup || !ADM_paramLoad(setup,fieldsAlign_param,&config))
    {
        // Default value
        config.swapFields=false;
        config.topAdjH=0;
        config.topAdjV=0;
        config.botAdjH=0;
        config.botAdjV=0;
    }
   
    lineBuf = new uint8_t [previousFilter->getInfo()->width];
    data444.setSize(ADM_IMAGE_ALIGN(previousFilter->getInfo()->width) * previousFilter->getInfo()->height * 3);
    yuv444[0] = data444.at(0);
    yuv444[1] = data444.at(ADM_IMAGE_ALIGN(previousFilter->getInfo()->width) * previousFilter->getInfo()->height);
    yuv444[2] = data444.at(ADM_IMAGE_ALIGN(previousFilter->getInfo()->width) * previousFilter->getInfo()->height * 2);
    yuv444stride[0] = ADM_IMAGE_ALIGN(previousFilter->getInfo()->width);
    yuv444stride[2] = yuv444stride[1] = yuv444stride[0];
    forwScale = new ADMColorScalerSimple(previousFilter->getInfo()->width, previousFilter->getInfo()->height, ADM_PIXFRMT_YV12, ADM_PIXFRMT_YUV444);
    backScale = new ADMColorScalerSimple(previousFilter->getInfo()->width, previousFilter->getInfo()->height, ADM_PIXFRMT_YUV444, ADM_PIXFRMT_YV12);
}
/**
    \fn fieldsAlignFilter
    \brief destructor
*/
fieldsAlignFilter::~fieldsAlignFilter()
{
    delete [] lineBuf;
    delete forwScale;
    delete backScale;
    data444.clean();
}


/**
    \fn shiftLine
*/
void fieldsAlignFilter::shiftLine(uint8_t * line, int width, int amount)
{
    if (amount <= (-1*width))
    {
        memset(line, line[width-1], width);
    }
    else
    if (amount < 0)
    {
        memmove(line, line-amount, width+amount);
        memset(line+width+amount, line[width-1], -1*amount);
    }
    else
    if (amount >= width)
    {
        memset(line, line[0], width);
    }
    else
    {
        memmove(line+amount, line, width-amount);
        memset(line, line[0], amount);
    }
}
/**
    \fn getFrame
    \brief Get a processed frame
*/
bool fieldsAlignFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        ADM_warning("Fields Align: Cannot get frame\n");
        return false;
    }

    int h = image->GetHeight(PLANAR_Y);
    int w = image->GetWidth(PLANAR_Y);
    int stride = image->GetPitch(PLANAR_Y);
    uint8_t * ptr = image->GetWritePtr(PLANAR_Y);
    if(config.swapFields)
    {
        for (int y=0; (y+1)<h; y+=2)
        {
            memcpy(lineBuf, ptr+y*stride, w);
            memcpy(ptr+y*stride, ptr+(y+1)*stride, w);
            memcpy(ptr+(y+1)*stride, lineBuf, w);
        }
    }
    if (config.topAdjH || config.botAdjH || config.topAdjV || config.botAdjV)
    {
        uint8_t * yuv420[3];
        int yuv420stride[3];
        image->GetWritePlanes(yuv420);
        image->GetPitches(yuv420stride);
        forwScale->convertPlanes(yuv420stride,yuv444stride,yuv420,yuv444);
        for (int p=0; p<3; p++)
        {
            stride = yuv444stride[p];
            ptr = yuv444[p];
            
            if(config.topAdjH)
            {
                for (int y=0; y<h; y+=2)
                {
                    shiftLine(ptr+y*stride, w, config.topAdjH);
                }
            }
            if(config.botAdjH)
            {
                for (int y=1; y<h; y+=2)
                {
                    shiftLine(ptr+y*stride, w, config.botAdjH);
                }
            }
            if(config.topAdjV)
            {
                if (config.topAdjV < 0)
                {
                    for (int y=0; y<h; y++)
                    {
                        if (y%2 == 0)
                        {
                            int yn = y-config.topAdjV*2;
                            while (yn >= h) yn-=2;
                            memcpy(ptr+y*stride, ptr+yn*stride, w);
                        }
                    }                    
                }
                else
                {
                    for (int y=h-1; y>=0; y--)
                    {
                        if (y%2 == 0)
                        {
                            int yn = y-config.topAdjV*2;
                            while (yn < 0) yn+=2;
                            memcpy(ptr+y*stride, ptr+yn*stride, w);
                        }
                    }                    
                }
            }
            if(config.botAdjV)
            {
                if (config.botAdjV < 0)
                {
                    for (int y=0; y<h; y++)
                    {
                        if (y%2 == 1)
                        {
                            int yn = y-config.botAdjV*2;
                            while (yn >= h) yn-=2;
                            memcpy(ptr+y*stride, ptr+yn*stride, w);
                        }
                    }                    
                }
                else
                {
                    for (int y=h-1; y>=0; y--)
                    {
                        if (y%2 == 1)
                        {
                            int yn = y-config.botAdjV*2;
                            while (yn < 0) yn+=2;
                            memcpy(ptr+y*stride, ptr+yn*stride, w);
                        }
                    }                    
                }
            }
        }
        backScale->convertPlanes(yuv444stride,yuv420stride,yuv444,yuv420);
    }

    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         fieldsAlignFilter::getCoupledConf(CONFcouple **couples)
{
      return ADM_paramSave(couples, fieldsAlign_param,&config);
}

void fieldsAlignFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, fieldsAlign_param, &config);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *fieldsAlignFilter::getConfiguration(void)
{
    static char cfg[256];
    static const char *yesno[2]={"N","Y"};
    snprintf(cfg,255,"Swap fields: %s  Horizontal adjust: [%d , %d]  Vertical adjust: [%d , %d].",
                    yesno[config.swapFields?1:0],config.topAdjH,config.botAdjH,config.topAdjV,config.botAdjV);
    return cfg;
}

/**
    \fn configure
*/
bool fieldsAlignFilter::configure(void)
{
  
  diaElemToggle swapFields(&(config.swapFields),QT_TRANSLATE_NOOP("fieldsAlign","Swap fields"), NULL);
  diaElemInteger topAdjH(&(config.topAdjH), QT_TRANSLATE_NOOP("fieldsAlign","Top field horizontal adjust"), -16, 16, NULL);
  diaElemInteger botAdjH(&(config.botAdjH), QT_TRANSLATE_NOOP("fieldsAlign","Bottom field horizontal adjust"), -16, 16, NULL);
  diaElemInteger topAdjV(&(config.topAdjV), QT_TRANSLATE_NOOP("fieldsAlign","Top field vertical adjust"), -16, 16, NULL);
  diaElemInteger botAdjV(&(config.botAdjV), QT_TRANSLATE_NOOP("fieldsAlign","Bottom field vertical adjust"), -16, 16, NULL);
  
  
  diaElem *elems[5]={&swapFields,&topAdjH,&botAdjH,&topAdjV,&botAdjV};
  
  return diaFactoryRun(QT_TRANSLATE_NOOP("fieldsAlign","Fields Align"),5,elems);
}


//EOF
