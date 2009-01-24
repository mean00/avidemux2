/***************************************************************************
                          ADM_vidMSharpen  -  description
                             -------------------
    
    email                : fixounet@free.fr

    Port of Donal Graft Msharpen which is (c) Donald Graft
    http://www.neuron2.net
    http://puschpull.org/avisynth/decomb_reference_manual.html

        It is a bit less efficient as we do hz & vz blur separately
        The formula has been changed a bit from 1 1 1 to 1 2 1
        for speed aspect & MMX  
        Mean

 ***************************************************************************/
/*
	Msharpen plugin for Avisynth -- performs detail-preserving smoothing.

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


#include "ADM_vidMSharpen_param.h"
#include "DIA_factory.h"

#define aprintf(...) {}

class Msharpen : public AVDMGenericVideoStream
{
private:
	MSHARPEN_PARAM	*_param;
	VideoCache	*vidCache;
        ADMImage        *blurrImg,*work;

        uint32_t        invstrength;

        void detect_edges(ADMImage *src, ADMImage *dst, unsigned char *dstp, int plane);
        void blur_plane(ADMImage *src, ADMImage *blur, unsigned char *blurp, int plane) ;
        void detect_edges_HiQ(ADMImage *src, ADMImage *dst, unsigned char *dstp, int plane);
        void apply_filter(ADMImage *src,ADMImage *blur, ADMImage *dst, unsigned char *dstp, int plane) ;
public:    

			Msharpen(AVDMGenericVideoStream *in,CONFcouple *couples)   ;
			~Msharpen();


				 
	char 		*printConf( void );
	uint8_t 	configure(AVDMGenericVideoStream *in);
	uint8_t		getCoupledConf( CONFcouple **couples);
	uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len, ADMImage *data,uint32_t *flags);
};


static FILTER_PARAM msharpenParam={4,{"mask", "highq","threshold", "strength"}};


//REGISTERX(VF_SHARPNESS, "msharpen",QT_TR_NOOP("MSharpen"),
//QT_TR_NOOP("Sharpen edges without amplifying noise. By Donald Graft."),VF_MSHARPEN,1,msharpen_create,msharpen_script);
//********** Register chunk ************

VF_DEFINE_FILTER(Msharpen,msharpenParam,
    msharpen,
                QT_TR_NOOP("MSharpen"),
                1,
                VF_SHARPNESS,
                QT_TR_NOOP("Sharpen edges without amplifying noise. By Donald Graft."));
//********** Register chunk ************

//_______________________________________________

Msharpen::Msharpen(AVDMGenericVideoStream *in,CONFcouple *couples)

{
	_in=in;		
   	memcpy(&_info,_in->getInfo(),sizeof(_info));    
  	_info.encoding=1;
	_uncompressed=NULL;		
  	_info.encoding=1;


	_param=new MSHARPEN_PARAM;
	//
	if(couples)
		{
			GET(mask);
			GET(highq);
			GET(strength);	
			GET(threshold);	
		}
		else // Default
  		{
			_param->mask=0;       // Show mask
			_param->highq=1;
			_param->strength=100;	
			_param->threshold=15;	
		}
                
		invstrength=255-_param->strength;	
		blurrImg=new ADMImage(_info.width,_info.height);
                work=new ADMImage(_info.width,_info.height);
    		vidCache=new VideoCache(5,in);
}
//________________________________________________________
uint8_t	Msharpen::getCoupledConf( CONFcouple **couples)
{
	*couples=NULL;
	*couples=new CONFcouple(4);
#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
		CSET(mask);
		CSET(highq);
		CSET(strength);	
		CSET(threshold);	
	return 1;
}
//________________________________________________________
Msharpen::~Msharpen(void)
{

	if(vidCache) delete vidCache;
	if(_param) delete _param;
	if(blurrImg) delete blurrImg;
        if(work) delete work;
	vidCache=NULL;
	_param=NULL;
	blurrImg=NULL;
        work=NULL;
}
//________________________________________________________
uint8_t Msharpen::configure(AVDMGenericVideoStream *in)
{
uint8_t r=0;
	_in=in;
	ADM_assert(_param);

#define PX(x) &(_param->x)
  
        
    diaElemToggle    mask(PX(mask),QT_TR_NOOP("_Mask"));
    diaElemToggle    highq(PX(highq),QT_TR_NOOP("_High Q"));
    
    diaElemUInteger   threshold(PX(threshold),QT_TR_NOOP("_Threshold:"),1,255);
    diaElemUInteger   strength(PX(strength),QT_TR_NOOP("_Strength:"),1,255);
    
    
  diaElem *elems[4]={&mask,&highq,&threshold,&strength};

  if(diaFactoryRun(QT_TR_NOOP("MSharpen"),4,elems))
  {
         invstrength=255-_param->strength;
         return 1;
  }
  return 0;
}

//________________________________________________________
char *Msharpen::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Donald Graft Msharpen Strength:%d Threshold:%d",_param->strength,_param->threshold);
        
}
	
//________________________________________________________
uint8_t Msharpen::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
				ADMImage *data,uint32_t *flags)
{
ADMImage *src,*blur,*dst;
/*
	PVideoFrame src = child->GetFrame(n, env);
	PVideoFrame blur = env->NewVideoFrame(vi);
	PVideoFrame dst = env->NewVideoFrame(vi);
*/
unsigned char *blurp;
unsigned char *dstp;
	dst=data;
	if(frame>=_info.nb_frames) return 0;
	src=vidCache->getImage(frame);
	blur=blurrImg;
	{
		for (int i=0;i<3;i++)
		{
		
			//unsigned char *blurp = blur->GetWritePtr(plane);
			switch(i)
			{
				case 0:blurp=YPLANE(blur);break;
				case 1:blurp=UPLANE(blur);break;
				case 2:blurp=VPLANE(blur);break;
			}
			blur_plane(src, blur, blurp ,i);

		//	unsigned char *dstp = dst->GetWritePtr(plane);
			switch(i)
			{
				case 0:dstp=YPLANE(dst);break;
				case 1:dstp=UPLANE(dst);break;
				case 2:dstp=VPLANE(dst);break;
			}

			detect_edges(blur, dst, dstp, i);
			if (_param->highq == true)
				detect_edges_HiQ(blur, dst, dstp, i);

			if (!_param->mask) apply_filter(src, blur, dst, dstp, i);
		}
	}

	vidCache->unlockAll();
	return 1;
}

