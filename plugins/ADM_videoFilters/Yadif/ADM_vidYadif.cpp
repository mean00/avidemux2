/*
	Yadif C-plugin for Avisynth 2.5 - Yet Another DeInterlacing Filter
	Copyright (C)2007 Alexander G. Balakhnin aka Fizick  http://avisynth.org.ru
    Port of YADIF filter from MPlayer
	Copyright (C) 2006 Michael Niedermayer <michaelni@gmx.at>

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

    Avisynth_C plugin
	Assembler optimized for GNU C compiler

*/
/*
  Ported to avidemux by mean
  Same license as original (?GPL)
*/

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "DIA_factory.h"
//************************************************
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define ABS(a) ((a) > 0 ? (a) : (-(a)))

#define MIN3(a,b,c) MIN(MIN(a,b),c)
#define MAX3(a,b,c) MAX(MAX(a,b),c)

//===========================================================================//
#ifdef ADM_CPU_X86
extern "C"
{
void filter_line_mmx2(int mode, uint8_t *dst, const uint8_t *prev, const uint8_t *cur, const uint8_t *next, int w, int refs, int parity);
}
#endif
//
typedef struct YADIF_PARAM
{
    uint32_t mode;
    uint32_t order;
}YADIF_PARAM;

class  ADMVideoYadif:public AVDMGenericVideoStream
{

 protected:
  virtual char                 *printConf(void);
  YADIF_PARAM                  *_param;
   VideoCache                  *vidCache;
  void                         updateInfo(void);

 public:
                  ADMVideoYadif(AVDMGenericVideoStream *in, CONFcouple *setup);
  virtual         ~ADMVideoYadif();
  virtual uint8_t configure( AVDMGenericVideoStream *instream) ;
  virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len, ADMImage *data,uint32_t *flags);
  virtual uint8_t	getCoupledConf( CONFcouple **couples)				;
 }     ;

static FILTER_PARAM yadifParam={2,{"mode","order"}};
//************************************

