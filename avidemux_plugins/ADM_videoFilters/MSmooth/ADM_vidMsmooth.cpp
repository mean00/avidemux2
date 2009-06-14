/***************************************************************************
                          ADM_vidMsmooth  -  description
                             -------------------
    
    email                : fixounet@free.fr

    Port of Donal Graft MSMooth which is (c) Donald Graft
    http://www.neuron2.net
    http://puschpull.org/avisynth/decomb_reference_manual.html

 ***************************************************************************/
/*
	MSmooth plugin for Avisynth -- performs detail-preserving smoothing.

	Copyright (C) 2003 Donald A. Graft

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"

#include "DIA_factory.h"

#define aprintf(...) {}


#define GETFRAME(g, fp) \
{ \
	int GETFRAMEf; \
	GETFRAMEf = (g); \
	if (GETFRAMEf < 0) GETFRAMEf = 0; \
	if (GETFRAMEf > num_frames_hi - 1) GETFRAMEf = num_frames_hi - 1; \
	(fp) = vidCache->getImage(GETFRAMEf); \
}

#include "ADM_vidMSmooth_param.h"


extern uint8_t 	PutHintingData(unsigned char *video, unsigned int hint);
extern uint8_t 	GetHintingData(unsigned char *video, unsigned int *hint);
extern void 	BitBlt(uint8_t * dstp, int dst_pitch, const uint8_t* srcp,
            		int src_pitch, int row_size, int height);
//extern  void 	DrawString(uint8_t *dst, int x, int y, const char *s);
//extern  void    DrawString(ADMImage *dst, int x, int y, const char *s);
//#define DrawString(a,b,c,d) DrawString(NULL,b,c,d)
#define DrawString drawString

extern  void 	DrawStringYUY2(uint8_t *dst, int x, int y, const char *s); 

static void Blur_C(uint8_t *in, uint8_t *out, uint32_t w, uint32_t h) ;
#ifdef ADM_CPU_ALTIVEC
void Blur_Altivec(uint8_t *in, uint8_t *out, uint32_t w, uint32_t h);
#endif
#ifdef ADM_CPU_X86
void Blur_MMX(uint8_t *in, uint8_t *out, uint32_t w, uint32_t h);
#endif
class Msmooth : public AVDMGenericVideoStream
{
private:
	MSMOOTH_PARAM	*_param;
	VideoCache	*vidCache;
	uint8_t		show, debug;
	ADMImage 	*blur,*work,*mask,*final,*final2;
public:    

			Msmooth(AVDMGenericVideoStream *in,CONFcouple *couples)   ;
			~Msmooth();
    	uint8_t 	*GetFrameYV12(int n);
	void  		SmoothingPassYV12(const unsigned char *srcp, unsigned char *maskp, unsigned char *workp,
				unsigned char *finalp,int row_size, int height, int spitch, int dpitch);
	void 		EdgeMaskYV12(const unsigned char *srcp, unsigned char *blurp, unsigned char *workp,
				 unsigned char *maskp,int row_size, int height, int src_pitch, int blur_pitch);
				 
	char 		*printConf( void );
	uint8_t 	configure(AVDMGenericVideoStream *in);
	uint8_t		getCoupledConf( CONFcouple **couples);
	uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
				ADMImage *data,uint32_t *flags);
};


static FILTER_PARAM msmoothParam={4,{"showmask", "highq","threshold", "strength"}};


//REGISTERX(VF_SHARPNESS,   "msmooth",QT_TR_NOOP("MSmooth by Donald Graft"),QT_TR_NOOP
    //("Smooth the image, don't blur edges. Useful on anime."),VF_MSMOOTH,1,create_msmooth,msmooth_script);*
//********** Register chunk ************

VF_DEFINE_FILTER(Msmooth,msmoothParam,
    msmooth,
                QT_TR_NOOP("MSmooth by Donald Graft"),
                1,
                VF_SHARPNESS,
                QT_TR_NOOP("Smooth the image, don't blur edges. Useful on anime."));
//********** Register chunk ************

//_______________________________________________

Msmooth::Msmooth(AVDMGenericVideoStream *in,CONFcouple *couples)

{
	_in=in;		
   	memcpy(&_info,_in->getInfo(),sizeof(_info));    
  	_info.encoding=1;
	_uncompressed=NULL;		
  	_info.encoding=1;
	show=0;
	debug=0;
	_param=new MSMOOTH_PARAM;
	//
	if(couples)
		{
			GET(showmask);
			GET(threshold);
			GET(highq);
			GET(strength);	
		}
		else // Default
  		{
			_param->showmask=0;
			_param->threshold=15;
			_param->strength=3;
			_param->highq=1;			
		}
		
	uint32_t sz=(_info.width*_info.height*3)>>1;
	#define NW(x) x=new ADMImage(_info.width,_info.height);ADM_assert(x);
	NW(blur);
	NW(work);
	NW(mask);
	NW(final);
	NW(final2);
	
    	vidCache=new VideoCache(5,in);
}
//________________________________________________________
uint8_t	Msmooth::getCoupledConf( CONFcouple **couples)
{
	*couples=NULL;
	*couples=new CONFcouple(4);
#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
	CSET(showmask);
	CSET(threshold);
	CSET(highq);
	CSET(strength);	
	
	return 1;
}
//________________________________________________________
Msmooth::~Msmooth(void)
{

	if(vidCache) delete vidCache;
	if(_param) delete _param;
#undef NW
#define NW(x) if(x) {delete  x;x=NULL;}
	NW(blur);
	NW(work);
	NW(mask);
	NW(final);
	NW(final2);
}
//________________________________________________________
uint8_t Msmooth::configure(AVDMGenericVideoStream *in)
{
	_in=in;
	ADM_assert(_param);
        
        diaElemToggle toggle(&(_param->highq),QT_TR_NOOP("_High quality"));
        diaElemToggle mask(&(_param->showmask),QT_TR_NOOP("Show _mask"));
        diaElemUInteger threshold(&(_param->threshold),QT_TR_NOOP("_Threshold:"),0,100);
        diaElemUInteger strength(&(_param->strength),QT_TR_NOOP("_Strength:"),0,100);
	  
    diaElem *elems[4]={&toggle,&mask,&threshold,&strength};
  
    return diaFactoryRun(QT_TR_NOOP("MSmooth"),4,elems);
}

//________________________________________________________
char *Msmooth::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Donald Graft MSmooth");
        
}
	
//________________________________________________________
uint8_t Msmooth::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
				ADMImage *data,uint32_t *flags)
{
    ADMImage *src = vidCache->getImage(frame);
   
    ADMImage * deliver;
        if(frame>= _info.nb_frames) return 0;
	const unsigned char *srcpY = YPLANE(src) ;
	const unsigned char *srcp_savedY = srcpY;
    	int src_pitchY = _info.width;
	
	const unsigned char *srcpU =UPLANE(src);
	const unsigned char *srcp_savedU = srcpU;
    	int src_pitchUV = _info.width>>1;
	const unsigned char *srcpV =VPLANE(src);
	const unsigned char *srcp_savedV = srcpV;

    	unsigned char *blurpY = YPLANE(blur);//
	unsigned char *blurp_savedY = blurpY;
    	int blur_pitchY =  _info.width;
    	unsigned char *blurpU = UPLANE(blur);
	unsigned char *blurp_savedU = blurpU;
    	int blur_pitchUV = _info.width>>1;
    	unsigned char *blurpV = VPLANE(blur);
	unsigned char *blurp_savedV = blurpV;

    unsigned char *workpY = YPLANE(work); //->GetWritePtr(PLANAR_Y);
	unsigned char *workp_savedY = workpY;
    unsigned char *workpU = UPLANE(work);
	unsigned char *workp_savedU = workpU;
    unsigned char *workpV = VPLANE(work);
	unsigned char *workp_savedV = workpV;

    unsigned char *maskpY = YPLANE(mask); //->GetWritePtr(PLANAR_Y);
	unsigned char *maskp_savedY = maskpY;
    unsigned char *maskpU = UPLANE(mask);
	unsigned char *maskp_savedU = maskpU;
    unsigned char *maskpV = VPLANE(mask);
	unsigned char *maskp_savedV = maskpV;

    unsigned char *finalpY = YPLANE(final);//->GetWritePtr(PLANAR_Y);
	unsigned char *finalp_savedY = finalpY;
    unsigned char *finalpU = UPLANE(final);
	unsigned char *finalp_savedU = finalpU;
    unsigned char *finalpV = VPLANE(final);
	unsigned char *finalp_savedV = finalpV;

    unsigned char *finalp2Y = YPLANE(final2); //->GetWritePtr(PLANAR_Y);
	unsigned char *finalp2_savedY = finalp2Y;
    unsigned char *finalp2U = UPLANE(final2);
	unsigned char *finalp2_savedU = finalp2U;
    unsigned char *finalp2V = VPLANE(final2);
	unsigned char *finalp2_savedV = finalp2V;

    int row_sizeY = _info.width;//blur->GetRowSize(PLANAR_Y);
    int row_sizeUV = _info.width>>1;//blur->GetRowSize(PLANAR_U);
    int heightY = _info.height;//blur->GetHeight(PLANAR_Y);
    int heightUV = _info.height>>1;//blur->GetHeight(PLANAR_U);
	int y, reps;

	/* Create the detail mask. */
	EdgeMaskYV12(srcpY, blurpY, workpY, maskpY, row_sizeY, heightY, src_pitchY, blur_pitchY);
	EdgeMaskYV12(srcpU, blurpU, workpU, maskpU, row_sizeUV, heightUV, src_pitchUV, blur_pitchUV);
	EdgeMaskYV12(srcpV, blurpV, workpV, maskpV, row_sizeUV, heightUV, src_pitchUV, blur_pitchUV);

	if (_param->showmask == true)
	{
		if (show == true)
		{
			char buf[80];
			//env->MakeWritable(&mask);
			sprintf(buf, "0.2 beta");
			DrawString(mask, 0, 0, buf);
			sprintf(buf, "From Donald Graft");
			DrawString(mask, 0, 1, buf);
		}
		//return mask;
		memcpy(data->data,mask->data,(_info.width*_info.height*3)>>1);
		data->copyInfo(src);
		vidCache->unlockAll();
		return 1;
	}

	/* Fix up output frame borders. */
	srcpY = srcp_savedY;
	finalpY = finalp_savedY;
	finalp2Y = finalp2_savedY;
	memcpy(finalpY, srcpY, row_sizeY);
	memcpy(finalpY + (heightY-1)*blur_pitchY, srcpY + (heightY-1)*blur_pitchY, row_sizeY);
	memcpy(finalp2Y, srcpY, row_sizeY);
	memcpy(finalp2Y + (heightY-1)*blur_pitchY, srcpY + (heightY-1)*blur_pitchY, row_sizeY);
	for (y = 0; y < heightY; y++)
	{
		finalpY[0] = finalp2Y[0] = srcpY[0];
		finalpY[row_sizeY-1] = finalp2Y[row_sizeY-1] = srcpY[row_sizeY-1];
		srcpY += src_pitchY;
		finalpY += blur_pitchY;
		finalp2Y += blur_pitchY;
	}
	srcpU = srcp_savedU;
	finalpU = finalp_savedU;
	finalp2U = finalp2_savedU;
	memcpy(finalpU, srcpU, row_sizeUV);
	memcpy(finalpU + (heightUV-1)*blur_pitchUV, srcpU + (heightUV-1)*blur_pitchUV, row_sizeUV);
	memcpy(finalp2U, srcpU, row_sizeUV);
	memcpy(finalp2U + (heightUV-1)*blur_pitchUV, srcpU + (heightUV-1)*blur_pitchUV, row_sizeUV);
	for (y = 0; y < heightUV; y++)
	{
		finalpU[0] = finalp2U[0] = srcpU[0];
		finalpU[row_sizeUV-1] = finalp2U[row_sizeUV-1] = srcpU[row_sizeUV-1];
		srcpU += src_pitchUV;
		finalpU += blur_pitchUV;
		finalp2U += blur_pitchUV;
	}
	srcpV = srcp_savedV;
	finalpV = finalp_savedV;
	finalp2V = finalp2_savedV;
	memcpy(finalpV, srcpV, row_sizeUV);
	memcpy(finalpV + (heightUV-1)*blur_pitchUV, srcpV + (heightUV-1)*blur_pitchUV, row_sizeUV);
	memcpy(finalp2V, srcpV, row_sizeUV);
	memcpy(finalp2V + (heightUV-1)*blur_pitchUV, srcpV + (heightUV-1)*blur_pitchUV, row_sizeUV);
	for (y = 0; y < heightUV; y++)
	{
		finalpV[0] = finalp2V[0] = srcpV[0];
		finalpV[row_sizeUV-1] = finalp2V[row_sizeUV-1] = srcpV[row_sizeUV-1];
		srcpV += src_pitchUV;
		finalpV += blur_pitchUV;
		finalp2V += blur_pitchUV;
	}

	/* Masked smoothing. */
	reps = 0;
	SmoothingPassYV12(srcp_savedY, maskp_savedY, workp_savedY, finalp_savedY, row_sizeY, heightY, src_pitchY, blur_pitchY);