/**********************************
 * MMX / ISSE by Klaus Post and Milan Cutka.
 *
 * Delivers same result as YUY2, except planes
 *  are processed independently.
 *********************************/

void Msharpen::blur_plane(ADMImage *src, ADMImage *blur, unsigned char *blurp, int plane) 
{
/*
  uint64_t mask1 = 0x00001C711C711C71LL;
  uint64_t mask2 = 0x1C711C711C710000LL;
  uint64_t mask3 = 0x0000200000002000LL;
  uint64_t mask4 = 0x0000000000ff0000LL;

  const unsigned char *srcp = src->GetReadPtr(plane);
	const unsigned char *srcp_saved = srcp;
	unsigned char *blurp_saved = blurp;
  int src_pitch = src->GetPitch(plane);
  int blur_pitch = blur->GetPitch(plane);
  int h = src->GetHeight(plane);
  int w = src->GetRowSize(plane);
*/
const unsigned char *srcp,*srcpn,*srcpp; 
const unsigned char *srcp_saved ;
unsigned char *wk,*wk_saved;
unsigned char *blurp_saved ;
int src_pitch;
int blur_pitch;
int h;
int w ;
int wh ,ww,hh;
 	blurp_saved=blurp;
	switch(plane)
		{
			case 0:
				srcp_saved=srcp=YPLANE(src);        
                                wk_saved=wk=YPLANE(work);
				ww=_info.width;
				hh=_info.height;
				break;
			case 1:
				srcp_saved=srcp=UPLANE(src);
                                wk_saved=wk=UPLANE(work);
				ww=_info.width>>1;
				hh=_info.height>>1;
				break;
			case 2:
				srcp_saved=srcp=VPLANE(src);
                                wk_saved=wk=VPLANE(work);
				ww=_info.width>>1;
				hh=_info.height>>1;
				break;
		}
		src_pitch=ww;
		blur_pitch=ww;
		w=ww;
		h=hh;
 
  
  wk+=blur_pitch;
  srcpp=srcp;
  srcp+=src_pitch,
  srcpn=srcp+src_pitch;
  int val;

  // Vertical only for now      
#ifdef ADM_CPU_X86
  if(CpuCaps::hasMMX())
  {
  int off;
#ifdef GCC_2_95_X
        __asm__(
                        ADM_ALIGN16
                        "pxor  %mm7,%mm7\n"
                : : );
#else
        __asm__(
                        ADM_ALIGN16
                        "pxor  %%mm7,%%mm7\n"
                : : );
#endif
                        
  for (int y=1; y<h-1 ;y++) 
  {               
        for (int x =  (w>>3);x>0; x--)
                {
                        off=x<<3;                        
                        __asm__(
                        ADM_ALIGN16
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
                           "r" (srcp+off), "r" (srcpp+off), "r" (wk+off)
                        );
                }                        
        srcp+=src_pitch;
        srcpp+=src_pitch;
        srcpn+=src_pitch;
        wk+=src_pitch;     
  }
  __asm__("emms\n");
 }
 else
#endif      
  {
  for (int y=1; y<h-1 ;y++) 
  {       

        for(int x=0;x<w;x++)
        {
                val=srcp[x]+srcpn[x]+srcpp[x]+srcp[x];
                wk[x]=(val)>>2;
        } 
        srcp+=src_pitch;
        srcpp+=src_pitch;
        srcpn+=src_pitch;
        wk+=src_pitch;     
  }
  }
  //************ Horizontal****************
  blurp=blurp_saved;
  srcp=wk_saved;
  for (int y=1; y<h-1 ;y++) 
  {         
        for(int x=1;x<w-1;x++)
        {
                val=srcp[x-1]+srcp[x]+srcp[x+1]+srcp[x];
                blurp[x]=val>>2;
        } 
        srcp+=src_pitch;
        srcpp+=src_pitch;
        srcpn+=src_pitch;
        blurp+=src_pitch;     
  }
  //******************
        /* Fix up blur frame borders. */
        srcp = srcp_saved;
        blurp = blurp_saved;
        memcpy(blurp, srcp, w);
        memcpy(blurp + (h-1)*blur_pitch, srcp + (h-1)*src_pitch, w);
        for (int y = 0; y < h; y++)
        {
                blurp[0] = srcp[0];
                blurp[w-1] = srcp[w-1];
                srcp += src_pitch;
                blurp += blur_pitch;
        }

}
#if 0
    __asm {
      mov esi, srcp  // esi=srcpp
      mov ecx, blurp
      mov ebx, src_pitch
      mov edi, esi 
      mov edx, esi 
      add edi, ebx // edi=srcp+pitch=srcp
      add edx, ebx 
      add edx, ebx  //edx=srcp+2*pitch=srcpn
      xor eax, eax
      movq mm7,mask1 // mm7 mask1
      movq mm6,mask2 // mm6 mask2
     align 16
xloop:
      movd mm0,[esi+eax]  //srcpp[x}
      pxor mm5,mm5
       movd mm1,[edi+eax] //srcp[x}
      movd mm2,[edx+eax]  // srcpn   
      punpcklbw mm0,mm5  // mm0 low srcpp
      punpcklbw mm1,mm5   // mm1 low srcp
      movq mm3, mm0
      punpcklbw mm2,mm5   // mm2 low srcn
      movq mm4, mm1
      movq mm5, mm2
      paddw mm0,mm1     // srcp+srcpp
      paddw mm3,mm4   // 2*srcp+srcpp
      paddw mm0,mm2    // 2*srcp+srcpp+srcn
      paddw mm3,mm5   // idem
      pmaddwd mm0,mm7
       movq mm5,[mask3]
      pmaddwd mm3,mm6
       pshufw mm1,mm0, 11101110b  // Move upper to lower
      pshufw mm4,mm3, 11101110b   
       paddd mm0,mm1
      paddd mm3,mm4
       paddd mm0, mm5
      paddd mm3, mm5
       psrld mm0,16
      pand mm3,[mask4]
      psrld mm3,8
      por mm0,mm3
      movd [eax+ecx],mm0  // blurp[i]
      add eax,2
      cmp eax,[w]
      jle xloop;
    }
    srcp += src_pitch;
    blurp += blur_pitch;
