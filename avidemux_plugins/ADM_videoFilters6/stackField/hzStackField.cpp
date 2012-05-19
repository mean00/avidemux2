/***************************************************************************
                          Separate Fields.cpp  -  description
                             -------------------
Convert a x*y * f fps video into -> x*(y/2)*fps/2 video

Same idea as for avisynth separatefield


    begin                : Thu Mar 21 2002
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
#include "ADM_coreVideoFilterInternal.h"

/**
    \class AVDMVideoHzStackField
*/
class AVDMVideoHzStackField : public  ADM_coreVideoFilter
{
protected:
                    ADMImage *current;
public:
                
                    AVDMVideoHzStackField(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~AVDMVideoHzStackField();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   AVDMVideoHzStackField,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "hzstackfield",            // internal name (must be uniq!)
                        "Horizontal Stack Fields",            // Display name
                        "Put fields side by side." // Description
                    );

// Now implements the interesting parts
/**
    \fn AVDMVideoHzStackField
    \brief constructor
*/
AVDMVideoHzStackField::AVDMVideoHzStackField(  ADM_coreVideoFilter *in,CONFcouple *setup) 
: ADM_coreVideoFilter(in,setup)
{
    current=new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
    info.width<<=1;
    info.height>>=1;
}
/**
    \fn AVDMVideoHzStackField
    \brief destructor
*/
AVDMVideoHzStackField::~AVDMVideoHzStackField()
{
    if(current) delete current;
    current=NULL;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         AVDMVideoHzStackField::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *AVDMVideoHzStackField::getConfiguration(void)
{
    
    return "Horizontal Stack Field.";
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool AVDMVideoHzStackField::getNextFrame(uint32_t *fn,ADMImage *image)
{
  // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,current))
    {
        ADM_warning("HzStackField : Cannot get frame\n");
        return false;
    }		
		//
        //  AAAA
        //  BBBB      AAAABBBB
        //  CCCC      CCCCDDDD
        //  DDDD
        
        for(int i=0;i<3;i++)
        {
            ADM_PLANE plane=(ADM_PLANE)i;
            int w=image->GetWidth(plane);
            int h=image->GetHeight(plane);
            uint8_t *src1=current->GetReadPtr(plane);
            uint8_t *dst1=image->GetWritePtr(plane);        
            int pitchSrc=current->GetPitch(plane);
            int pitchDst=image->GetPitch(plane);

            BitBlit(dst1, pitchDst,src1,pitchSrc*2,w/2,h);
            BitBlit(dst1+w/2, pitchDst,src1+pitchSrc,pitchSrc*2,w/2,h);
        }
        image->copyInfo(current);

        return 1;
}
// EOF