//	env->BitBlt(finalp_savedU, blur_pitchUV, srcp_savedU, src_pitchUV, row_sizeUV, heightUV);
//	env->BitBlt(finalp_savedV, blur_pitchUV, srcp_savedV, src_pitchUV, row_sizeUV, heightUV);
	SmoothingPassYV12(srcp_savedU, maskp_savedU, workp_savedU, finalp_savedU, row_sizeUV, heightUV, src_pitchUV, blur_pitchUV);
	SmoothingPassYV12(srcp_savedV, maskp_savedV, workp_savedV, finalp_savedV, row_sizeUV, heightUV, src_pitchUV, blur_pitchUV);
	if (++reps >= _param->strength)
	{
		deliver = final;
		goto done;
	}
	while (1)
	{
		SmoothingPassYV12(finalp_savedY, maskp_savedY, workp_savedY, finalp2_savedY, row_sizeY, heightY, blur_pitchY, blur_pitchY);
		SmoothingPassYV12(finalp_savedU, maskp_savedU, workp_savedU, finalp2_savedU, row_sizeUV, heightUV, blur_pitchUV, blur_pitchUV);
		SmoothingPassYV12(finalp_savedV, maskp_savedV, workp_savedV, finalp2_savedV, row_sizeUV, heightUV, blur_pitchUV, blur_pitchUV);
		if (++reps >=  _param->strength)
		{
			deliver = final2;
			goto done;
		}
		SmoothingPassYV12(finalp2_savedY, maskp_savedY, workp_savedY, finalp_savedY, row_sizeY, heightY, blur_pitchY, blur_pitchY);
		SmoothingPassYV12(finalp2_savedU, maskp_savedU, workp_savedU, finalp_savedU, row_sizeUV, heightUV, blur_pitchUV, blur_pitchUV);
		SmoothingPassYV12(finalp2_savedV, maskp_savedV, workp_savedV, finalp_savedV, row_sizeUV, heightUV, blur_pitchUV, blur_pitchUV);
		if (++reps >=  _param->strength)
		{
			deliver = final;
			goto done;
		}
	}