//      __asm emms;
#endif
 

//***************************************************
void Msharpen::detect_edges(ADMImage *src, ADMImage *dst, unsigned char *dstp, int plane) 
{
  static uint64_t m255=0xffffffffffffffffLL;
  static uint64_t threshold64;
  int ww,hh;

  threshold64=_param->threshold;
  if(!threshold64) threshold64=1;
  threshold64=threshold64+(threshold64<<8)+(threshold64<<16)+(threshold64<<24)+(threshold64<<32)+(threshold64<<40)+(threshold64<<48)+(threshold64<<56);
/*
  const unsigned char *srcp = src->GetReadPtr(plane);
  int src_pitch = src->GetPitch(plane);
  int dst_pitch = dst->GetPitch(plane);
  const unsigned char *srcpn = srcp+src_pitch;
  int h = src->GetHeight(plane);
  int w = src->GetRowSize(plane);
*/
  const unsigned char *srcp,*srcp_saved; 
  int src_pitch ;
  int dst_pitch ; 
  const unsigned char *srcpn; 
  int h ;
  int w ;
  unsigned char *dstp_saved = dstp;

  switch(plane)
	{
		case 0: srcp=YPLANE(src);
			ww=_info.width;
			hh=_info.height;
			break;
		case 2:
		case 1:
			if(plane==1) 	
				srcp=UPLANE(src);
			else
				srcp=VPLANE(src);
			ww=_info.width>>1;
			hh=_info.height>>1;
			break;
		default:
			ADM_assert(0);
	}
	src_pitch=ww;
	dst_pitch=ww;
	w=ww;
	h=hh;
	srcpn=srcp+src_pitch;
	srcp_saved=srcp;

 int p,n,c;
 for (int y=0;y<h-1;y++)
  {
   for(int xx=1;xx<w-1;xx++)
        {
                p=srcp[xx+1];
                n=srcpn[xx+1];
                c=srcpn[xx-1];
                
                if(abs(n-p)>_param->threshold || abs(c-p)>_param->threshold) dstp[xx+1]=0xff;
                                else dstp[xx+1]=0;
                
        }
   srcp+=src_pitch;
   srcpn+=src_pitch;
   dstp+=dst_pitch;
  }
  if (_param->mask) {
    dstp=dstp_saved;
    memset(dstp_saved+(h-1)*dst_pitch,0,w);  // Not used, if not returning mask
    for (int y=0;y<h;dstp+=dst_pitch,y++) {
      dstp[0]=0;
      dstp[1]=0;
      dstp[w-1]=0;
      dstp[w-2]=0;
    }
  }
}
#if 0
unsigned char *dstpend=dstp+w-1;
   __asm 
    {
     mov esi,[srcp]
     inc esi
     mov ecx,[srcpn]
     inc ecx
     mov edx,[srcpn]
     dec edx
     mov edi,[dstp]
     inc edi
     mov eax,[dstpend]
     movq mm6,[threshold64]
     pxor mm5,mm5
     movq mm4,[m255]
     align 16
    diag1:
     movq mm2,[ecx]
      movq mm1,[esi]
     movq mm3,[edx]

     //abs(mm1-mm2)
      movq mm7,mm1
     pminub mm1,mm2  //srcpn+1 - srcp+1
      pmaxub mm2,mm7
     psubusb mm2,mm1
      movq mm1,[esi]
     psubusb mm2,mm6
     
     //abs(mm1-mm3)
      movq mm7,mm1
     pminub mm1,mm3 // srcp+1 srcp-1
      pmaxub mm3,mm7
     psubusb mm3,mm1
     psubusb mm3,mm6

     por     mm2,mm3
     pcmpeqb mm2,mm5
     pxor    mm2,mm4
     movq    [edi],mm2
     
     add esi,8
     add ecx,8
     add edx,8
     add edi,8
     cmp edi,eax
     jl  diag1

   }
