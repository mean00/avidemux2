//
//
// C++ Implementation: ADM_mpdetc
//
// Description: 
//
//	This is a port of mplayer ivtc filter by Richard 
//	Copyright Richard Felker
//
// Author:Richard Felker, port to avidemux2 by  mean <fixounet@free.fr>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
//
#define HAVE_DETC
#ifdef HAVE_DETC

#include "config.h"
#include "ADM_lavcodec.h"
#include "fourcc.h"

#include "ADM_toolkit/toolkit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_video/ADM_mpdetc.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_FILTER
#include "ADM_osSupport/ADM_debug.h"

//static void decimate(uint8_t *src,uint8_t *target, uint32_t linessrc, uint32_t width);
#include "ADM_filter/video_filters.h"


static FILTER_PARAM flipParam={0,{""}};


SCRIPT_CREATE(mpdetc_script,AVDMVideoMPDetc,flipParam);
BUILD_CREATE(mpdetc_create,AVDMVideoMPDetc);


void 		*my_memcpy_pic(uint8_t * dst, uint8_t * src, int bytesPerLine, int height, 
						int dstStride, int srcStride);
unsigned int 	hash_pic(unsigned char *img, int w, int h, int stride);
static void 	block_diffs_C(struct metrics *m, unsigned char *old, unsigned char *nw, int os, int ns);
#ifdef ADM_CPU_X86
void block_diffs_MMX(struct metrics *m, unsigned char *old, unsigned char *ew, int os, int ns);
#endif
static void 	diff_planes(struct metrics *m, unsigned char *old, unsigned char *nw, int w, int h, int os, int ns);
static void 	diff_fields(struct frameinfo *fi, uint8_t  *old, uint8_t  *nw);
static void 	status(struct frameinfo *f);
static void 	copy_image(ADMImage  *sdest, ADMImage  *ssrc, int field);

static uint32_t 	myw,myh;
static struct vf_priv_s *myparam; // UGLY FIXME BURK




char *AVDMVideoMPDetc::printConf( void )
{
 	static char buf[50];

 	sprintf((char *)buf," Mplayer ivtc");
        return buf;
}

//_______________________________________________________________
AVDMVideoMPDetc::AVDMVideoMPDetc(
				AVDMGenericVideoStream *in,CONFcouple *setup)
{
UNUSED_ARG(setup);
  	_in=in;
   	memcpy(&_info,_in->getInfo(),sizeof(_info));
	
	// We alter frame rate here and # of frames
	aprintf("Frame in : %lu fpsin: %lu\n",_info.nb_frames,_info.fps1000);
	_info.fps1000=(_info.fps1000*4)/5;	
	_info.nb_frames=_info.nb_frames/5;
	_info.nb_frames=_info.nb_frames*4;
	aprintf("Frame out : %lu fpout: %lu\n",_info.nb_frames,_info.fps1000);
//	_uncompressed=new uint8_t[(3*_info.width*_info.height)>>1];
//	_lastFrame=new uint8_t[(3*_info.width*_info.height)>>1];
	_uncompressed=new ADMImage(_info.width,_info.height);
	_lastFrame=new ADMImage(_info.width,_info.height);
	
	_param=new vf_priv_s;
	_param->drop = 0;
	_param->first = 1;
	_inFrame=0;
	_lastAsked=0xffffff0;
		
}

// ___ destructor_____________
AVDMVideoMPDetc::~AVDMVideoMPDetc()
{
 	delete  _uncompressed;
	delete  _lastFrame;
	if(myparam)
	{
		delete _param;
		_param=NULL;
	}
	

}

