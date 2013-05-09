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
#include "DIA_factory.h"
#include "ADM_coreVideoFilter.h"
#include "msharpen.h"
#include "msharpen_desc.cpp"
/**
    \class Msharpen
*/
class Msharpen : public ADM_coreVideoFilterCached
{
private:
        msharpen	_param;
        ADMImage        *blurrImg,*work;

        uint32_t        invstrength;

                void    detect_edges(ADMImage *src, ADMImage *dst, int plane);
                void    blur_plane(ADMImage *src, ADMImage *blur, int plane) ;
                void    detect_edges_HiQ(ADMImage *src, ADMImage *dst, int plane);
                void    apply_filter(ADMImage *src,ADMImage *blur, ADMImage *dst,int plane) ;
public:    

                            Msharpen(ADM_coreVideoFilter *in,CONFcouple *couples)   ;
                            ~Msharpen();

       virtual const char  *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
       virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual void         setCoupledConf(CONFcouple *couples);
       virtual bool         configure(void) ;                 /// Start graphical user interface        
       
};
//********** Register chunk ************

// DECLARE FILTER 

DECLARE_VIDEO_FILTER(   Msharpen,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_SHARPNESS,            // Category
                        "msharpen",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("msharpen","Msharpen"),            // Display name
                        QT_TRANSLATE_NOOP("msharpen","Sharpen edges without amplifying noise. By Donald Graft.") // Description
                    );

/**
    \fn ctor
*/
Msharpen::Msharpen(ADM_coreVideoFilter *in,CONFcouple *couples)  
    : ADM_coreVideoFilterCached(5,in,couples)
{
    if(!couples || !ADM_paramLoad(couples,msharpen_param,&_param))
    {
        _param.mask=0;       // Show mask
        _param.highq=1;
        _param.strength=100;	
        _param.threshold=15;	
    }
                
    invstrength=255-_param.strength;	
    blurrImg=new ADMImageDefault(info.width,info.height);
    work=new ADMImageDefault(info.width,info.height);
    
}
/**
    \fn getCoupledConf
*/
bool Msharpen::getCoupledConf( CONFcouple **couples)
{
    return ADM_paramSave(couples, msharpen_param,&_param);
}

void Msharpen::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, msharpen_param, &_param);
}
/**
    \fn dtor
*/
Msharpen::~Msharpen(void)
{
    if(blurrImg) delete blurrImg;
    if(work) delete work;
	
    blurrImg=NULL;
    work=NULL;
}
/**
    \fn configure
*/
bool Msharpen::configure(void)
{
uint8_t r=0;

#define PX(x) &(_param.x)
  
        
    diaElemToggle    mask(PX(mask),QT_TRANSLATE_NOOP("msharpen","_Mask"));
    diaElemToggle    highq(PX(highq),QT_TRANSLATE_NOOP("msharpen","_High Q"));
    
    diaElemUInteger   threshold(PX(threshold),QT_TRANSLATE_NOOP("msharpen","_Threshold:"),1,255);
    diaElemUInteger   strength(PX(strength),QT_TRANSLATE_NOOP("msharpen","_Strength:"),1,255);
    
    
  diaElem *elems[4]={&mask,&highq,&threshold,&strength};

  if(diaFactoryRun(QT_TRANSLATE_NOOP("msharpen","MSharpen"),4,elems))
  {
         invstrength=255-_param.strength;
         return 1;
  }
  return 0;
}
/**
    \fn getConfiguration
*/
const char *Msharpen::getConfiguration(void)
{
   static char conf[80];
    conf[0]=0;
    snprintf(conf,80," Msharpen Strength:%d Threshold:%d",_param.strength,_param.threshold);
    return conf;
}
	
/**
    \fn getNextFrame
*/
bool         Msharpen::getNextFrame(uint32_t *fn,ADMImage *image)
{
ADMImage *src,*blur,*dst;

    src=vidCache->getImage(nextFrame);
    if(!src)
        return false; // EOF
    
    blur=blurrImg;
    dst=image;

    dst->Pts=src->Pts;
	
    for (int i=0;i<3;i++)
    {
            
            blur_plane(src, blur, i);
            detect_edges(blur, dst,  i);
            if (_param.highq == true)
                detect_edges_HiQ(blur, dst,  i);
            if (!_param.mask) 
                apply_filter(src, blur, dst,  i);
    }

    *fn=nextFrame;
    nextFrame++;
    vidCache->unlockAll();
    return true;
}

/**********************************
 * MMX / ISSE by Klaus Post and Milan Cutka.
 *
 * Delivers same result as YUY2, except planes
 *  are processed independently.
 *********************************/

void Msharpen::blur_plane(ADMImage *src, ADMImage *blur, int plane) 
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
unsigned char *blurp_saved,*blurp ;
int src_pitch;
int blur_pitch;
int work_pitch;
int h;
int w ;
int wh ,ww,hh;
 	blurp_saved=blurp=blur->GetWritePtr((ADM_PLANE)plane);
        srcp_saved=srcp=src->GetReadPtr((ADM_PLANE)plane);
        wk_saved=wk=work->GetWritePtr((ADM_PLANE)plane);
        
        ww=src->GetWidth((ADM_PLANE)plane);
        hh=src->GetHeight((ADM_PLANE)plane);
        
        src_pitch=src->GetPitch((ADM_PLANE)plane);
        blur_pitch=blur->GetPitch((ADM_PLANE)plane);
        work_pitch=work->GetPitch((ADM_PLANE)plane);
        
        w=ww;
        h=hh;


        wk+=work_pitch;
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
                        ADM_ASM_ALIGN16
                        "pxor  %mm7,%mm7\n"
                : : );