done:
	if (show == true)
	{
		char buf[80];
		//env->MakeWritable(&deliver);
		sprintf(buf, "0.2beta");
		DrawString(deliver, 0, 0, buf);
		sprintf(buf, "Donald Graft");
		DrawString(deliver, 0, 1, buf);
	}
	memcpy(data->data,deliver->data,(_info.width*_info.height*3)>>1);
	//return(deliver);
	data->copyInfo(src);
	vidCache->unlockAll();
	return 1;
}


#ifdef ADM_CPU_ALTIVEC
#define vecbyte vector unsigned char
#define vect16 vector unsigned short

#define LOAD_ALIGN(dest,src) \
		dest = (vect16)vec_ld(0, src); 

//______________________
void Blur_Altivec(uint8_t *in, uint8_t *out, uint32_t w, uint32_t h)
{
uint8_t *srcp,*srcpn,*srcpp;
uint8_t *workp;
uint32_t x,y;
int16_t  v16[8];

    vect16 pp,pc,pn,res,res2;
	vect16 rp,rc,rn,resl;
	vect16 zero,deux;
	
	uint32_t off;
	vector unsigned char MSQ,mask;
	zero=vec_splat_u16(0);
	deux=vec_splat_u16(2);

	srcpp = in;
	srcp  = srcpp + w;
	srcpn = srcp + w;
	workp = out + w;
	for (y = 1; y < h - 1; y++)
	{
		for (x = 0; x < (w>>4); x++)
		{
			off=x<<4;
			LOAD_ALIGN(pp,srcpp+off);
			LOAD_ALIGN(pc,srcp+off);
			LOAD_ALIGN(pn,srcpn+off);
			
			aprintf("sn %vd\n",pn); 
			aprintf("sp %vd\n",pp);
			aprintf("sc %vd\n",pc);
		
			
			rp=(vect16)vec_mergel( (vecbyte)zero,(vecbyte)pp);
			rn=(vect16)vec_mergel( (vecbyte)zero,(vecbyte)pn);
			rc=(vect16)vec_mergel( (vecbyte)zero,(vecbyte)pc);
			
			aprintf("rn %vd\n",rn); 
			aprintf("rp %vd\n",rp);
			aprintf("rc %vd\n",rc);
		
			res=vec_add(rp,rc);
			res2=vec_add(rc,rn);
			
			aprintf("re %vd\n",res);
			aprintf("r2 %vd\n",res2);
			
			res=vec_add(res,res2);
			
			resl=vec_sr(res,deux);
			aprintf("rS %vd\n",res);
			aprintf("r2 %vd\n",res2);
			aprintf("rl %vd\n",resl);
									
			pp=(vect16)vec_mergeh( (vecbyte)zero,(vecbyte)pp);
			pn=(vect16)vec_mergeh( (vecbyte)zero,(vecbyte)pn);
			pc=(vect16)vec_mergeh( (vecbyte)zero,(vecbyte)pc);
			
			res=vec_add(pp,pc);
			res2=vec_add(pc,pn);
			res=vec_add(res,res2);
			res=vec_sr(res,deux);
			
			aprintf("rH %vd\n",res);
			
			res=(vect16)vec_pack(res,resl);
			aprintf("rF %vd\n",res);
			
			vec_st((vecbyte)res,0,(vector unsigned char *)(workp+off));
			
		}
		srcpp += w;
        srcp += w;
        srcpn += w;
        workp += w;
    }
	// Do it at the end as it may have been scratched
	// due to 16 byte alignment
	memcpy(out, in, w);
	memcpy(out + (h-1)*w, in + (h-1)*w, w);
	
}
#endif