int foo(struct vf_priv_s *p, uint8_t  *nw, uint8_t *cur)
{
	struct frameinfo *f = p->fi;

	f[0] = f[1];
	diff_fields(&(f[1]), cur, nw);
	status(&(f[1]));

	// Immediately drop this frame if it's already been used.
	if (p->dropnext) {
		p->dropnext = 0;
		return F_DROP;
	}
	
	// Sometimes a pulldown frame comes all by itself, so both
	// its top and bottom field are duplicates from the adjacent
	// two frames. We can just drop such a frame, but we
	// immediately show the next frame instead to keep the frame
	// drops evenly spaced during normal 3:2 pulldown sequences.
	if ((3*f[1].r.o < f[1].r.e) && (f[1].r.s < f[1].r.d)) {
		p->dropnext = 1;
		return F_NEXT;
	}
	
	// If none of these conditions hold, we will consider the frame
	// progressive and just show it as-is.
	if (!(  (3*f[0].r.e < f[0].r.o) ||
		((2*f[0].r.d < f[0].r.s) && (f[0].r.s > 1200)) ||
		((2*f[1].r.t < f[1].r.p) && (f[1].r.p > 1200))  ))
		return F_SHOW;

	// Otherwise, we have to decide whether to merge or drop.
	// If the noise metric only increases minimally, we're off
	// to a good start...
	if (((2*f[1].r.t < 3*f[1].r.p) && (f[1].r.t < 3600)) ||
		(f[1].r.t < 900) || (f[1].r.d < 900)) {
		// ...and if noise decreases or the duplicate even field
		// is detected, we go ahead with the merge.
		if ((3*f[0].r.e < f[0].r.o) || (2*f[1].r.t < f[1].r.p)) {
			p->dropnext = 1;
			return F_MERGE;
		}
	}
	return F_DROP;
}

//
//	Basically ask a uncompressed frame from editor and ask
//		GUI to decompress it .
//

#define clear(x) {memset(x,0,page);memset(x+page,128,page>>1);}

uint8_t AVDMVideoMPDetc::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
uint32_t page=_info.width*_info.height;

		if(frame>=_info.nb_frames) return 0;
		
       				
		myw=_info.width;
		myh=_info.height; // UGLY!!!
		myparam=_param;
		aprintf("Request for %lu frames (last %lu)\n",frame,_lastAsked);
		// First frame or out of order, we copy it to prefill buffer
		if(_lastAsked+1!=frame) 
		{
			// Convert to source filter frame no
		  uint32_t off;			
			aprintf("--Out of order access\n");
			off=frame%4;	
			frame=frame-off;		
			_inFrame=(frame*5)>>2;
			// 4 frame -> 5 frame
			_inFrame+=off;
			// Init buffer
			// Clear refimage
			clear(_lastFrame);	
			_param->lastdrop=0;
			_param->inframes = 1;
			_param->outframes=1;					
			_param->first=0;
			_copy=0;
			_lastAsked=frame;
			if(!_in->getFrameNumberNoAlloc(_inFrame, len,data,flags)) return 0;	
			return 1;
		}
		// Sequential access, we have the previous frame pre-filled		
		uint8_t frame_ready=0;
		int off;
		
		_lastAsked=frame;
		while(!frame_ready )
		{
			
			
			if(!_in->getFrameNumberNoAlloc(_inFrame, len,_uncompressed,flags))
			{
				// be nice and return 1 if it does not seem to stupid
				if(_param->inframes*4-_param->outframes*5 < 30)
				{
					_param->outframes++;
					printf("Skew ignored\n");
					return 1;
				}
				 return 0;
			}
		
			_inFrame++;
			myparam->inframes++;
			off=4*_param->inframes - 5*(_param->outframes+1);
			aprintf("**** Inframe : %lu / outframe : %lu / Off: %d (in: %lu)\n",
				 _param->inframes,_param->outframes,off,_inFrame);
/*				 
			if(off>11)
			{
				_copy=9;
			}
			if(_copy)
			{
				_copy--;
				aprintf("Too much drops!\n");
				_param->outframes++;
				_param->lastdrop=0;
				if(!_copy)
					copy_image(_lastFrame, _uncompressed, 2);
				copy_image(data, _uncompressed, 2);
				
				return 1;				
			}				 
			
*/			
			
			switch (foo(_param, _uncompressed->data, _lastFrame->data)) 
			{
			case F_DROP:
				copy_image(_lastFrame, _uncompressed, 2);
				frame_ready = 0;
				_param->lastdrop = 0;
				aprintf("MPivtc DROP\n");				
				// we are dropping too much frames!
				if(off>6)
				{
					aprintf("Cancelled dropout!, deinterlacing in place----------\n");
					memcpy(data->data,_lastFrame,(page*3)>>1);
					frame_ready=1;
					_param->outframes++;
					_param->dropnext=0;
					// we deinterlace it in place
					AVPicture src;
					uint32_t page=_info.width*_info.height;
		
					src.data[0]=data->data;
					src.data[1]=data->data+page;
					src.data[2]=data->data+((page*5)>>2);
  					
		
					src.linesize[0]=_info.width;
					src.linesize[1]=_info.width>>1;
					src.linesize[2]=_info.width>>1;
  
					if (avpicture_deinterlace(&src,&src,
						PIX_FMT_YUV420P,_info.width,_info.height)<0)
					{
						printf("Error in avpicture deinterlace!\n");
						return 0;
					} 		
				}
				
				break;
			case F_MERGE:
				copy_image(_lastFrame, _uncompressed, 0);
				frame_ready = do_put_image(_lastFrame);
				if(frame_ready) memcpy(data->data,_lastFrame->data,(page*3)>>1);
				copy_image(_lastFrame, _uncompressed, 1);
				aprintf("MPivtc MERGE\n");
				//clear(_lastFrame);
				break;
			case F_NEXT:
				copy_image(_lastFrame, _uncompressed, 2);
				frame_ready = do_put_image(_lastFrame);
				if(frame_ready) memcpy(data->data,_lastFrame->data,(page*3)>>1);
				aprintf("MPivtc NEXT\n");
				//clear(_lastFrame);
				break;
			case F_SHOW:
				frame_ready = do_put_image(_lastFrame);
				if(frame_ready) memcpy(data->data,_lastFrame->data,(page*3)>>1);
				copy_image(_lastFrame, _uncompressed, 2);
				aprintf("MPivtc OK\n");
				//clear(_lastFrame);
				break;
			}
		}
	if(!frame_ready) return 0;	
	return 1;
}
		