#else
        __asm__(
                        ADM_ASM_ALIGN16
                        "pxor  %%mm7,%%mm7\n"
                : : );
#endif
  int wmod8=w>>3;                      
  for (int y=1; y<h-1 ;y++) 
  {               
        off=0;
        for (int x =0;x< wmod8; x++)
                {
                                               
                        __asm__(
                        ADM_ASM_ALIGN16
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
                        off+=8; 
                }    
        // mod 8 fix
        for(int x=wmod8*8;x<w;x++)
        {
                val=2*srcp[x]+srcpn[x]+srcpp[x];
                wk[x]=(val)>>2;
        }
        srcp+=src_pitch;
        srcpp+=src_pitch;
        srcpn+=src_pitch;
        wk+=work_pitch;     
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
        wk+=work_pitch;     
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
        blurp+=blur_pitch;     
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


/**
 * \fn detect_edges
 * @param src
 * @param dst
 * @param plane
 */
void Msharpen::detect_edges(ADMImage *src, ADMImage *dst,  int plane) 
{
  int ww,hh;

  const unsigned char *srcp,*srcp_saved; 
  const unsigned char *srcpn; 
  int src_pitch ;
  int dst_pitch ; 
  int h ;
  int w ;
  unsigned char *dstp;
  unsigned char *dstp_saved;

    src_pitch=src->GetPitch((ADM_PLANE)plane);
    dst_pitch=dst->GetPitch((ADM_PLANE)plane);
    srcp=src->GetReadPtr((ADM_PLANE)plane);
    dstp=dst->GetWritePtr((ADM_PLANE)plane);
    ww=src->GetWidth((ADM_PLANE)plane);
    hh=src->GetHeight((ADM_PLANE)plane);
    
    srcpn=srcp+src_pitch;
    srcp_saved=srcp;
    dstp_saved = dstp;

    int p,n,c;
    for (int y=0;y<hh-1;y++)
     {
      for(int xx=1;xx<ww-1;xx++)
        {
            p=srcp[xx+1];
            n=srcpn[xx+1];
            c=srcpn[xx-1];

            if(abs(n-p)>_param.threshold || abs(c-p)>_param.threshold) dstp[xx+1]=0xff;
                            else dstp[xx+1]=0;

        }
      srcp+=src_pitch;
      srcpn+=src_pitch;
      dstp+=dst_pitch;
     }
  if (_param.mask) 
  {
    dstp=dstp_saved;
    memset(dstp_saved+(h-1)*dst_pitch,0,w);  // Not used, if not returning mask
    for (int y=0;y<hh;dstp+=dst_pitch,y++) 
    {
      dstp[0]=0;
      dstp[1]=0;
      dstp[w-1]=0;
      dstp[w-2]=0;
    }
  }
}

//***************************************************
void Msharpen::detect_edges_HiQ(ADMImage *src, ADMImage *dst, int plane) 
{
// Vertical detail detection
  unsigned char *srcp,*srcp_saved; 
  int src_pitch ;
  int dst_pitch ; 
   unsigned char *srcpn; 
   unsigned char *dstp_saved,*dstp;
  int h ;
  int w ;

 
    srcp=src->GetReadPtr((ADM_PLANE)plane);
    dstp=dst->GetReadPtr((ADM_PLANE)plane);
    

    w=src->GetWidth((ADM_PLANE)plane);
    h=src->GetHeight((ADM_PLANE)plane);


    dst_pitch=dst->GetPitch((ADM_PLANE)plane);;
    src_pitch=src->GetPitch((ADM_PLANE)plane);;

    srcp_saved=srcp;
    dstp_saved=dstp;
  
    int b1,b2;
 
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
      if (abs(b1-b2)>=_param.threshold)
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
      if (abs(b1-b2)>=_param.threshold)
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
void Msharpen::apply_filter(ADMImage *src,ADMImage *blur, ADMImage *dst, int plane) 
{
  // TODO: MMX / ISSE
  const unsigned char *srcp ;
  const unsigned char *blurp ;
  const unsigned char *srcp_saved; 
  unsigned char *dstp_saved,*dstp ;
  const unsigned char *blurp_saved ;
  int src_pitch;
  int blur_pitch;
  int dst_pitch;
  int h;
  int w;
  
    srcp=src->GetReadPtr((ADM_PLANE)plane);
    blurp=blur->GetReadPtr((ADM_PLANE)plane);
    dstp=dst->GetWritePtr((ADM_PLANE)plane);
    
    w=src->GetWidth((ADM_PLANE)plane);
    h=src->GetHeight((ADM_PLANE)plane);

    blur_pitch=blur->GetPitch((ADM_PLANE)plane);;
    dst_pitch=dst->GetPitch((ADM_PLANE)plane);;
    src_pitch=src->GetPitch((ADM_PLANE)plane);;

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
  blurp=blurp+blur_pitch;
  int b4;
  for (int y=1;y<h-1;srcp+=src_pitch,dstp+=dst_pitch,blurp+=blur_pitch,y++)
  {
    for (int x=1;x<w-1;)
    {
      // small optimization, do 4 at a time if nothing to do
#if 0
      if (*(int*)(dstp+x)==0) 
      {
        *(int*)(dstp+x)=*(int*)(srcp+x);
        x+=4;
        continue;
      }
#endif
      if (dstp[x])
      {                                     
        b4=4*(int)(srcp[x])-3*(int)(blurp[x]);
        if (b4<0) 
            b4=0; 
        else 
            if (b4>255) 
                b4=255;
        dstp[x]=(_param.strength*b4+invstrength*(int)(srcp[x]))>>8;
      }
      else
        dstp[x]=srcp[x];
      x++; 
    }  
    dstp[0]=srcp[0]; 
    dstp[w-1]=srcp[w-1]; 
  }   
}
//***************************************************