#ifdef ADM_CPU_X86
//______________________
void Blur_MMX(uint8_t *in, uint8_t *out, uint32_t w, uint32_t h)
{
uint8_t *srcp,*srcpn,*srcpp;
uint8_t *workp;
uint32_t x,y;
uint32_t off;
	
	srcpp = in;
	srcp  = srcpp + w;
	srcpn = srcp + w;
	workp = out + w;
	for (y = 1; y < h - 1; y++)
	{
		for (x =  (w>>3);x>0; x--)
		{
			off=x<<3;
			
			__asm__(
			ADM_ALIGN16
			"pxor  %%mm7,%%mm7\n"
			"movq  (%0),%%mm0\n"
			"movq  %%mm0,%%mm6\n"
			"punpckhbw %%mm7,%%mm0\n" // High part extended to 16 bits
			"punpcklbw %%mm7,%%mm6\n" // low part ditto
			
			"movq  (%1),%%mm1\n"
			"movq  %%mm1,%%mm5\n"
			"punpckhbw %%mm7,%%mm1\n"
			"punpcklbw %%mm7,%%mm5\n"
			
			"movq  (%2),%%mm2\n"
			"movq  %%mm2,%%mm4\n"
			"punpckhbw %%mm7,%%mm2\n"
			"punpcklbw %%mm7,%%mm4\n"
			
			"paddw %%mm1,%%mm0\n"
			"paddw %%mm5,%%mm6\n"
			
			"paddw %%mm1,%%mm2\n"
			"paddw %%mm5,%%mm4\n"
			
			"paddw %%mm0,%%mm2\n"
			"paddw %%mm6,%%mm4\n"
			"psrlw $2, %%mm4\n"
			"psrlw $2, %%mm2\n"
			"packuswb %%mm2,%%mm4\n"
			"movq %%mm4,(%3)\n" //
			
			: : "r" (srcpn+off),
			   "r" (srcp+off), "r" (srcpp+off), "r" (workp+off)
			);
			
		}
		workp[0]=srcp[0];
		workp[w-1]=srcp[w-1];
		srcpp += w;
        	srcp += w;
        	srcpn += w;
        	workp += w;
    }
	// Do it at the end as it may have been scratched
	// due to 16 byte alignment
	memcpy(out, in, w);
	memcpy(out + (h-1)*w, in + (h-1)*w, w);
	__asm__ ("emms\n" : :);
	
}
#endif

