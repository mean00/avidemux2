/***************************************************************************
                          ADM_vidDenoise.cpp  -  description
                             -------------------
    begin                : Mon Nov 25 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
    
    Denoiser inspired from DNR in transcode
    Ported to YV12 and simplified
    
   Original code  Copyright (C) Gerhard Monzel - November 2001
 
    
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

#include "ADM_videoFilterDynamic.h"
#include "ADM_vidDenoise.h"


#include "DIA_factory.h"

static FILTER_PARAM denoiseParam={5,{"lumaLock","lumaThreshold","chromaLock","chromaThreshold",
					"sceneChange"}};

//********** Register chunk ************

VF_DEFINE_FILTER(ADMVideoDenoise,denoiseParam,
                denoise,
                QT_TR_NOOP("Denoise"),
                1,
                VF_NOISE,
                QT_TR_NOOP("Port of Transcode DNR."));
//********** Register chunk ************


//static uint8_t matrixReady=0;
//static uint8_t doOnePix(uint8_t *in,uint8_t *out,uint8_t *lock,uint8_t *nb);


char *ADMVideoDenoise::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Denoise : Lum :%02ld/:%02ld / Chm :%02ld/%02ld",
  								_param->lumaLock,
          				_param->lumaThreshold,
              		_param->chromaLock,
                	_param->chromaThreshold);
        
}
static uint8_t distMatrix[256][256];
static uint32_t fixMul[16];
static bool distMatrixDone=false;

static void buildDistMatrix( void )
{
int d;	
	for(uint32_t y=255;y>0;y--)
	for(uint32_t x=255;x>0;x--)
	{
		  d=x-y;
		  if(d<0) d=-d;
		  distMatrix[x][y]=d;
		
	}

	 for(int i=1;i<16;i++)
                        {
                                        fixMul[i]=(1<<16)/i;
                        }

}

//_______________________________________________________________

ADMVideoDenoise::ADMVideoDenoise(
									AVDMGenericVideoStream *in,CONFcouple *couples)
{

    if(distMatrixDone==false)
    {
        buildDistMatrix();
        distMatrixDone=true;
    }
  	_in=in;		
   	memcpy(&_info,_in->getInfo(),sizeof(_info));  			 	
    uint32_t page;
    
  _info.encoding=1;
  
  page= _in->getInfo()->width*_in->getInfo()->height;
  
//  _uncompressed=new uint8_t [page];
  _uncompressed=new ADMImage(_in->getInfo()->width,_in->getInfo()->height);
  ADM_assert(_uncompressed);
  
 // _locked=new uint8_t [page];
  _locked=new ADMImage(_in->getInfo()->width,_in->getInfo()->height);
  ADM_assert(_locked);
 
//	_lockcount=new uint8_t [page];


  
   _lockcount=new ADMImage(_in->getInfo()->width,_in->getInfo()->height);
  memset(YPLANE(_lockcount),0,page);  
  memset(UPLANE(_lockcount),0,page>>2);  
  memset(VPLANE(_lockcount),0,page>>2);  
        
  _param=NULL;
  
  if(couples)
  	{
			_param=NEW(NOISE_PARAM);
			GET(lumaLock);
			GET(lumaThreshold);
			GET(chromaLock);
			GET(chromaThreshold);
			GET(sceneChange);
		 }
	else
		{
			  #define XXX 1
			  _param=NEW(NOISE_PARAM);
			  _param->lumaLock=  4*XXX;
			  _param->lumaThreshold= 10*XXX;
			  _param->chromaLock=  8*XXX;
			  _param->chromaThreshold= 16*XXX;
        _param->sceneChange=  30*XXX;
			}
  	  _lastFrame=0xfffffff0;	
}


uint8_t	ADMVideoDenoise::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(5);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
	CSET(lumaLock);
	CSET(lumaThreshold);
	CSET(chromaLock);
	CSET(chromaThreshold);
	CSET(sceneChange);

	return 1;

}
ADMVideoDenoise::~ADMVideoDenoise()
{
 	
	delete  _uncompressed;
 	delete  _locked;
  	delete  _lockcount;
  DELETE(_param);
  
  _uncompressed=_locked=_lockcount=NULL;
}

//
//	Remove y and v just keep U and expand it
//
uint8_t ADMVideoDenoise::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
   //uint32_t x,w;
  	uint32_t page; 
   		ADM_assert(_param);
		if(frame>= _info.nb_frames) return 0;
								
			
       		if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;

		
		page=_info.width*_info.height;  
		*len=(page*3)>>1;           

	if((_lastFrame+1)!=frame) // async jump
	{
			// just copy it 
			memcpy(YPLANE(data),YPLANE(_uncompressed),page);
			memcpy(UPLANE(data),UPLANE(_uncompressed),page>>2);
			memcpy(VPLANE(data),VPLANE(_uncompressed),page>>2);
			
			memcpy(YPLANE(_locked),YPLANE(_uncompressed),page);
			memcpy(UPLANE(_locked),UPLANE(_uncompressed),page>>2);
			memcpy(VPLANE(_locked),VPLANE(_uncompressed),page>>2);
			
			_lastFrame=frame;
			return 1;
	}          
	_lastFrame=frame;
          
          // copy chroma for now
        
         
          
          //
          //uint32_t count=0;
          //uint32_t cell=page*4; // size of luma
          uint8_t *in,*out,*lock,*nb;
          uint8_t *uin,*uout,*ulock,*unb;
          uint8_t *vin,*vout,*vlock,*vnb;
          
          
          //uint32_t d;
          // init all
          
          // luma
          nb=YPLANE(_lockcount);
          lock=YPLANE(_locked);
          in=YPLANE(_uncompressed);
          out=YPLANE(data);
          // u
          unb=UPLANE(_lockcount);
          ulock=UPLANE(_locked);
          uin=UPLANE(_uncompressed);
          uout=UPLANE(data);
          // v
          vnb=VPLANE(_lockcount);
          vlock=VPLANE(_locked);
          vin=VPLANE(_uncompressed);
          vout=VPLANE(data);
          
          
          uint32_t xx,yy/*,dl*/,du,dv;
          uint32_t locked=0;
          for(yy=_info.height>>1;yy>0;yy--)
          {
	          for(xx=_info.width>>1;xx>0;xx--)          
  	        {
			du=distMatrix[*uin][*ulock];	
			dv=distMatrix[*vin][*vlock];		
						
			// if chroma is locked , we try to lock luma
			if( (du<_param->chromaLock)
				 && (dv<_param->chromaLock))
			 {  
				*uout=*ulock;
 				*vout=*vlock;

#define PIX(z) 		doOnePix(in+z,out+z,lock+z,nb+z) 
				locked+=PIX(0)+	PIX(1)+ PIX(_info.width)+PIX(_info.width+1);
			}
			else
			 // if chroma is blended, we blend luma
#undef PIX								  
#define PIX(z) 		doBlend(in+z,out+z,lock+z,nb+z)									 
				if( (du<_param->chromaThreshold)
					 && (dv<_param->chromaThreshold))
				{
			 		PIX(0);
				    	PIX(1);
				     	PIX(_info.width);
				     	PIX(_info.width+1);	
				      *uout=*ulock=(*uin+*uin)>>1;
 					*vout=*vlock=(*vin+*vin)>>1;
				}
#undef PIX											
										
			else
			{
#define PIX(z) *(out+z)=*(lock+z)=*(in+z);*(nb+z)=0			
											
				PIX(0);
				PIX(1);
				PIX(_info.width);
				PIX(_info.width+1);		
				*uout=*ulock=*uin;
 				*vout=*vlock=*vin;
				
#undef PIX		
			}
								  
											                        				                        
			uin++;uout++;ulock++;unb++;   
			vin++;vout++;vlock++;vnb++;   
			in++;out++;lock++;nb++;   
			in++;out++;lock++;nb++;   
							
		}
            // 
            in+=_info.width;
            out+=_info.width;
            lock+=_info.width;
            nb+=_info.width;            						
	};
          
          if(locked>((page*3)>>2)) // if more than 75% pixel not locked -> scene change
          {
			memcpy(YPLANE(data),YPLANE(_uncompressed),page);
			memcpy(UPLANE(data),UPLANE(_uncompressed),page>>2);
			memcpy(VPLANE(data),VPLANE(_uncompressed),page>>2);
			
			memcpy(YPLANE(_locked),YPLANE(_uncompressed),page);
			memcpy(UPLANE(_locked),UPLANE(_uncompressed),page>>2);
			memcpy(VPLANE(_locked),VPLANE(_uncompressed),page>>2);
	}
      data->copyInfo(_uncompressed);  
      return 1;
}

