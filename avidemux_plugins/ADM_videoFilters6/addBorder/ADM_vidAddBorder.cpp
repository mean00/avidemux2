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
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidAddBorder.h"
#include "DIA_factory.h"
#include "addBorder_desc.cpp"

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *addBorders::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"Add Border : Left:%"LU" Right:%"LU" Top:%"LU" Bottom:%"LU" => %"LU"x%"LU"\n",
                param.left,param.right,param.top,param.bottom,
                info.width,info.height);
    return conf;
}
/**
    \fn ctor
*/
addBorders::addBorders( ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{	
	 if(!setup || !ADM_paramLoad(setup,addBorder_param,&param))
    {
        // Default value
        param.left=0;
        param.right=0;
        param.top=0;
        param.bottom=0;
        
    }
	info.width=in->getInfo()->width+param.left+param.right;
    info.height=in->getInfo()->height+param.top+param.bottom;
  	  	
}
/**
    \fn dtor
*/
addBorders::~addBorders()
{

}

/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         addBorders::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, addBorder_param,&param);
}
/**

*/
bool addBorders::getNextFrame(uint32_t *fn,ADMImage *image)
{
    ADMImageRefWrittable ref(previousFilter->getInfo()->width,previousFilter->getInfo()->height);
 
    uint32_t offset=param.top*image->GetPitch(PLANAR_Y);
    ref._planes[0]=image->GetWritePtr(PLANAR_Y)+param.left+offset;


    offset=(param.top>>1)*image->GetPitch(PLANAR_U);
    ref._planes[1]=image->GetWritePtr(PLANAR_U)+(param.left>>1)+offset;

    offset=(param.top>>1)*image->GetPitch(PLANAR_V);
    ref._planes[2]=image->GetWritePtr(PLANAR_V)+(param.left>>1)+offset;

    ref._planeStride[0]=info.width;
    ref._planeStride[1]=info.width>>1;
    ref._planeStride[2]=info.width>>1;
    if(false==previousFilter->getNextFrame(fn,&ref))
    {
        ADM_warning("FlipFilter : Cannot get frame\n");
        return false;
    }
    // Now do fill
    return true;
}

#if 0
		if(frame>=_info.nb_frames) 
		{
			printf("Filter : out of bound!\n");
			return 0;
		}
	
		ADM_assert(_param);									
								
			// read uncompressed frame
       		if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;
       		
				// blacken screen
				memset(YPLANE(data),16,_info.width*_info.height);
				memset(UPLANE(data),128,(_info.width*_info.height)>>2);
				memset(VPLANE(data),128,(_info.width*_info.height)>>2);

				// do luma
				uint8_t *src,*dest;
       		uint32_t y,x,line,lineout;
       		
       		y=_in->getInfo()->height;
       		x=_in->getInfo()->width;
       		line=x;
		lineout=_info.width;
		
		// copy Luma
       		src=YPLANE(_uncompressed);
       		dest=YPLANE(data)+_param->left+_info.width*_param->top;
       		
       		for(uint32_t k=y;k>0;k--)
       		{
       		 	    memcpy(dest,src,line);
       		 	    src+=line;
       		 	    dest+=lineout;
       		}
       		 
		// U and V now
		uint8_t *src_u,*src_v;
		uint8_t *dst_u,*dst_v;

       		src_u=UPLANE(_uncompressed);
       		src_v=VPLANE(_uncompressed);
       		line>>=1;
       		lineout>>=1;       		       		 	
		dst_u=UPLANE( data)+(_info.width*_param->top>>2)+
						(_param->left>>1);;
		dst_v= VPLANE( data)+(_info.width*_param->top>>2)+
						(_param->left>>1);;

       		 for(uint32_t k=y>>1;k>0;k--)
       		 {
       		 	memcpy(dst_u,src_u,line);
       		 	memcpy(dst_v,src_v,line);

       			src_u+=line;
       			src_v+=line;

       			dst_u+=lineout;
       		    	dst_v+=lineout;

       		 }
       		  *len= _info.width*_info.height+(_info.width*_info.height>>1);
		  data->copyInfo(_uncompressed);

      return 1;
}
#endif
/**
    \fn configure
*/
bool addBorders::configure(void)
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
          
          diaElemUInteger dleft(&left,QT_TR_NOOP("_Left border:"),       0,width);
          diaElemUInteger dright(&right,QT_TR_NOOP("_Right border:"),    0,width);
          diaElemUInteger dtop(&(top),QT_TR_NOOP("_Top border:"),          0,height);
          diaElemUInteger dbottom(&(bottom),QT_TR_NOOP("_Bottom border:"), 0,height);
            
          diaElem *elems[4]={&dleft,&dright,&dtop,&dbottom};
          if(diaFactoryRun(QT_TR_NOOP("Add Borders"),4,elems))
          {
            if((left&1) || (right&1)|| (top&1) || (bottom&1))
            {
              GUI_Error_HIG(QT_TR_NOOP("Incorrect parameters"),QT_TR_NOOP("All parameters must be even and within range.")); 
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
                info.width=width+left+right;
                info.height=height+top+bottom;
                return 1;
            }
          }
          return 0;
      }
}