//______________________
void Blur_C(uint8_t *in, uint8_t *out, uint32_t w, uint32_t h)
{
uint8_t *srcp,*srcpn,*srcpp;
uint8_t *workp;
uint32_t x,y;
	srcpp = in;
	srcp  = srcpp + w;
	srcpn = srcp + w;
	workp = out + w;
	memcpy(out, in, w);
	memcpy(out + (h-1)*w, in + (h-1)*w, w);
	for (y = 1; y < h - 1; y++)
	{
		workp[0] = srcp[0];
		workp[w-1] = srcp[w-1];
        for (x = 1; x < w - 1; x++)
		{
			workp[x] = (srcpp[x] + srcp[x] + srcpn[x]) /3;
		}
        srcpp += w;
        srcp += w;
        srcpn += w;
        workp += w;
    }

}
//_______________________________
void  Msmooth::EdgeMaskYV12(const unsigned char *srcp, unsigned char *blurp, unsigned char *workp, unsigned char *maskp,
                        int row_size, int height, int src_pitch, int blur_pitch)
{
	const unsigned char *srcp_saved = srcp;
	const unsigned char *srcpp;
	const unsigned char *srcpn;

	unsigned char *blurp_saved = blurp;
	const unsigned char *blurpn;

	unsigned char *workp_saved = workp;
	unsigned char *workpp;
	unsigned char *workpn;

	unsigned char *maskp_saved = maskp;
	int x, y;
	int y1, y2, y3, y4;

	/* Blur the source image prior to detail detection. */
#ifdef ADM_CPU_X86
		//printf("MMX\n");
	if(CpuCaps::hasMMX())
	{
		Blur_MMX((uint8_t *)srcp,(uint8_t *)workp,row_size,height);
		Blur_MMX((uint8_t *)workp,(uint8_t *)blurp,row_size,height);
	}
	else
	#endif
	#ifdef ADM_CPU_ALTIVEC
	#define ISALIGNED(x) (!( ((long long)x)&15 ))
		if( ISALIGNED(srcp) && ISALIGNED(blurp) && ISALIGNED(workp) && ISALIGNED(maskp) && !(src_pitch&15))
		{
			Blur_Altivec((uint8_t *)srcp,(uint8_t *)workp,row_size,height);
			Blur_Altivec((uint8_t *)workp,(uint8_t *)blurp,row_size,height);
		}
		else
	#endif
	{
		Blur_C((uint8_t *)srcp,(uint8_t *)workp,row_size,height);
		Blur_C((uint8_t *)workp,(uint8_t *)blurp,row_size,height);
	}
	/* Diagonal detail detection. */
	blurp = (unsigned char *) workp_saved;
	blurpn = blurp + blur_pitch;
	maskp = maskp_saved;
	y1 = blurp[0];
	y3 = blurpn[0];
	for (y = 0; y < height - 1; y++) 
	{
		for (x = 0; x < row_size - 1; x++)
		{
			y2 = blurp[x+1];
			y4 = blurpn[x+1];
			if ((abs(y1 - y4) >= _param->threshold) || (abs(y2 - y3) >= _param->threshold))
			{
				maskp[x] = 0xff;
			}
			else
			{
				maskp[x] = 0x0;
			}
			y1 = y2; y3 = y4;
		}
		maskp += blur_pitch;
		blurp += blur_pitch;
		blurpn += blur_pitch;
	}

	/* If set for high quality, also do horizontal and vertical
	   detail detection. */
	if ( _param->highq == true)
	{
		/* Msmooth convolve vertical. */
		for (x = 0; x < row_size; x++)
		{
 			blurp = blurp_saved;
			blurpn = blurp + blur_pitch;
			maskp = maskp_saved;
			y1 = blurp[x];
			for (y = 0; y < height - 1; y++)
			{
				y2 = blurpn[x];
				if (abs(y1 - y2) >= _param->threshold)
				{
					maskp[x] = 0xff;
				}
				y1 = y2;
				maskp += blur_pitch;
				blurp += blur_pitch;
				blurpn += blur_pitch;
			}
		}

		/* Msmooth convolve horizontal. */
		blurp = blurp_saved;
		maskp = maskp_saved;
		for (y = 0; y < height; y++)
		{
			y1 = blurp[0];
			for (x = 0; x < row_size - 1; x++)
			{
				y2 = blurp[x+1];
				if (abs(y1 - y2) >= _param->threshold)
				{
					maskp[x] = 0xff;
				}
				y1 = y2;
			}
			maskp += blur_pitch;
			blurp += blur_pitch;
		}

	}
	/* Fix up detail map borders. */
	maskp = maskp_saved;
	memset(maskp, 0xff, row_size);
	memset(maskp + (height-1)*blur_pitch, 0xff, row_size);
	for (y = 0; y < height; y++)
	{
//		*((unsigned int *)(&maskp[0])) = 0xffffffff;
//		*((unsigned int *)(&maskp[row_size-1])) = 0xffffffff;
		maskp[0] = 0xff;
		maskp[row_size-1] = 0xff;
		maskp += blur_pitch;
	}
}