__asm emms;
#endif
//***************************************************
void Msharpen::detect_edges_HiQ(ADMImage *src, ADMImage *dst, unsigned char *dstp, int plane) 
{
  // Vertical detail detection
/*
  const unsigned char *srcp = src->GetReadPtr(plane);
	const unsigned char *srcp_saved = srcp;
  const unsigned char *srcpn;
	unsigned char *dstp_saved = dstp;
  int src_pitch = src->GetPitch(plane);
  int dst_pitch = dst->GetPitch(plane);
  int h = src->GetHeight(plane);
  int w = src->GetRowSize(plane);
*/

  int b1,b2;
  int ww,hh;
 const unsigned char *srcp,*srcp_saved; 
  int src_pitch ;
  int dst_pitch ; 
  const unsigned char *srcpn; 
  int h ;
  int w ;
  unsigned char *dstp_saved = dstp;

  switch(plane)
	{
		case 0: srcp=YPLANE(src);
			ww=_info.width;
			hh=_info.height;
			break;
		case 2:
		case 1:
			if(plane==1) 	
				srcp=UPLANE(src);
			else
				srcp=VPLANE(src);
			ww=_info.width>>1;
			hh=_info.height>>1;
			break;
		default:
			ADM_assert(0);
	}
	src_pitch=ww;
	dst_pitch=ww;
	w=ww;
	h=hh;
	srcpn=srcp+src_pitch;
	srcp_saved=srcp;

  for (int x=0;x<w;x++)
  {
    srcp=srcp_saved;
    srcpn=srcp+src_pitch;
    dstp=dstp_saved;
    b1=srcp[x];
    for (int y=0;y<h-1;dstp+=dst_pitch,srcp+=src_pitch,srcpn+=src_pitch,y++)
    {
      b2=srcpn[x];
      if (abs(b1-b2)>=_param->threshold)
        dstp[x]=255;
      b1=b2;
    }
  }
  
  // Horizontal detail detection
  srcp=srcp_saved;
  dstp=dstp_saved;
  for (int y=0;y<h;dstp+=dst_pitch,srcp+=src_pitch,y++)
  {
    b1=srcp[0]; //MEANX srcp[x]
    for (int x=0;x<w-1;x++)
    {
      b2=srcp[x+1];
      if (abs(b1-b2)>=_param->threshold)
        dstp[x]=255;
      b1=b2;
    }
  }
  // Fix up detail map borders
  dstp = dstp_saved;
  memset(dstp,0,w);
  memset(dstp+dst_pitch,0,w);
  memset(dstp+(h-2)*dst_pitch,0,w);
  memset(dstp+(h-1)*dst_pitch,0,w);
  for (int y=0;y<h;dstp+=dst_pitch,y++)
  {
    dstp[0]=0;
    dstp[1]=0;
    dstp[w-1]=0;
    dstp[w-2]=0;
  }
}
//***************************************************
void Msharpen::apply_filter(ADMImage *src,ADMImage *blur, ADMImage *dst, unsigned char *dstp, int plane) 
{
  // TODO: MMX / ISSE
/*
  const unsigned char *srcp = src->GetReadPtr(plane);
  const unsigned char *blurp = blur->GetReadPtr(plane);
  const unsigned char *srcp_saved = srcp;
  unsigned char *dstp_saved = dstp;
  const unsigned char *blurp_saved = blurp;
  int src_pitch = src->GetPitch(plane);
  int blur_pitch = blur->GetPitch(plane);
  int dst_pitch = dst->GetPitch(plane);
  int h = src->GetHeight(plane);
  int w = src->GetRowSize(plane);
 */
  const unsigned char *srcp ;
  const unsigned char *blurp ;
  const unsigned char *srcp_saved; 
  unsigned char *dstp_saved ;
  const unsigned char *blurp_saved ;
  int src_pitch;
  int blur_pitch;
  int dst_pitch;
  int h;
  int w;
  int ww,hh; 

  switch(plane)
	{
		case 0:
			 srcp=YPLANE(src);
			 blurp=YPLANE(blur);
			 ww=_info.width;
			 hh=_info.height;
			 break;
		case 1:
		case 2:
			if(plane==1)
			{
			 srcp=UPLANE(src);
			 blurp=UPLANE(blur);
			}
			else
			{
			 srcp=VPLANE(src);
			 blurp=VPLANE(blur);
			}
			 ww=_info.width>>1;
			 hh=_info.height>>1;
			break;
	}

	w=ww;
	h=hh;
	blur_pitch=ww;
	dst_pitch=ww;
	src_pitch=ww;
	srcp_saved=srcp;
	blurp_saved=blurp;
	dstp_saved=dstp;
	
  memcpy(dstp,srcp,w);
  memcpy(dstp+(h-1)*dst_pitch,srcp+(h-1)*src_pitch,w);
  for (int y=0;y<h;srcp+=src_pitch,dstp+=dst_pitch,y++)
  {
    dstp[0]=srcp[0];
    dstp[w-1]=srcp[w-1];
  }
  
  // Now sharpen the edge areas and we're done
  srcp=srcp_saved+src_pitch;
  dstp=dstp_saved+dst_pitch;
  blurp=blurp+dst_pitch;
  int b4;
  for (int y=1;y<h-1;srcp+=src_pitch,dstp+=dst_pitch,blurp+=blur_pitch,y++)
  {
    for (int x=1;x<w-1;)
    {
      if (*(int*)(dstp+x)==0) 
      {
        *(int*)(dstp+x)=*(int*)(srcp+x);
        x+=4;
        continue;
      }
      if (dstp[x])
      {                                     
        b4=4*int(srcp[x])-3*int(blurp[x]);
        if (b4<0) b4=0; else if (b4>255) b4=255;
        dstp[x]=(_param->strength*b4+invstrength*srcp[x])>>8;
      }
      else
        dstp[x]=srcp[x];
      x++; 
    }  
    dstp[w-1]=srcp[w-1]; 
  }   
}
//***************************************************





