/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>
#include <string>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "DIA_coreToolkit.h"
#include "DIA_factory.h"
#include "ADM_vidMisc.h"
#include "blend.h"
#include "blend_desc.cpp"
/**
        \class AVDM_BlendFrames
 *      \brief fade video plugin
 */
class AVDM_BlendFrames : public  ADM_coreVideoFilter
{
protected:
                blend          param;
                uint32_t       **buffer;
                uint32_t      accumulated;
                //void         AccumulateFrame(ADMImage *buffer,ADMImage *frame);
                //void         WriteFrameAndClearBuffer(ADMImage *buffer,ADMImage *frame,uint32_t N);
public:
                             AVDM_BlendFrames(ADM_coreVideoFilter *previous,CONFcouple *conf);
                             ~AVDM_BlendFrames();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
   //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples);   /// Return the current filter configuration
        virtual void         setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void);           /// Start graphical user interface

};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(AVDM_BlendFrames,
	1,0,0,              // Version
	ADM_UI_ALL,         // UI
	VF_TRANSFORM,       // Category
	"blend",            // internal name (must be uniq!)
	QT_TRANSLATE_NOOP("blend","Blend Frames"),// Display name
	QT_TRANSLATE_NOOP("blend","Blend groups of N frames into a single frame.  Useful for speeding up slow motion footage or creating timelapses.") // Description
);   
/**
 * \fn configure
 * \brief UI configuration
 * @param 
 * @return 
 */
bool AVDM_BlendFrames::configure()
{
#define MAX_BLEND_FRAMES 16777216//2^32/2^8 This is the in-all-cases limit, but on average it should be able to support probably 25% more.  However, no frames will be exported unless the number of frames in the video is equal or greater.
  diaElemUInteger N(&(param.N),QT_TRANSLATE_NOOP("blend","Frames"),1,MAX_BLEND_FRAMES);
  diaElem *elems[1]={&N};
  if(diaFactoryRun(QT_TRANSLATE_NOOP("blend","Blend"),1,elems)){
    info.totalDuration=previousFilter->getInfo()->totalDuration/((uint64_t)param.N);//This bad boy reports the proper duration to the loading bar
    info.markerA=previousFilter->getInfo()->markerA/((uint64_t)param.N);
    info.markerB=previousFilter->getInfo()->markerB/((uint64_t)param.N);
    return 1;
  }else
    return 0;
}
/**
 *      \fn getConfiguration
 * 
 */
const char *AVDM_BlendFrames::getConfiguration(void)
{
    static char conf[12];
    snprintf(conf,12," N:%d ",param.N);
    return conf;
}

/**
 * \fn ctor
 * @param in
 * @param couples
 */
AVDM_BlendFrames::AVDM_BlendFrames(ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
    if(!setup || !ADM_paramLoad(setup,blend_param,&param))
    {
        // Default value
        param.N=1;
    }
    accumulated=0;
    buffer=NULL;
    info.totalDuration=previousFilter->getInfo()->totalDuration/((uint64_t)param.N);
    info.markerA=previousFilter->getInfo()->markerA/((uint64_t)param.N);
    info.markerB=previousFilter->getInfo()->markerB/((uint64_t)param.N);
}
/**
 * \fn setCoupledConf
 * \brief save current setup from couples
 * @param couples
 */
void AVDM_BlendFrames::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, blend_param, &param);
}

/**
 * \fn getCoupledConf
 * @param couples
 * @return setup as couples
 */
bool AVDM_BlendFrames::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, blend_param,&param);
}

/**
 * \fn dtor
 */
AVDM_BlendFrames::~AVDM_BlendFrames(void)
{
	if(buffer)      
	{
    	for(int i=0;i<1;i++)
    		delete [] buffer[i];
		delete [] buffer;
		buffer=NULL;
	}
}
/**
 * 
 * @param source
 * @param source2
 * @param dest
 * @param offset
 * @return 
 */

/**
 * \fn getNextFrame
 * @param fn
 * @param image
 * @return 
 */
bool AVDM_BlendFrames::getNextFrame(uint32_t *fn,ADMImage *image)
{
	while(true){
		if(previousFilter->getNextFrame(fn,image)==false)
			return false;
		
		if(buffer==NULL){
			//Create new 32 bit accumulation buffer
			buffer = new uint32_t*[3];
			for(int i=0;i<3;i++)
			{
				int w=(int)image->GetWidth((ADM_PLANE)i);
				int h=(int)image->GetHeight((ADM_PLANE)i);
				buffer[i] = new uint32_t[w*h];
				//I know that there is some way to initialize this with zeroes more efficiently, but I don't know how to do it.
				for(int y=0;y<h;y++)
				{
					for(int x=0;x<w;x++)
					{
						buffer[i][y*w+x]=0;
					}
				}
			}
		}

		//Accumulate frame into buffer
		uint8_t *fplanes[3];
		int fpitches[3];
		image->GetReadPlanes(fplanes);
		image->GetPitches(fpitches);
		for(int i=0;i<3;i++)
		{
			int w=(int)image->GetWidth((ADM_PLANE)i);
			int h=(int)image->GetHeight((ADM_PLANE)i);
			uint8_t *f=fplanes[i];
			for(int y=0;y<h;y++)
			{
				for(int x=0;x<w;x++)
				{
					buffer[i][y*w+x]+=(uint32_t)f[x];//
				}
				f+=fpitches[i];
			}        
		}
		accumulated++;

		//Output a frame when N frames have been accumulated
		if(accumulated==param.N){
			accumulated=0;
			//Divide buffer by N and write to 'image'
			//image=new ADMImageDefault(frame->GetWidth(PLANAR_Y),frame->GetHeight(PLANAR_Y));
			//image->copyInfo(frame);//Who knows what crazy info the frame has
			if(image->Pts!=ADM_NO_PTS)
				image->Pts=image->Pts/param.N;
			uint8_t *iplanes[3];
			image->GetWritePlanes(iplanes);
			for(int i=0;i<3;i++)
			{
				int w=(int)image->GetWidth((ADM_PLANE)i);
				int h=(int)image->GetHeight((ADM_PLANE)i);
				uint8_t *ip=iplanes[i];
				for(int y=0;y<h;y++)
				{
					for(int x=0;x<w;x++)
					{
						ip[x]=(uint8_t)(buffer[i][y*w+x]/(uint32_t)param.N);//Not sure if this will cast weirdly//It casted weirdly and made everything green, fixed now
						buffer[i][y*w+x]=0;//Reset buffer to 0
					}
					ip+=fpitches[i];
				}
			}
			return true;
		}
	}
	return false;
}
//EOF