/*
	Return 1 if ok, 0 if dropped
*/
uint8_t AVDMVideoMPDetc::do_put_image(ADMImage *data)
{
	int dropflag;
	int off=0;
/*
	switch (_param->drop && !_param->dropnext) 
	{
	case 0:
		dropflag = 0;
		break;
	case 1:
		dropflag = (++_param->lastdrop >= 5);
		break;
	case 2:
		dropflag = (++_param->lastdrop >= 5) && (4*_param->inframes <= 5*_param->outframes);
		break;
	}
*/
	off=4*_param->inframes - 5*(_param->outframes+1);
	aprintf("Dropnext : %d lastdrop  : %d off : %d \n",_param->dropnext,_param->lastdrop,off);
		
	dropflag = ((_param->dropnext)||(++_param->lastdrop >= 5)) && off< 0;
	
	if (dropflag) {
		aprintf("-->drop! [%d/%d=%g]\n",
			_param->outframes, _param->inframes, (float)_param->outframes/_param->inframes);
		_param->lastdrop = 0;
		return 0;
	}

	_param->outframes++;
	return 1;


}






void *my_memcpy_pic(uint8_t  * dst, uint8_t * src, int bytesPerLine, int height, int dstStride, int srcStride)
{
	int i;
	void *retval=dst;

	for(i=0; i<height; i++)
	{
		memcpy(dst, src, bytesPerLine);
		src+= srcStride;
		dst+= dstStride;
	}

	return retval;
}