VF_DEFINE_FILTER(ADMVideoYadif,yadifParam,
                YADIF,
                QT_TR_NOOP("yadif"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Yet Another DeInterlacer. Ported from MPlayer."));

//************************************
//
static void filter_plane(int mode, uint8_t *dst, int dst_stride, const uint8_t *prev0, const uint8_t *cur0, const uint8_t *next0, int refs, int w, int h, int parity, int tff, int mmx);


//***************************************************
//***************************************************
char *ADMVideoYadif::printConf( void )
{
  ADM_FILTER_DECLARE_CONF(" Yadif : mode %u order %d",_param->mode, _param->order);
  
}

ADMVideoYadif::ADMVideoYadif(AVDMGenericVideoStream *in, CONFcouple *couples)
{
  _in=in;		

  memcpy(&_info,_in->getInfo(),sizeof(_info)); 
  _info.encoding=1;

 // _uncompressed=new uint8_t [3*_in->getInfo()->width*_in->getInfo()->height];
 

  if(couples)
  {
   	 _param=new (YADIF_PARAM);
	GET(mode);
        GET(order);
  }
  else
  {
    _param = new ( YADIF_PARAM);
    _param->mode=0;
    _param->order=1;
  }
  _uncompressed=new ADMImage(_in->getInfo()->width,_in->getInfo()->height);
  ADM_assert(_uncompressed);    	  	
  vidCache = new VideoCache (10, in);
  updateInfo();
}

void ADMVideoYadif::updateInfo(void)
{
   memcpy(&_info,_in->getInfo(),sizeof(_info)); 
  if(_param->mode &1 ) // Bob
  {
    _info.nb_frames*=2;
    _info.fps1000*=2;
  }
}

uint8_t ADMVideoYadif::configure( AVDMGenericVideoStream *instream) 
{
  _in= instream;
     diaMenuEntry tMode[]={
                             {0,      QT_TR_NOOP("Temporal & spatial check"),NULL},
                             {1,   QT_TR_NOOP("Bob, temporal & spatial check"),NULL},
                             {2,      QT_TR_NOOP("Skip spatial temporal check"),NULL},
                             {3,  QT_TR_NOOP("Bob, skip spatial temporal check"),NULL}
          };
     diaMenuEntry tOrder[]={
                             {0,      QT_TR_NOOP("Bottom field first"),NULL},
                             {1,   QT_TR_NOOP("Top field first"),NULL}
          };
  
     diaElemMenu mMode(&(_param->mode),   QT_TR_NOOP("_Mode:"), 4,tMode);
     diaElemMenu morder(&(_param->order),   QT_TR_NOOP("_Order:"), 2,tOrder);
     
     diaElem *elems[]={&mMode,&morder};
     
     if(diaFactoryRun(QT_TR_NOOP("yadif"),sizeof(elems)/sizeof(diaElem *),elems))
     {
        updateInfo();
        return 1;
     }
     return 0;
}

uint8_t	ADMVideoYadif::getCoupledConf( CONFcouple **couples)
{
#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))

          ADM_assert(_param);
          *couples=new CONFcouple(2);

          CSET(mode);
          CSET(order);
          return 1;

}


ADMVideoYadif::~ADMVideoYadif()
{
        delete  _uncompressed;
        _uncompressed=NULL;
       
        delete vidCache;
        vidCache = NULL;
        
        delete _param;
        _param=NULL;
}

uint8_t ADMVideoYadif::getFrameNumberNoAlloc(uint32_t frame,
                                              uint32_t *len,
                                              ADMImage *data,
                                              uint32_t *flags)
{
        int mode;
        int parity;
        int tff;
        int iplane;
        int cpu;
        int n;
        ADMImage *src, *dst, * prev, *next;

    
        mode = _param->mode;

        if (mode & 1) 
                n = (frame>>1); // bob
        else
                n = frame;

        src = vidCache->getImage(n);
  // Request frame 'n' from the child (source) clip.

        if (n>0)
                prev =  vidCache->getImage( n-1); // get previous frame
        else
                prev= vidCache->getImage(0); // get very first frame

        if (n< _in->getInfo()->nb_frames-1)
                next = vidCache->getImage( n+1); // get next frame
        else
                next = vidCache->getImage( _in->getInfo()->nb_frames-1); // get last frame

        dst = data;
        
        if(!prev || !src || !next)
        {
            printf("Failed to read frame for frame %u\n",frame);
            vidCache->unlockAll();
            return 0;
        }
        
  // Construct a frame based on the information of the current frame
  // contained in the "vi" struct.
#if 0 //MEANX
        if (_params->order == -1)
//		tff = avs_is_tff(&p->vi) == 0 ? 0 : 1; // 0 or 1
                tff = avs_get_parity(p->child, n) ? 1 : 0; // 0 or 1
        else
#endif
                tff = _param->order;	
        
        parity = (mode & 1) ? (frame & 1) ^ (1^tff) : (tff ^ 1);  // 0 or 1

      //MEANX  cpu = avs_get_cpu_flags(p->env);

        for (iplane = 0; iplane<3; iplane++)
        {
                ADM_PLANE plane = (iplane==0) ? PLANAR_Y : (iplane==1) ? PLANAR_U : PLANAR_V;

                const unsigned char* srcp = src->GetWritePtr(plane);
          // Request a Read pointer from the current source frame

                const unsigned char* prevp0 = prev->GetWritePtr( plane);
                unsigned char* prevp = (unsigned char*) prevp0; // with same pitch
          // Request a Read pointer from the prev source frame.

                const unsigned char* nextp0 = next->GetWritePtr( plane);
                unsigned char* nextp = (unsigned char*) nextp0; // with same pitch
          // Request a Read pointer from the next source frame.

                unsigned char* dstp = dst->GetWritePtr( plane);
                // Request a Write pointer from the newly created destination image.
          // You can request a writepointer to images that have just been

                const int dst_pitch = dst->GetPitch( plane);
          // Requests pitch (length of a line) of the destination image.
          // For more information on pitch see: http://www.avisynth.org/index.php?page=WorkingWithImages
                // (short version - pitch is always equal to or greater than width to allow for seriously fast assembly code)

                const int width =dst->GetPitch( plane);
          // Requests rowsize (number of used bytes in a line.
          // See the link above for more information.

                const int height = dst->GetHeight( plane);
          // Requests the height of the destination image.

                const int src_pitch = src->GetPitch(plane);
                const int prev_pitch = prev->GetPitch(plane);
                const int next_pitch = next->GetPitch(plane);

                // in v.0.1-0.3  all source pitches are  assumed equal (for simplicity)
                                // consider other (rare) case
                if (prev_pitch != src_pitch)
                {
                    prevp = (unsigned char *)ADM_alloc(height*src_pitch);
                    int h;
                    for (h=0; h<0; h++)
                      memcpy(prevp+h*src_pitch, prevp0+h*prev_pitch, width);
                }
                    
                if (next_pitch != src_pitch)
                {
                    nextp = (unsigned char *)ADM_alloc(height*src_pitch);
                    int h;
                    for (h=0; h<0; h++)
                      memcpy(nextp+h*src_pitch, nextp0+h*next_pitch, width);
                }
                    
                filter_plane(mode, dstp, dst_pitch, prevp, srcp, nextp, src_pitch, width, height, parity, tff, 0);
                if (prev_pitch != src_pitch)
                        ADM_dealloc(prevp);
                if (next_pitch != src_pitch)
                        ADM_dealloc(nextp);
        }
       vidCache->unlockAll();
      return 1;
}
//****************

static void filter_line_c(int mode, uint8_t *dst, const uint8_t *prev, const uint8_t *cur, const uint8_t *next, int w, int refs, int parity){
    int x;
    const uint8_t *prev2= parity ? prev : cur ;
    const uint8_t *next2= parity ? cur  : next;
    for(x=0; x<w; x++){
        int c= cur[-refs];
        int d= (prev2[0] + next2[0])>>1;
        int e= cur[+refs];
        int temporal_diff0= ABS(prev2[0] - next2[0]);
        int temporal_diff1=( ABS(prev[-refs] - c) + ABS(prev[+refs] - e) )>>1;
        int temporal_diff2=( ABS(next[-refs] - c) + ABS(next[+refs] - e) )>>1;
        int diff= MAX3(temporal_diff0>>1, temporal_diff1, temporal_diff2);
        int spatial_pred= (c+e)>>1;
        int spatial_score= ABS(cur[-refs-1] - cur[+refs-1]) + ABS(c-e)
                         + ABS(cur[-refs+1] - cur[+refs+1]) - 1;

#define CHECK(j)\
    {   int score= ABS(cur[-refs-1+ j] - cur[+refs-1- j])\
                 + ABS(cur[-refs  + j] - cur[+refs  - j])\
                 + ABS(cur[-refs+1+ j] - cur[+refs+1- j]);\
        if(score < spatial_score){\
            spatial_score= score;\
            spatial_pred= (cur[-refs  + j] + cur[+refs  - j])>>1;\

        CHECK(-1) CHECK(-2) }} }}
        CHECK( 1) CHECK( 2) }} }}

        if(mode<2){
            int b= (prev2[-2*refs] + next2[-2*refs])>>1;
            int f= (prev2[+2*refs] + next2[+2*refs])>>1;
#if 0
            int a= cur[-3*refs];
            int g= cur[+3*refs];
            int max= MAX3(d-e, d-c, MIN3(MAX(b-c,f-e),MAX(b-c,b-a),MAX(f-g,f-e)) );
            int min= MIN3(d-e, d-c, MAX3(MIN(b-c,f-e),MIN(b-c,b-a),MIN(f-g,f-e)) );
#else
            int max= MAX3(d-e, d-c, MIN(b-c, f-e));
            int min= MIN3(d-e, d-c, MAX(b-c, f-e));
#endif

            diff= MAX3(diff, min, -max);
        }

        if(spatial_pred > d + diff)
           spatial_pred = d + diff;
        else if(spatial_pred < d - diff)
           spatial_pred = d - diff;

        dst[0] = spatial_pred;

        dst++;
        cur++;
        prev++;
        next++;
        prev2++;
        next2++;
    }
}

