/***************************************************************************
                          Hue/Saturation filter ported from mplayer 
 (c) Michael Niedermayer
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>
#include "DIA_flyDialog.h"
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "hue.h"
#include "hue_desc.cpp"


extern uint8_t DIA_getHue(hue *param, ADM_coreVideoFilter *in);
/**
    \class ADMVideoHue
*/
class  ADMVideoHue:public ADM_coreVideoFilterCached
{

  protected:
            void            update(void);
            hue             _param;    
            float           _hue;
            float           _saturation;
  public:
                             ADMVideoHue(ADM_coreVideoFilter *in,CONFcouple *couples)   ;
                             ~ADMVideoHue();

               
       virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	   virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
	   virtual void setCoupledConf(CONFcouple *couples);
       virtual bool         configure(void) ;                 /// Start graphical user interface        

}     ;



// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ADMVideoHue,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_COLORS,            // Category
                        "hue",            // internal name (must be uniq!)
                        "Mplayer Hue",            // Display name
                        ("Adjust hue and saturation.") // Description
                    );
/**
    \fn HueProcess_C
*/
void HueProcess_C(uint8_t *udst, uint8_t *vdst, uint8_t *usrc, uint8_t *vsrc, int dststride, int srcstride,
		    int w, int h, float hue, float sat)
{
	int i;
	const int s= (int)rint(sin(hue) * (1<<16) * sat);
	const int c= (int)rint(cos(hue) * (1<<16) * sat);

	while (h--) {
		for (i = 0; i<w; i++)
		{
			const int u= usrc[i] - 128;
			const int v= vsrc[i] - 128;
			int new_u= (c*u - s*v + (1<<15) + (128<<16))>>16;
			int new_v= (s*u + c*v + (1<<15) + (128<<16))>>16;
			if(new_u & 768) new_u= (-new_u)>>31;
			if(new_v & 768) new_v= (-new_v)>>31;
			udst[i]= new_u;
			vdst[i]= new_v;
		}
		usrc += srcstride;
		vsrc += srcstride;
		udst += dststride;
		vdst += dststride;
	}
}

/**
    \fn configure
*/
bool ADMVideoHue::configure()
{
uint8_t r=0;
  
  r=  DIA_getHue(&_param, previousFilter);
  if(r) update();
  return r;  
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoHue::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Hue :%2.2f %2.2f",_param.hue,_param.saturation);
    return s;
}
/**
    \fn ctor
*/
ADMVideoHue::ADMVideoHue(  ADM_coreVideoFilter *in,CONFcouple *couples) :
        ADM_coreVideoFilterCached(1,in,couples)
{
    if(!couples || !ADM_paramLoad(couples,hue_param,&_param))
    {
        _param.hue =0.0;                
        _param.saturation=1.0;
    }      
    update();
}
/**
    \fn update
*/
void ADMVideoHue::update(void)
{
    _hue=_param.hue*M_PI/180.;
    _saturation=(100+_param.saturation)/100;
}
/**
    \fn dtor
*/
ADMVideoHue::~ADMVideoHue()
{
  
}
/**
    \fn getCoupledConf
*/
bool         ADMVideoHue::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, hue_param,&_param);
}

void ADMVideoHue::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, hue_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool         ADMVideoHue::getNextFrame(uint32_t *fn,ADMImage *image)
{
ADMImage *src;
        src=vidCache->getImage(nextFrame);
        if(!src)
            return false; // EOF
        *fn=nextFrame++;
        image->copyInfo(src);

        image->copyPlane(src,image,PLANAR_Y); // Luma is untouched

        HueProcess_C(image->GetWritePtr(PLANAR_V), image->GetWritePtr(PLANAR_U),
                     src->GetReadPtr(PLANAR_V), src->GetReadPtr(PLANAR_U),
                    
                image->GetPitch(PLANAR_U),src->GetPitch(PLANAR_U), // assume u&v pitches are =
                info.width>>1,info.height>>1, 
                _hue, _saturation);
 
        vidCache->unlockAll();
        return 1;
}
//void HueProcess_C(uint8_t *udst, uint8_t *vdst, uint8_t *usrc, uint8_t *vsrc, 
//int dststride, int srcstride,   int w, int h, float hue, float sat)

  