unsigned int hash_pic(unsigned char *img, int w, int h, int stride)
{
	int step = w*h/1024;
	unsigned int hash=0;
	int x=0, y;

	step -= step % 3;

	for (y=0; y<h; y++) {
		for (; x<w; x+=step) {
			hash = hash ^ (hash<<4) ^ img[x];
		}
		x -= w;
		img += stride;
	}
	
	return hash;
}
#define MAG(a) (((a)^((a)>>31))-((a)>>31))
#define LOWPASS(s) ((s)[0])


 void block_diffs_C(struct metrics *m, unsigned char *old, unsigned char *nw, int os, int ns)
{
	int x, y, e=0, o=0, s=0, p=0, t=0;
	unsigned char *oldp, *newp;
	m->s = m->p = m->t = 0;
	for (x = 8; x; x--) {
		oldp = old++;
		newp = nw++;
		s = p = t = 0;
		for (y = 4; y; y--) {
			e += MAG(newp[0]-oldp[0]);
			o += MAG(newp[ns]-oldp[os]);
			s += newp[ns]-newp[0];
			p += oldp[os]-oldp[0];
			t += oldp[os]-newp[0];
			oldp += os<<1;
			newp += ns<<1;
		}
		m->s += MAG(s);
		m->p += MAG(p);
		m->t += MAG(t);
	}
	m->e = e;
	m->o = o;
	m->d = e+o;
}
#define MAXUP(a,b) ((a) = ((a)>(b)) ? (a) : (b))

static void diff_planes(struct frameinfo *fi,
	unsigned char *old, unsigned char *nw, int w, int h, int os, int ns)
{
	int x, y;
	struct metrics l;
	struct metrics *peak=&fi->p, *rel=&fi->r, *mean=&fi->m;
	memset(peak, 0, sizeof(struct metrics));
	memset(rel, 0, sizeof(struct metrics));
	memset(mean, 0, sizeof(struct metrics));
	for (y = 0; y < h-7; y += 8) {
		for (x = 8; x < w-8-7; x += 8) {
#if 00 //FIXME def USE_MMX	
	#define block_diffs block_diffs_MMX
#else
	#define block_diffs block_diffs_C
#endif	
			block_diffs(&l, old+x+y*os, nw+x+y*ns, os, ns);
			mean->d += l.d;
			mean->e += l.e;
			mean->o += l.o;
			mean->s += l.s;
			mean->p += l.p;
			mean->t += l.t;
			MAXUP(peak->d, l.d);
			MAXUP(peak->e, l.e);
			MAXUP(peak->o, l.o);
			MAXUP(peak->s, l.s);
			MAXUP(peak->p, l.p);
			MAXUP(peak->t, l.t);
			MAXUP(rel->e, l.e-l.o);
			MAXUP(rel->o, l.o-l.e);
			MAXUP(rel->s, l.s-l.t);
			MAXUP(rel->p, l.p-l.t);
			MAXUP(rel->t, l.t-l.p);
			MAXUP(rel->d, l.t-l.s); /* hack */
	
		}
	}
	x = (w/8-2)*(h/8);
	mean->d /= x;
	mean->e /= x;
	mean->o /= x;
	mean->s /= x;
	mean->p /= x;
	mean->t /= x;
}
void diff_fields(struct frameinfo *fi, uint8_t  *old, uint8_t  *nw)
{
	diff_planes(fi, old, nw,
		myw, myh, myw, myw);
}



 void status(struct frameinfo *f)
{
	aprintf("       pd=%d re=%d ro=%d rp=%d rt=%d rs=%d rd=%d pp=%d pt=%d ps=%d\n",
		f->p.d, f->r.e, f->r.o, f->r.p, f->r.t, f->r.s, f->r.d, f->p.p, f->p.t, f->p.s);
}