void filter_plane(int mode, uint8_t *dst, int dst_stride, const uint8_t *prev0, const uint8_t *cur0, const uint8_t *next0, int refs, int w, int h, int parity, int tff, int mmx)
{
void (*filter_line)(int mode, uint8_t *dst, const uint8_t *prev, const uint8_t *cur, const uint8_t *next, int w, int refs, int parity);
	int y;
	filter_line = filter_line_c;
#ifdef ADM_CPU_X86
	if (CpuCaps::hasMMXEXT()) 
		filter_line = filter_line_mmx2;
#endif

        memcpy(dst, cur0, w);
        memcpy(dst + dst_stride, cur0 + refs, w);
        for(y=2; y<h-1; y++){
            if(((y ^ parity) & 1)){
                const uint8_t *prev= prev0 + y*refs;
                const uint8_t *cur = cur0 + y*refs;
                const uint8_t *next= next0 + y*refs;
                uint8_t *dst2= dst + y*dst_stride;
                filter_line(mode, dst2, prev, cur, next, w, refs, (parity ^ tff));
            }else{
                memcpy(dst + y*dst_stride, cur0 + y*refs, w);
            }
        }
        memcpy(dst + (h-1)*dst_stride, cur0 + (h-1)*refs, w);

#ifdef ADM_CPU_X86
	if (CpuCaps::hasMMXEXT()) 
		asm volatile("emms");
#endif
}

