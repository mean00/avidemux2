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
#include "ADM_coreVideoFilter.h"
#include "ADM_vidMisc.h"

/**
    \class AVDMVideoSeparateField
*/
class AVDMVideoSeparateField : public  ADM_coreVideoFilterCached
{
protected:
        
public:
                
                    AVDMVideoSeparateField(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~AVDMVideoSeparateField();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   AVDMVideoSeparateField,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "SeparateFields",            // internal name (must be uniq!)
                        "Separate Fields",            // Display name
                        "Split each image into 2 fields." // Description
                    );
/**
    \fn getCoupledConf
*/
bool         AVDMVideoSeparateField::getCoupledConf(CONFcouple **couples) 
{
     *couples=new CONFcouple(0);
    return true;
}

void AVDMVideoSeparateField::setCoupledConf(CONFcouple *couples)
{
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *AVDMVideoSeparateField::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"Separate Fields");
    return conf;
}
/**
    \fn AVDMVideoSeparateField
    \brief constructor
*/
AVDMVideoSeparateField::AVDMVideoSeparateField(  ADM_coreVideoFilter *in,CONFcouple *setup) 
        : ADM_coreVideoFilterCached(4,in,setup)
{
    info.height>>=1;
    info.frameIncrement/=2;    
}
/**
    \fn AVDMVideoSeparateField
    \brief dtor
*/
AVDMVideoSeparateField::~AVDMVideoSeparateField()
{

}


/**
    \fn getFrame
    \brief Interleave frame*2 and frame*2+1

    AAA
    BBB        AAA   BBB
    CCC  =>    CCC + DDD
    DDD

*/
bool AVDMVideoSeparateField::getNextFrame(uint32_t *fn,ADMImage *image)
{
   
ADMImage *cur,*next;
uint32_t frame=nextFrame++; 
        *fn=frame;
        cur=vidCache->getImage(frame/2);
        if(!cur)
        {
                ADM_warning("Seoarate field : cannot read\n");
                vidCache->unlockAll();
                return 0;
        }

        for(int i=0;i<3;i++)
        {
            ADM_PLANE plane=(ADM_PLANE)i;

            int w=cur->GetWidth(plane);
            int h=cur->GetHeight(plane);

            uint8_t *src1=cur->GetReadPtr(plane);
            uint8_t *dst1=image->GetWritePtr(plane);        
            int pitchSrc=cur->GetPitch(plane);
            int pitchDst=image->GetPitch(plane);
            int offset=0;
            if(frame&1) // bottom
                offset=pitchSrc;
            BitBlit(dst1, pitchDst,src1+offset,pitchSrc*2,w,h/2);
        }
        image->copyInfo(cur);
        if(frame&1)
            image->Pts+=info.frameIncrement;
        vidCache->unlockAll();
        //ADM_info("Output PTS=%s\n",ADM_us2plain(image->Pts));
        return 1;
}
// EOF