//void copy_image(mp_image_t *dmpi, mp_image_t *mpi, int field)
void copy_image(ADMImage  *sdest, ADMImage  *ssrc, int field)
{
	uint32_t page;
	page=myw*myh;
	uint8_t *src,*dest;
	src=ssrc->data;
	dest=sdest->data;
	switch (field) {
	case 0:
		my_memcpy_pic(dest, src, myw, myh>>1,
			myw*2, myw*2);
		//if (mpi->flags & MP_IMGFLAG_PLANAR) {
		if(1){
			src+=page;
			dest+=page;
			/*my_memcpy_pic(dmpi->planes[1], mpi->planes[1],
				mpi->chroma_width, mpi->chroma_height/2,
				dmpi->stride[1]*2, mpi->stride[1]*2);
			my_memcpy_pic(dmpi->planes[2], mpi->planes[2],
				mpi->chroma_width, mpi->chroma_height/2,
				dmpi->stride[2]*2, mpi->stride[2]*2);*/
			my_memcpy_pic(dest, src, myw>>1, myh>>2,
						myw, myw);
			src+=page>>2;
			dest+=page>>2;
			my_memcpy_pic(dest, src, myw>>1, myh>>2,
					myw, myw);
		}
		break;
	case 1:
		/*my_memcpy_pic(dmpi->planes[0]+dmpi->stride[0],
			mpi->planes[0]+mpi->stride[0], mpi->w, mpi->h/2,
			dmpi->stride[0]*2, mpi->stride[0]*2);*/
		my_memcpy_pic(dest+myw, src+myw, myw, myh>>1,
			myw*2, myw*2);	
		//if (mpi->flags & MP_IMGFLAG_PLANAR) {
		if(1){/*
			my_memcpy_pic(dmpi->planes[1]+dmpi->stride[1],
				mpi->planes[1]+mpi->stride[1],
				mpi->chroma_width, mpi->chroma_height/2,
				dmpi->stride[1]*2, mpi->stride[1]*2);
			my_memcpy_pic(dmpi->planes[2]+dmpi->stride[2],
				mpi->planes[2]+mpi->stride[2],
				mpi->chroma_width, mpi->chroma_height/2,
				dmpi->stride[2]*2, mpi->stride[2]*2);*/
			src+=page;
			dest+=page;
			/*my_memcpy_pic(dmpi->planes[1], mpi->planes[1],
				mpi->chroma_width, mpi->chroma_height/2,
				dmpi->stride[1]*2, mpi->stride[1]*2);
			my_memcpy_pic(dmpi->planes[2], mpi->planes[2],
				mpi->chroma_width, mpi->chroma_height/2,
				dmpi->stride[2]*2, mpi->stride[2]*2);*/
			my_memcpy_pic(dest+myw/2, src+myw/2, myw>>1, myh>>2,
					myw, myw);
			src+=page>>2;
			dest+=page>>2;
			my_memcpy_pic(dest+myw/2, src+myw/2, myw>>1, myh>>2,
					myw, myw);				
		}
		break;
	case 2:
		/*
		memcpy_pic(dmpi->planes[0], mpi->planes[0], mpi->w, mpi->h,
			dmpi->stride[0], mpi->stride[0]);
		if (mpi->flags & MP_IMGFLAG_PLANAR) {
			memcpy_pic(dmpi->planes[1], mpi->planes[1],
				mpi->chroma_width, mpi->chroma_height,
				dmpi->stride[1], mpi->stride[1]);
			memcpy_pic(dmpi->planes[2], mpi->planes[2],
				mpi->chroma_width, mpi->chroma_height,
				dmpi->stride[2], mpi->stride[2]);
		}*/
		memcpy(dest,src,(page*3)>>1);
		break;
	}
}
#if 0 // FIXME USE_MMX
 void block_diffs_MMX(struct metrics *m, unsigned char *old, unsigned char *nw, int os, int ns)
{
	int i;
	short out[24]; // output buffer for the partial metrics from the mmx code
	
	__asm__ (
		"movl $4, %%ecx \n\t"
		"pxor %%mm4, %%mm4 \n\t" // 4 even difference sums
		"pxor %%mm5, %%mm5 \n\t" // 4 odd difference sums
		"pxor %%mm7, %%mm7 \n\t" // all zeros
		
		".balign 16 \n\t"
		"1: \n\t"
		
		// Even difference
		"movq (%%esi), %%mm0 \n\t"
		"movq (%%esi), %%mm2 \n\t"
		"addl %%eax, %%esi \n\t"
		"movq (%%edi), %%mm1 \n\t"
		"addl %%ebx, %%edi \n\t"
		"psubusb %%mm1, %%mm2 \n\t"
		"psubusb %%mm0, %%mm1 \n\t"
		"movq %%mm2, %%mm0 \n\t"
		"movq %%mm1, %%mm3 \n\t"
		"punpcklbw %%mm7, %%mm0 \n\t"
		"punpcklbw %%mm7, %%mm1 \n\t"
		"punpckhbw %%mm7, %%mm2 \n\t"
		"punpckhbw %%mm7, %%mm3 \n\t"
		"paddw %%mm0, %%mm4 \n\t"
		"paddw %%mm1, %%mm4 \n\t"
		"paddw %%mm2, %%mm4 \n\t"
		"paddw %%mm3, %%mm4 \n\t"
		
		// Odd difference
		"movq (%%esi), %%mm0 \n\t"
		"movq (%%esi), %%mm2 \n\t"
		"addl %%eax, %%esi \n\t"
		"movq (%%edi), %%mm1 \n\t"
		"addl %%ebx, %%edi \n\t"
		"psubusb %%mm1, %%mm2 \n\t"
		"psubusb %%mm0, %%mm1 \n\t"
		"movq %%mm2, %%mm0 \n\t"
		"movq %%mm1, %%mm3 \n\t"
		"punpcklbw %%mm7, %%mm0 \n\t"
		"punpcklbw %%mm7, %%mm1 \n\t"
		"punpckhbw %%mm7, %%mm2 \n\t"
		"punpckhbw %%mm7, %%mm3 \n\t"
		"paddw %%mm0, %%mm5 \n\t"
		"paddw %%mm1, %%mm5 \n\t"
		"paddw %%mm2, %%mm5 \n\t"
		"paddw %%mm3, %%mm5 \n\t"
			
		"decl %%ecx \n\t"
		"jnz 1b \n\t"
		"movq %%mm4, (%%edx) \n\t"
		"movq %%mm5, 8(%%edx) \n\t"
		: 
		: "S" (old), "D" (nw), "a" (os), "b" (ns), "d" (out)
		: "memory"
		);
	m->e = out[0]+out[1]+out[2]+out[3];
	m->o = out[4]+out[5]+out[6]+out[7];
	m->d = m->e + m->o;

	asm (
		// First loop to measure first four columns
		"movl $4, %%ecx \n\t"
		"pxor %%mm4, %%mm4 \n\t" // Past spacial noise
		"pxor %%mm5, %%mm5 \n\t" // Temporal noise
		"pxor %%mm6, %%mm6 \n\t" // Current spacial noise
		
		".balign 16 \n\t"
		"2: \n\t"
		
		"movq (%%esi), %%mm0 \n\t"
		"movq (%%esi,%%eax), %%mm1 \n\t"
		"addl %%eax, %%esi \n\t"
		"addl %%eax, %%esi \n\t"
		"movq (%%edi), %%mm2 \n\t"
		"movq (%%edi,%%ebx), %%mm3 \n\t"
		"addl %%ebx, %%edi \n\t"
		"addl %%ebx, %%edi \n\t"
		"punpcklbw %%mm7, %%mm0 \n\t"
		"punpcklbw %%mm7, %%mm1 \n\t"
		"punpcklbw %%mm7, %%mm2 \n\t"
		"punpcklbw %%mm7, %%mm3 \n\t"
		"paddw %%mm1, %%mm4 \n\t"
		"paddw %%mm1, %%mm5 \n\t"
		"paddw %%mm3, %%mm6 \n\t"
		"psubw %%mm0, %%mm4 \n\t"
		"psubw %%mm2, %%mm5 \n\t"
		"psubw %%mm2, %%mm6 \n\t"
		
		"decl %%ecx \n\t"
		"jnz 2b \n\t"
		
		"movq %%mm0, %%mm1 \n\t"
		"movq %%mm0, %%mm2 \n\t"
		"movq %%mm0, %%mm3 \n\t"
		"pcmpgtw %%mm4, %%mm1 \n\t"
		"pcmpgtw %%mm5, %%mm2 \n\t"
		"pcmpgtw %%mm6, %%mm3 \n\t"
		"pxor %%mm1, %%mm4 \n\t"
		"pxor %%mm2, %%mm5 \n\t"
		"pxor %%mm3, %%mm6 \n\t"
		"psubw %%mm1, %%mm4 \n\t"
		"psubw %%mm2, %%mm5 \n\t"
		"psubw %%mm3, %%mm6 \n\t"
		"movq %%mm4, (%%edx) \n\t"
		"movq %%mm5, 16(%%edx) \n\t"
		"movq %%mm6, 32(%%edx) \n\t"

		"movl %%eax, %%ecx \n\t"
		"shll $3, %%ecx \n\t"
		"subl %%ecx, %%esi \n\t"
		"movl %%ebx, %%ecx \n\t"
		"shll $3, %%ecx \n\t"
		"subl %%ecx, %%edi \n\t"

		// Second loop for the last four columns
		"movl $4, %%ecx \n\t"
		"pxor %%mm4, %%mm4 \n\t"
		"pxor %%mm5, %%mm5 \n\t"
		"pxor %%mm6, %%mm6 \n\t"
		
		".balign 16 \n\t"
		"3: \n\t"
		
		"movq (%%esi), %%mm0 \n\t"
		"movq (%%esi,%%eax), %%mm1 \n\t"
		"addl %%eax, %%esi \n\t"
		"addl %%eax, %%esi \n\t"
		"movq (%%edi), %%mm2 \n\t"
		"movq (%%edi,%%ebx), %%mm3 \n\t"
		"addl %%ebx, %%edi \n\t"
		"addl %%ebx, %%edi \n\t"
		"punpckhbw %%mm7, %%mm0 \n\t"
		"punpckhbw %%mm7, %%mm1 \n\t"
		"punpckhbw %%mm7, %%mm2 \n\t"
		"punpckhbw %%mm7, %%mm3 \n\t"
		"paddw %%mm1, %%mm4 \n\t"
		"paddw %%mm1, %%mm5 \n\t"
		"paddw %%mm3, %%mm6 \n\t"
		"psubw %%mm0, %%mm4 \n\t"
		"psubw %%mm2, %%mm5 \n\t"
		"psubw %%mm2, %%mm6 \n\t"
		
		"decl %%ecx \n\t"
		"jnz 3b \n\t"
		
		"movq %%mm0, %%mm1 \n\t"
		"movq %%mm0, %%mm2 \n\t"
		"movq %%mm0, %%mm3 \n\t"
		"pcmpgtw %%mm4, %%mm1 \n\t"
		"pcmpgtw %%mm5, %%mm2 \n\t"
		"pcmpgtw %%mm6, %%mm3 \n\t"
		"pxor %%mm1, %%mm4 \n\t"
		"pxor %%mm2, %%mm5 \n\t"
		"pxor %%mm3, %%mm6 \n\t"
		"psubw %%mm1, %%mm4 \n\t"
		"psubw %%mm2, %%mm5 \n\t"
		"psubw %%mm3, %%mm6 \n\t"
		"movq %%mm4, 8(%%edx) \n\t"
		"movq %%mm5, 24(%%edx) \n\t"
		"movq %%mm6, 40(%%edx) \n\t"

		"emms \n\t"
		: 
		: "S" (old), "D" (nw), "a" (os), "b" (ns), "d" (out)
		: "memory"
		);
	m->p = m->t = m->s = 0;
	for (i=0; i<8; i++) {
		m->p += out[i];
		m->t += out[8+i];
		m->s += out[16+i];
	}
	//printf("e=%d o=%d d=%d p=%d t=%d s=%d\n", m->e, m->o, m->d, m->p, m->t, m->s);
}
#endif
#endif
