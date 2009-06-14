//
// C++ Implementation: Soften
//
// Description: 
//
// See .h file
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

//
//	This is the very unoptimized version
//	could use some stuff and some mmx too
//

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"

#include "ADM_vidSoften.h"
#include "DIA_enter.h"
#include "DIA_factory.h"

static FILTER_PARAM softParam={3,{"radius","luma","chroma"}};
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

//********** Register chunk ************
//REGISTERX(VF_SHARPNESS, "soften",QT_TR_NOOP("Soften"),QT_TR_NOOP(
//"A variant of stabilize."),VF_SOFTEN,1,soften_create,soften_script);
VF_DEFINE_FILTER(ADMVideoMaskedSoften,softParam,
    soften,
                QT_TR_NOOP("Soften"),
                1,
                VF_SHARPNESS,
                QT_TR_NOOP("A variant of stabilize."));
//********** Register chunk ************


uint8_t ADMVideoMaskedSoften::configure( AVDMGenericVideoStream *instream)
{
        _in=instream;
        /*uint32_t luma,chroma;
	uint32_t radius;
	*/
        
        diaElemUInteger luma(&(_param->luma),QT_TR_NOOP("_Luma threshold:"),0,255);
        diaElemUInteger chroma(&(_param->chroma),QT_TR_NOOP("C_hroma threshold:"),0,255);
        diaElemUInteger radius(&(_param->radius),QT_TR_NOOP("_Radius:"),1,60);
	  
    diaElem *elems[3]={&luma,&chroma,&radius};
  
    return diaFactoryRun(QT_TR_NOOP("Soften"),3,elems);
}
uint8_t	ADMVideoMaskedSoften::getCoupledConf( CONFcouple **couples)
{

			*couples=new CONFcouple(3);

			CSET(radius);
			CSET(luma);
			CSET(chroma);

		return 1;	
}
char *ADMVideoMaskedSoften::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Soften : radius: %lu l:%lu c:%lu", 
		_param->radius,_param->luma, _param->chroma);
        
}