//
//	0 copy
//  1 lock
//  2 threshold
//
uint8_t ADMVideoDenoise::doOnePix(uint8_t *in,uint8_t *out,uint8_t *lock,uint8_t *nb)
{
unsigned int d;
		d=distMatrix[*(in)][*(lock)]; 
		if(d<_param->lumaLock)         
		{								                
			if(*(nb)>30)  // out of scope -> copy new                   
			{  	// too much copy ->                              
				*(nb)=0;                       
				*(out)=(*(in)+*(lock))>>1;
				*(lock)=*(out);    	
				return DN_COPY;      
			}                                 
			else                               
			{                                   
				*(out)=*(lock);		
				*nb += 1; // *(nb)++;	
				return DN_LOCK;		
			}                  
		}                     
		else if(d< _param->lumaThreshold) 
			{                                  
				 *(nb)=0;                           
				*(out)=(*(in)+*(lock))>>1;	
				return DN_BLEND;
			}
			else   // too big delta
			{    
				 *(nb)=0; 
				*(out)=*(in);	  
				*(lock)=*(in);    
				return DN_COPY;
			}                     
					                           
			ADM_assert(0);
			return 0;

}
uint8_t ADMVideoDenoise::doBlend(uint8_t *in,uint8_t *out,uint8_t *lock,uint8_t *nb)
{
unsigned int d;
		   d=distMatrix[*(in)][*(lock)]; 
		   *nb=0;
		   
			if(d<_param->lumaThreshold)         
			{
					*(out)=(*(in)+*(lock))>>1;					
			}
			else
			*out=*in;
			return 0;
	
}


uint8_t ADMVideoDenoise::configure(AVDMGenericVideoStream * instream)
{
  UNUSED_ARG(instream);
  
#define PX(x) &(_param->x)
  
    diaElemUInteger   lumaLock(PX(lumaLock),QT_TR_NOOP("_Luma lock:"),0,255);
    diaElemUInteger   chromaLock(PX(chromaLock),QT_TR_NOOP("C_hroma lock:"),0,255);
    diaElemUInteger   lumaThreshold(PX(lumaThreshold),QT_TR_NOOP("L_uma threshold:"),0,255);
    diaElemUInteger   chromaThreshold(PX(chromaThreshold),QT_TR_NOOP("Ch_roma threshold:"),0,255);
    
    diaElemUInteger   sceneChange(PX(sceneChange),QT_TR_NOOP("_Scene change:"),0,100);
    
    
    
       diaElem *elems[5]={&lumaLock,&chromaLock,&lumaThreshold,&chromaThreshold,&sceneChange};
  
   return diaFactoryRun(QT_TR_NOOP("Denoise"),5,elems);
}

// EOF