void  Msmooth::SmoothingPassYV12(const unsigned char *srcp, unsigned char *maskp,
									  unsigned char *workp, unsigned char *finalp,
									  int row_size, int height, int spitch, int dpitch)
{
	const unsigned char *srcp_saved = srcp;
	const unsigned char *srcpp;
	const unsigned char *srcpn;

	unsigned char *workp_saved = workp;
	unsigned char *workpp;
	unsigned char *workpn;

	unsigned char *maskp_saved = maskp;
	unsigned char *maskpp;
	unsigned char *maskpn;

	unsigned char *finalp_saved = finalp;

	int x, y, ysum;

	srcpp = srcp_saved;
 	srcp = srcpp + spitch;
	srcpn = srcp + spitch;
	maskpp = maskp_saved;
 	maskp = maskpp + dpitch;
	maskpn = maskp + dpitch;
	workp = workp_saved + dpitch;
	memcpy(workp_saved, srcp_saved, row_size);
	memcpy(workp_saved + (height-1)*dpitch, srcp_saved + (height-1)*spitch, row_size);
	for (y = 1; y < height - 1; y++)
	{
		workp[0] = srcp[0];
		workp[row_size-1] = srcp[row_size-1];
        for (x = 1; x < row_size - 1; x++)
		{
			int count = 1;

			if (!maskp[x])
			{
				ysum = srcp[x];
				if (!maskpp[x])
				{
					ysum += srcpp[x];
					count++;
				}
				if (!maskpn[x])
				{
					ysum += srcpn[x];
					count++;
				}
				workp[x] = ysum / count;
			}
		}
        srcpp += spitch;
        srcp += spitch;
        srcpn += spitch;
 		maskpp += dpitch;
		maskp += dpitch;
		maskpn += dpitch;
        workp += dpitch;
    }
	srcpp = srcp_saved;
 	srcp = srcpp + spitch;
	srcpn = srcp + spitch;
	workpp = workp_saved;
 	workp = workpp + dpitch;
	workpn = workp + dpitch;
	maskpp = maskp_saved;
 	maskp = maskpp + dpitch;
	maskpn = maskp + dpitch;
	finalp = finalp_saved + dpitch;
	for (y = 1; y < height - 1; y++)
	{
        for (x = 1; x < row_size - 1; x++)
		{
			int count = 1;

			if (!maskp[x])
			{
				ysum = workp[x];
				if (!maskp[x-1])
				{
					ysum += workp[x-1];
					count++;
				}
				if (!maskp[x+1])
				{
					ysum += workp[x+1];
					count++;
				}
				finalp[x] = ysum / count;
			}
			else
			{
				finalp[x] = srcp[x];
			}
		}
        srcpp += spitch;
        srcp += spitch;
        srcpn += spitch;
        workpp += dpitch;
        workp += dpitch;
        workpn += dpitch;
 		maskpp += dpitch;
		maskp += dpitch;
		maskpn += dpitch;
        finalp += dpitch;
    }
}