ADMVideoMaskedSoften::~ADMVideoMaskedSoften()
{
	if(_uncompressed)
 		delete _uncompressed;	
 	_uncompressed=NULL;
}


 ADMVideoMaskedSoften::ADMVideoMaskedSoften( AVDMGenericVideoStream *in,CONFcouple *couples)
{
		if(distMatrixDone==false)
        {
            buildDistMatrix();
            distMatrixDone=true;
        }
		_uncompressed=NULL;
		_in=in;
		ADM_assert(in);
		if(!couples)
		{
			
			_param=NEW(MaskedSoften_CONF);
	    		_param->radius=2; 
	    		_param->luma=5;
			_param->chroma=5;
			
	    	}
		else
		{
			_param=NEW(MaskedSoften_CONF);
			GET(radius);
			GET(luma);
			GET(chroma);
			
						
		}
		ADM_assert(in);
		
		memcpy(&_info,_in->getInfo(),sizeof(_info));	
			    	
	    	//_uncompressed=new uint8_t[3*_info.width*_info.height];	
		_uncompressed=new ADMImage(_info.width,_info.height);	
		

}
uint8_t ADMVideoMaskedSoften::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
		uint32_t page=_info.width*_info.height;
		int32_t radius=_param->radius;
		uint32_t offset;
		
		int blockx,blocky;
		
		*len=(page*3)>>1;
		if(frame>=_info.nb_frames) return 0;		
		
				
		if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags))
				 		return 0;

		// do luma only ATM
		// copy chroma
		memcpy(UPLANE(data),UPLANE(_uncompressed),page>>2);
		memcpy(VPLANE(data),VPLANE(_uncompressed),page>>2);
		
		// for luma, the radius first lines /last lines are unchanged
		memcpy(YPLANE(data),YPLANE(_uncompressed),radius*_info.width);
		
		offset=page-_info.width*radius-1;
		
		memcpy(YPLANE(data)+offset,
			YPLANE(_uncompressed)+offset,
			radius*_info.width);

		uint8_t *src,*dst;
		uint32_t val,cur,coef;
		//
		data->copyInfo(_uncompressed);
		// optimized one
		if(radius==2) return radius5(YPLANE(_uncompressed),YPLANE(data));
		if(radius==1) return radius3(YPLANE(_uncompressed),YPLANE(data));
		
		for(uint32_t y=radius;y<_info.height-radius;y++)
		{
			src=YPLANE(_uncompressed)+y*_info.width;
			dst=YPLANE(data)+y*_info.width;
			
			memcpy(dst,src,radius);
			src+=radius;
			dst+=radius;
			
			for(uint32_t x=radius;x<_info.width-radius;x++)
			{
				coef=0;
				val=0;
			
				for( blocky=-radius;blocky<=radius;blocky++)
				{
					for( blockx=-radius;blockx<=radius;blockx++)
					{
						cur=*(src+blockx+blocky*_info.width);
						
						if( distMatrix[cur][*src]<=_param->luma)
						{
							coef++;
							val+=cur;
						}
					
					
					}
				}
				ADM_assert(coef);
				if(coef!=1)
					val=(val+(coef>>1)-1)/coef;
				*dst++=val;
				src++;
				
				//*dst++=*src++;
			}
			memcpy(dst,src,radius);
		
		}	
	return 1;
}
uint8_t ADMVideoMaskedSoften::radius5(uint8_t *_uncompressed, uint8_t *data) 
{
int blockx,blocky;
uint32_t val,coef;

uint8_t *src,*dst;
uint8_t cur;

uint8_t *c0,*c1,*c2,*c3,*c4,ref;



	for(uint32_t y=2;y<_info.height-2;y++)
		{
			src=_uncompressed+y*_info.width;
			dst=data+y*_info.width;
			
			*dst++=*src++;
			*dst++=*src++;
			
			for(uint32_t x=2;x<_info.width-2;x++)
			{
				coef=0;
				val=0;
				c0=src-2-2*_info.width;
				c1=c0+_info.width;
				c2=c1+_info.width;
				c3=c2+_info.width;
				c4=c3+_info.width;
				
				ref=*src;
					
#define CHECK(x) cur=*x;if(distMatrix[cur][ref]<=_param->luma) {coef++;val+=cur;}x++;					
				for( blockx=5;blockx>0;blockx--)
				{
					CHECK(c0);
					CHECK(c1);
					CHECK(c2);
					CHECK(c3);
					CHECK(c4);
				}
				ADM_assert(coef);
				if(coef!=1)
					val=(val+(coef>>1)-1)/coef;

				*dst++=val;
				src++;
			}
			*dst++=*src++;
			*dst++=*src++;		
		}	
}
uint8_t ADMVideoMaskedSoften::radius3(uint8_t *_uncompressed, uint8_t *data) 
{
int blockx,blocky;
uint32_t val,coef;

uint8_t *src,*dst;
uint8_t cur;

uint8_t *c0,*c1,*c2,ref;

uint8_t *dist;

	for(uint32_t y=1;y<_info.height-1;y++)
		{
			src=_uncompressed+y*_info.width;
			dst=data+y*_info.width;
			
			*dst++=*src++;
			
			
			for(uint32_t x=1;x<_info.width-1;x++)
			{
				coef=0;
				val=0;
				c0=src-1-_info.width;
				c1=c0+_info.width;
				c2=c1+_info.width;
				
				ref=*src;
				dist=distMatrix[ref];
#undef CHECK				
#define CHECK(x) cur=*x;if(dist[cur]<=_param->luma) {coef++;val+=cur;}x++;					
					CHECK(c0);
					CHECK(c0);
					CHECK(c0);
					
					CHECK(c1);
					CHECK(c1);					
					CHECK(c1);
					
					CHECK(c2);
					CHECK(c2);					
					CHECK(c2);
				
					
				ADM_assert(coef);
				if(coef!=1)
					val=(val+(coef>>1)-1)/coef;
				*dst++=val;
				src++;
			}
			*dst++=*src++;
			
		}	
}






