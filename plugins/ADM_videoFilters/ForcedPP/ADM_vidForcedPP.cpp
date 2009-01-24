//
// C++ Implementation: ADM_vidForcedPP
//
// Description: 
//
//	Force postprocessing assuming constant quant & image type
//	Uselefull on some badly authored DVD for example
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"

#include "ADM_vidForcedPP.h"
#include "DIA_enter.h"
#include "DIA_factory.h"

// FIXME
#define AVI_KEY_FRAME   0x10
#define AVI_B_FRAME	0x4000	 // hopefully it is not used..


static FILTER_PARAM ppParam={3,{"postProcType","postProcStrength","forcedQuant"}};
//********** Register chunk ************

VF_DEFINE_FILTER(ADMVideoForcedPP,ppParam,
                forcedpp,
                QT_TR_NOOP("Forced postprocessing"),
                1,
                VF_NOISE,
                QT_TR_NOOP("Apply blind postprocessing.."));
//********** Register chunk ************
uint8_t ADMVideoForcedPP::configure( AVDMGenericVideoStream *instream)
{
	_in=instream;
        
        #define PX(x) &(_param->x)
  
        
    diaElemUInteger   postProcStrength(PX(postProcStrength),QT_TR_NOOP("_Filter strength:"),0,5);
    diaElemUInteger   quant(PX(forcedQuant),QT_TR_NOOP("_Quantizer:"),1,31);
    //diaElemToggle     swapuv(PX(swapuv),QT_TR_NOOP("Swap U&V"));
    
    uint32_t hzd,vzd,dring;
    
#define DOME(x,y) if(_param->postProcType & x) y=1;else y=0;
    
    DOME(1,hzd);
    DOME(2,vzd);
    DOME(4,dring);
    
     diaElemToggle     fhzd(&hzd,QT_TR_NOOP("_Horizontal deblocking"));
     diaElemToggle     fvzd(&vzd,QT_TR_NOOP("_Vertical deblocking"));
     diaElemToggle     fdring(&dring,QT_TR_NOOP("_Deringing"));
    
    
    
      diaElem *elems[5]={&postProcStrength,&quant,&fhzd
                        ,&fvzd,&fdring};

   if(diaFactoryRun(QT_TR_NOOP("Forced Postprocessing"),5,elems))
  {
#undef DOME
#define DOME(x,y) if(y) _param->postProcType |=x;
    _param->postProcType =0;
    DOME(1,hzd);
    DOME(2,vzd);
    DOME(4,dring);
    updatePostProc(&_postproc );	
    return 1;
  }
  return 0;	
 	
}
uint8_t	ADMVideoForcedPP::getCoupledConf( CONFcouple **couples)
{

			*couples=new CONFcouple(3);

			CSET(postProcType);
			CSET(postProcStrength);
			CSET(forcedQuant);

		return 1;	
}
char *ADMVideoForcedPP::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Forced pp : %lu %lu",_param->postProcType,_param->postProcStrength);
        
}

ADMVideoForcedPP::~ADMVideoForcedPP()
{
	if(_uncompressed)
 		delete _uncompressed;
	if(_postproc.ppContext)
		deletePostProc(&_postproc);

 	_uncompressed=NULL;
}


 ADMVideoForcedPP::ADMVideoForcedPP( AVDMGenericVideoStream *in,CONFcouple *couples)
{
		printf("Adding forced PP filter\n");
		_uncompressed=NULL;
		_in=in;
		ADM_assert(in);
		if(!couples)
		{
			
			_param=NEW(PP_CONF);
	    		_param->postProcType=7; // all
	    		_param->postProcStrength=5;
			_param->forcedQuant=10;
			
	    	}
		else
		{
			_param=NEW(PP_CONF);
			GET(postProcType);
			GET(postProcStrength);
			GET(forcedQuant);
			
						
		}
		ADM_assert(in);
		
		memcpy(&_info,_in->getInfo(),sizeof(_info));	
		
	    	initPostProc(&_postproc,_info.width,_info.height);
		_postproc.postProcType=_param->postProcType;
		_postproc.postProcStrength=_param->postProcStrength;
		_postproc.forcedQuant=_param->forcedQuant;
		updatePostProc(&_postproc);	    	
	    	
	    	//_uncompressed=new uint8_t[3*_info.width*_info.height];	
		_uncompressed=new ADMImage(_info.width,_info.height);	
		

}
uint8_t ADMVideoForcedPP::getFrameNumberNoAlloc(uint32_t frame,
							uint32_t *len,
							ADMImage *data,
							uint32_t *flags)
{
		uint32_t page=_info.width*_info.height;
		
		*len=(page*3)>>1;
		if(frame>=_info.nb_frames) return 0;		
		
		if(!(_postproc.postProcType && _postproc.postProcStrength) )
		{
			// disabled
			if(!_in->getFrameNumberNoAlloc(frame, len,data,flags)) return 0;
				
			return 1;
		
		}
			// we do postproc !
			// keep
			const uint8_t *iBuff[3];
			uint8_t *oBuff[3];
			int strideTab[3],strideTab2[3];			
				
				
			if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags))
			 		return 0;

			oBuff[0]=YPLANE(data);
                        oBuff[1]=VPLANE(data);
                        oBuff[2]=UPLANE(data);

				
			iBuff[0]=YPLANE(_uncompressed);
		 	iBuff[1]=VPLANE(_uncompressed);
 		 	iBuff[2]=UPLANE(_uncompressed);
				
				
		        strideTab[0]=strideTab2[0]=_info.width;
			strideTab[1]=strideTab2[1]=_info.width>>1;
			strideTab[2]=strideTab2[2]=_info.width>>1;
	
			int type;
			if(_uncompressed->flags&AVI_KEY_FRAME)
					type=1;
			else if(_uncompressed->flags & AVI_B_FRAME)
					type=3;
				else
					type=2;
		 	pp_postprocess(
		      		iBuff,
		        	strideTab,
		        	oBuff,
		        	strideTab2,
				_info.width,
		        	_info.height,
		        	NULL,
		          	0,
		         	_postproc.ppMode,
		          	_postproc.ppContext,
		          	type); // I ?
			  	
			
		
			data->copyInfo(_uncompressed);	
			//printf("Type:%d\n",type);
	return 1;
}

