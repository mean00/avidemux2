/***************************************************************************
                          AVDMVideoMergeField  -  description
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
    \class AVDMVideoMergeField
*/
class AVDMVideoMergeField : public  ADM_coreVideoFilterCached
{
protected:
        
public:
                
                    AVDMVideoMergeField(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~AVDMVideoMergeField();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   AVDMVideoMergeField,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "mergefields",            // internal name (must be uniq!)
                        "Merge Fields",            // Display name
                        "Merge two pictures as if they were two fields." // Description
                    );
/**
    \fn getCoupledConf
*/
bool         AVDMVideoMergeField::getCoupledConf(CONFcouple **couples) 
{
    *couples=new CONFcouple(0);
    return true;
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *AVDMVideoMergeField::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"Merge Fields");
    return conf;
}
/**
    \fn AVDMVideoMergeField
    \brief constructor
*/
AVDMVideoMergeField::AVDMVideoMergeField(  ADM_coreVideoFilter *in,CONFcouple *setup) 
        : ADM_coreVideoFilterCached(4,in,setup)
{

	info.height<<=1;
    info.frameIncrement*=2;
    
}
/**
    \fn AVDMVideoMergeField
    \brief dtor
*/
AVDMVideoMergeField::~AVDMVideoMergeField()
{

}


/**
    \fn getFrame
    \brief Interleave frame*2 and frame*2+1

  AAA   CCC        AAAA
  BBB + DDD  =>    CCCC
                   BBBB
                   DDDD

*/
bool AVDMVideoMergeField::getNextFrame(uint32_t *fn,ADMImage *image)
{
   
ADMImage *cur,*next;
uint32_t frame=nextFrame++;        
		cur=vidCache->getImage(2*frame);
		next=vidCache->getImage(2*frame+1);
		
		if(!cur )
		{
			ADM_warning("Merge field : cannot read\n");
			vidCache->unlockAll();
		 	return 0;
		}
        if(!next)
        {
            image->duplicateFull(cur);
            vidCache->unlockAll();
            return true;
        }
        
        for(int i=0;i<3;i++)
        {
            ADM_PLANE plane=(ADM_PLANE)i;

            int w=cur->GetWidth(plane);
            int h=cur->GetHeight(plane);
            uint8_t *src1=cur->GetReadPtr(plane);
            uint8_t *src2=next->GetReadPtr(plane);
            uint8_t *dst1=image->GetWritePtr(plane);        
            int pitchSrc=cur->GetPitch(plane);
            int pitchDst=image->GetPitch(plane);

            BitBlit(dst1, pitchDst*2,src1,pitchSrc,w,h);
            BitBlit(dst1+pitchDst, pitchDst*2,src2,pitchSrc,w,h);
        }
        image->copyInfo(cur);
		vidCache->unlockAll();
        return 1;
}
