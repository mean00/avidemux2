/***************************************************************************
                          ADM_vidDecTelecide  -  description
                             -------------------
    
    email                : fixounet@free.fr

    Port of Donal Graft Decimate which is (c) Donald Graft
    http://www.neuron2.net
    http://puschpull.org/avisynth/decomb_reference_manual.html

 ***************************************************************************/

/*
	Decimate plugin for Avisynth -- performs 1-in-N
	decimation on a stream of progressive frames, which are usually
	obtained from the output of my Telecide plugin for Avisynth.
	For each group of N successive frames, this filter deletes the
	frame that is most similar to its predecessor. Thus, duplicate
	frames coming out of Telecide can be removed using Decimate. This
	filter adjusts the frame rate of the clip as
	appropriate. Selection of the cycle size is selected by specifying
	a parameter to Decimate() in the Avisynth scipt.

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

	The author can be contacted at:
	Donald Graft
	neuron2@attbi.com.
*/

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"

#define aprintf(...) {}
#include "DIA_factory.h"


#define PROGRESSIVE  0x00000001
#define MAGIC_NUMBER (0xdeadbeef)
#define IN_PATTERN   0x00000002

extern uint8_t 	PutHintingData(unsigned char *video, unsigned int hint);
extern uint8_t 	GetHintingData(unsigned char *video, unsigned int *hint);
extern void 	BitBlt(uint8_t * dstp, int dst_pitch, const uint8_t* srcp,
            		int src_pitch, int row_size, int height);

#define DrawString drawString

#ifdef USE_SSE
	#define DECIMATE_MMX_BUILD_PLANE 1
	#define DECIMATE_MMX_BUILD	 1
#endif

#ifdef DECIMATE_MMX_BUILD_PLANE
static void isse_blend_decimate_plane(uint8_t * dst, uint8_t* src,  uint8_t* src_next, 
			int w, int h);
int isse_scenechange_32(const uint8_t *c_plane,const  uint8_t *tplane, int height, int width) ;	
int isse_scenechange_16(const uint8_t *c_plane,const  uint8_t *tplane, int height, int width) ;
int isse_scenechange_8(const uint8_t *c_plane,const  uint8_t *tplane, int height, int width) ;
#endif


#define OutputDebugString(x) aprintf("%s\n",x)
//________________________________
#define MAX_CYCLE_SIZE 25
#define MAX_BLOCKS 50

#define GETFRAME(g, fp) \
{ \
	int GETFRAMEf; \
	GETFRAMEf = (g); \
	if (GETFRAMEf < 0) GETFRAMEf = 0; \
	if (GETFRAMEf > num_frames_hi - 1) GETFRAMEf = num_frames_hi - 1; \
	(fp) = vidCache->getImage(GETFRAMEf); \
}
//________________________________
#include "ADM_vidDecDec_param.h"






#define BLKSIZE 32
//________________________________
/* Decimate 1-in-N implementation. */
class Decimate : public AVDMGenericVideoStream
{
	int 			num_frames_hi;
	
	DECIMATE_PARAM 		*_param;
	
	int last_request, last_result;
	bool last_forced;
	double last_metric;
	double metrics[MAX_CYCLE_SIZE];
	double showmetrics[MAX_CYCLE_SIZE];
	int Dprev[MAX_CYCLE_SIZE];
	int Dcurr[MAX_CYCLE_SIZE];
	int Dnext[MAX_CYCLE_SIZE];
	int Dshow[MAX_CYCLE_SIZE];
	unsigned int hints[MAX_CYCLE_SIZE];
	bool hints_invalid;
	bool all_video_cycle;
	bool firsttime;
	int heightY, row_sizeY, pitchY;
	int heightUV, row_sizeUV, pitchUV;
	int pitch, row_size, height;
	int xblocks, yblocks;
	unsigned int *sum, div;
	bool debug, show;
	
	VideoCache	*vidCache;
	
public:
				
			Decimate(AVDMGenericVideoStream *in,CONFcouple *couples);    
			~Decimate(void);
	uint8_t  	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
				ADMImage *data,uint32_t *flags);

    	uint8_t   	*GetFrame(int n);
	void   		DrawShow(ADMImage  *src, int useframe, bool forced, int dropframe,
		                              double metric, int inframe );
        void   		FindDuplicate(int frame, int *chosen, double *metric, bool *forced   );
    	void   		FindDuplicate2(int frame, int *chosen, bool *forced );
    	void   		FindDuplicateYUY2(int frame, int *chosen, double *metric, bool *force);
    	void   		FindDuplicate2YUY2(int frame, int *chosen, bool *forced );
	
	char 		*printConf( void );
	uint8_t 	configure(AVDMGenericVideoStream *in);
	uint8_t		getCoupledConf( CONFcouple **couples);
};


//********** Register chunk ************

static FILTER_PARAM decdecParam={5,{"cycle","mode","quality","threshold","threshold2"}};
VF_DEFINE_FILTER(Decimate,decdecParam,
                decimate,
                QT_TR_NOOP("Decomb Decimate"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Useful to remove dupes left by Telecide."));


uint8_t Decimate::configure(AVDMGenericVideoStream *in)
{
	_in=in;
#define PX(x) &(_param->x)
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry *)
        
    ELEM_TYPE_FLOAT t1=(ELEM_TYPE_FLOAT)_param->threshold;
    ELEM_TYPE_FLOAT t2=(ELEM_TYPE_FLOAT)_param->threshold2;

         diaMenuEntry tMode[]={
                             {0, QT_TR_NOOP("Discard closer"),NULL},
                             {1, QT_TR_NOOP("Replace (interpolate)"),NULL},
                             {2, QT_TR_NOOP("Discard longer dupe (animés)"),NULL},
                             {3, QT_TR_NOOP("Pulldown dupe removal"),NULL}
                          };
         diaMenuEntry tQuality[]={
                             {0, QT_TR_NOOP("Fastest (no chroma, partial luma)"),NULL},
                             {1, QT_TR_NOOP("Fast (partial luma and chroma)"),NULL},
                             {2, QT_TR_NOOP("Medium (full luma, no chroma)"),NULL},
                             {3, QT_TR_NOOP("Slow (full luma and chroma)"),NULL}
                          };
  
    
    diaElemMenu menuMode(PX(mode),QT_TR_NOOP("_Mode:"), 4,tMode);
    diaElemMenu menuQuality(PX(quality),QT_TR_NOOP("_Quality:"), 4,tQuality);
    diaElemFloat menuThresh1(&t1,QT_TR_NOOP("_Threshold 1:"),0,100.);
    diaElemFloat menuThresh2(&t2,QT_TR_NOOP("T_hreshold 2:"),0,100.);
    diaElemUInteger cycle(PX(cycle),QT_TR_NOOP("C_ycle:"),2,40);
    
    diaElem *elems[]={&cycle,&menuMode,&menuQuality,&menuThresh1,&menuThresh2};
    
  if(diaFactoryRun(QT_TR_NOOP("Decomb Decimate"),5,elems))
  {
    _param->threshold=(double )t1;
    _param->threshold2=(double )t2;
    return 1; 
  }
  return 0;        
        
        
}

char *Decimate::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Decomb Decimate cycle:%d",_param->cycle);
 
}


Decimate::Decimate(AVDMGenericVideoStream *in,CONFcouple *couples)		
{
{
		
		int count = 0;
		char buf[80];
		unsigned int *p;

		_in=in;		
   		memcpy(&_info,_in->getInfo(),sizeof(_info));    
  		_info.encoding=1;
		_uncompressed=NULL;		
  		_info.encoding=1;
		
		//		
		// Init here
		debug=0;
		show=0;		
#ifdef USE_SSE	
		if(CpuCaps::hasSSE())
		{
			printf("Decimate:SSE enabled\n");
		}
#endif
		//
		_param=new DECIMATE_PARAM;
		if(couples)
		{
			GET(cycle);
			GET(mode);
			GET(quality);
			GET(threshold);
			GET(threshold2);
			
		}
		else // Default
  		{
			_param->cycle=5;
			_param->mode=0;
			_param->quality=2;
			_param->threshold=0;
			_param->threshold2=3.0;
		}
		
		ADM_assert(_param->cycle);
		vidCache=new VideoCache(_param->cycle*2+1,in);
		
		if (_param->mode == 0 || _param->mode == 2 || _param->mode == 3)
		{
			num_frames_hi = _info.nb_frames;
			_info.nb_frames = _info.nb_frames * (_param->cycle - 1) / _param->cycle;
			_info.fps1000=_info.fps1000*(_param->cycle-1);
			_info.fps1000=(uint32_t)(_info.fps1000/_param->cycle);
			
		}
		last_request = -1;
		firsttime = true;
		sum = (unsigned int *) ADM_alloc(MAX_BLOCKS * MAX_BLOCKS * sizeof(unsigned int));
		ADM_assert(sum);		
		all_video_cycle = true;

		if (debug)
		{
			char b[80];
			sprintf(b, "Decimate %s by Donald Graft, Copyright 2003\n", 0); // VERSION
			OutputDebugString(b);
		}
	}
}
//________________________________________________________
uint8_t	Decimate::getCoupledConf( CONFcouple **couples)
{
	*couples=NULL;
	*couples=new CONFcouple(5);
#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
	
	CSET(cycle);
	CSET(mode);
	CSET(quality);
	CSET(threshold);
	CSET(threshold2);

	return 1;
}
//________________________________________________________
Decimate::~Decimate(void)
{
		if (sum != NULL) ADM_dealloc(sum);
		if(vidCache) delete vidCache;
		if(_param) delete _param;

		vidCache=NULL;
		_param=NULL;
		sum=NULL;
}
//________________________________________________________
void Decimate::DrawShow(ADMImage  *src, int useframe, bool forced, int dropframe,
						double metric, int inframe)
{
	char buf[80];
	int start = (useframe / _param->cycle) * _param->cycle;

	if (show == true)
	{
		sprintf(buf, "Decimate %s", 0); // VERSION
		DrawString(src, 0, 0, buf);
		sprintf(buf, "Copyright 2003 Donald Graft");
		DrawString(src, 0, 1, buf);
		sprintf(buf,"%d: %3.2f", start, showmetrics[0]);
		DrawString(src, 0, 3, buf);
		sprintf(buf,"%d: %3.2f", start + 1, showmetrics[1]);
		DrawString(src, 0, 4, buf);
		sprintf(buf,"%d: %3.2f", start + 2, showmetrics[2]);
		DrawString(src, 0, 5, buf);
		sprintf(buf,"%d: %3.2f", start + 3, showmetrics[3]);
		DrawString(src, 0, 6, buf);
		sprintf(buf,"%d: %3.2f", start + 4, showmetrics[4]);
		DrawString(src, 0, 7, buf);
		if (all_video_cycle == false)
		{
			sprintf(buf,"in frm %d, use frm %d", inframe, useframe);
			DrawString(src, 0, 8, buf);
			if (forced == false)
				sprintf(buf,"chose %d, dropping", dropframe);
			else
				sprintf(buf,"chose %d, dropping, forced!", dropframe);
			DrawString(src, 0, 9, buf);
		}
		else
		{
			sprintf(buf,"in frm %d", inframe);
			DrawString(src, 0, 8, buf);
			sprintf(buf,"chose %d, decimating all-video cycle", dropframe);
			DrawString(src, 0, 9, buf);
		}
	}
	if (debug)
	{
		if (!(inframe%_param->cycle))
		{
			sprintf(buf,"Decimate: %d: %3.2f\n", start, showmetrics[0]);
			OutputDebugString(buf);
			sprintf(buf,"Decimate: %d: %3.2f\n", start + 1, showmetrics[1]);
			OutputDebugString(buf);
			sprintf(buf,"Decimate: %d: %3.2f\n", start + 2, showmetrics[2]);
			OutputDebugString(buf);
			sprintf(buf,"Decimate: %d: %3.2f\n", start + 3, showmetrics[3]);
			OutputDebugString(buf);
			sprintf(buf,"Decimate: %d: %3.2f\n", start + 4, showmetrics[4]);
			OutputDebugString(buf);
		}
		if (all_video_cycle == false)
		{
			sprintf(buf,"Decimate: in frm %d useframe %d\n", inframe, useframe);
			OutputDebugString(buf);
			if (forced == false)
				sprintf(buf,"Decimate: chose %d, dropping\n", dropframe);
			else
				sprintf(buf,"Decimate: chose %d, dropping, forced!\n", dropframe);
			OutputDebugString(buf);
		}
		else
		{
			sprintf(buf,"Decimate: in frm %d\n", inframe);
			OutputDebugString(buf);
			sprintf(buf,"Decimate: chose %d, decimating all-video cycle\n", dropframe);
			OutputDebugString(buf);
		}
	}
}
//______________________________________________________________________
uint8_t Decimate::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
				ADMImage *data,uint32_t *flags)
{
	int dropframe, useframe, nextfrm, wY, wUV, hY, hUV, x, y, pitchY, pitchUV, dpitchY, dpitchUV;
	ADMImage  *src, *next, *dst;
	unsigned char *srcrpY, *nextrpY, *dstwpY;
	unsigned char *srcrpU, *nextrpU, *dstwpU;
	unsigned char *srcrpV, *nextrpV, *dstwpV;
	uint32_t inframe=frame;
	double metric;
	char buf[255];

        if(frame>= _info.nb_frames) return 0;
	*len=(_info.width*_info.height*3)>>1;
	num_frames_hi = _in->getInfo()->nb_frames; /* FIXME MEANX */
	if (_param->mode == 0)
	{
		bool forced = false;
		int start;

		/* Normal decimation. Remove the frame most similar to its preceding frame. */
		/* Determine the correct frame to use and get it. */
		useframe = inframe + inframe / (_param->cycle - 1);
		start = (useframe /  _param->cycle) * _param->cycle;
		FindDuplicate((useframe / _param->cycle) * _param->cycle, &dropframe, &metric, &forced);
		if (useframe >= dropframe) useframe++;
		GETFRAME(useframe, src);
		if (show == true)
		{
			sprintf(buf, "Decimate %s", 0);
			DrawString(src, 0, 0, buf);
			sprintf(buf, "Copyright 2003 Donald Graft");
			DrawString(src, 0, 1, buf);
			sprintf(buf,"%d: %3.2f", start, showmetrics[0]);
			DrawString(src, 0, 3, buf);
			sprintf(buf,"%d: %3.2f", start + 1, showmetrics[1]);
			DrawString(src, 0, 4, buf);
			sprintf(buf,"%d: %3.2f", start + 2, showmetrics[2]);
			DrawString(src, 0, 5, buf);
			sprintf(buf,"%d: %3.2f", start + 3, showmetrics[3]);
			DrawString(src, 0, 6, buf);
			sprintf(buf,"%d: %3.2f", start + 4, showmetrics[4]);
			DrawString(src, 0, 7, buf);
			sprintf(buf,"in frm %d, use frm %d", inframe, useframe);
			DrawString(src, 0, 8, buf);
			sprintf(buf,"dropping frm %d%s", dropframe, last_forced == true ? ", forced!" : "");
			DrawString(src, 0, 9, buf);
		}
		if (debug)
		{	
			if (!(inframe % _param->cycle))
			{
				sprintf(buf,"Decimate: %d: %3.2f\n", start, showmetrics[0]);
				OutputDebugString(buf);
				sprintf(buf,"Decimate: %d: %3.2f\n", start + 1, showmetrics[1]);
				OutputDebugString(buf);
				sprintf(buf,"Decimate: %d: %3.2f\n", start + 2, showmetrics[2]);
				OutputDebugString(buf);
				sprintf(buf,"Decimate: %d: %3.2f\n", start + 3, showmetrics[3]);
				OutputDebugString(buf);
				sprintf(buf,"Decimate: %d: %3.2f\n", start + 4, showmetrics[4]);
				OutputDebugString(buf);
			}
			sprintf(buf,"Decimate: in frm %d, use frm %d\n", inframe, useframe);
			OutputDebugString(buf);
			sprintf(buf,"Decimate: dropping frm %d%s\n", dropframe, last_forced == true ? ", forced!" : "");
			OutputDebugString(buf);
		}
	    //return src;
	        //memcpy(data,src,*len);

		data->duplicate(src);
		vidCache->unlockAll();
		  
		return 1;
	}
	else if (_param->mode == 1)
	{
		bool forced = false;
		int start = (inframe / _param->cycle) * _param->cycle;
		unsigned int hint, film = 1;

		GETFRAME(inframe, src);
	    	srcrpY = YPLANE(src); //(unsigned char *) src->GetReadPtr(PLANAR_Y);
		if (GetHintingData(srcrpY, &hint) == false)
		{
			film = hint & PROGRESSIVE;
//			if (film) OutputDebugString("film\n");
//			else OutputDebugString("video\n");
		}

		/* Find the most similar frame as above but replace it with a blend of
		   the preceding and following frames. */
		num_frames_hi = _in->getInfo()->nb_frames; /* FIXME MEANX */
		FindDuplicate((inframe / _param->cycle) * _param->cycle, &dropframe, &metric, &forced);
		if (!film || inframe != dropframe || (_param->threshold && metric > _param->threshold))
		{
			if (show == true)
			{

				sprintf(buf, "Decimate %s", 0);
				DrawString(src, 0, 0, buf);
				sprintf(buf, "Copyright 2003 Donald Graft");
				DrawString(src, 0, 1, buf);
				sprintf(buf,"%d: %3.2f", start, showmetrics[0]);
				DrawString(src, 0, 3, buf);
				sprintf(buf,"%d: %3.2f", start + 1, showmetrics[1]);
				DrawString(src, 0, 4, buf);
				sprintf(buf,"%d: %3.2f", start + 2, showmetrics[2]);
				DrawString(src, 0, 5, buf);
				sprintf(buf,"%d: %3.2f", start + 3, showmetrics[3]);
				DrawString(src, 0, 6, buf);
				sprintf(buf,"%d: %3.2f", start + 4, showmetrics[4]);
				DrawString(src, 0, 7, buf);
				sprintf(buf,"infrm %d", inframe);
				DrawString(src, 0, 8, buf);
				if (last_forced == false)
					sprintf(buf,"chose %d, passing through", dropframe);
				else
					sprintf(buf,"chose %d, passing through, forced!", dropframe);
				DrawString(src, 0, 9, buf);
			}
			if (debug)
			{
				if (!(inframe % _param->cycle))
				{
					sprintf(buf,"Decimate: %d: %3.2f\n", start, showmetrics[0]);
					OutputDebugString(buf);
					sprintf(buf,"Decimate: %d: %3.2f\n", start + 1, showmetrics[1]);
					OutputDebugString(buf);
					sprintf(buf,"Decimate: %d: %3.2f\n", start + 2, showmetrics[2]);
					OutputDebugString(buf);
					sprintf(buf,"Decimate: %d: %3.2f\n", start + 3, showmetrics[3]);
					OutputDebugString(buf);
					sprintf(buf,"Decimate: %d: %3.2f\n", start + 4, showmetrics[4]);
					OutputDebugString(buf);
				}
				sprintf(buf,"Decimate: in frm %d\n", inframe);
				OutputDebugString(buf);
				if (last_forced == false)
					sprintf(buf,"Decimate: chose %d, passing through\n", dropframe);
				else
					sprintf(buf,"Decimate: chose %d, passing through, forced!\n", dropframe);
				OutputDebugString(buf);
			}
			//return src;
			//memcpy(data,src,*len);

			data->duplicate(src);
			vidCache->unlockAll();
			return 1;
		}
		if (inframe < _in->getInfo()->nb_frames - 1) /* FIXME MEANX*/
			nextfrm = inframe + 1;
		else
			nextfrm = _in->getInfo()->nb_frames - 1;
		if (debug)
		{
			if (!(inframe % _param->cycle))
			{
				sprintf(buf,"Decimate: %d: %3.2f\n", start, showmetrics[0]);
				OutputDebugString(buf);
				sprintf(buf,"Decimate: %d: %3.2f\n", start + 1, showmetrics[1]);
				OutputDebugString(buf);
				sprintf(buf,"Decimate: %d: %3.2f\n", start + 2, showmetrics[2]);
				OutputDebugString(buf);
				sprintf(buf,"Decimate: %d: %3.2f\n", start + 3, showmetrics[3]);
				OutputDebugString(buf);
				sprintf(buf,"Decimate: %d: %3.2f\n", start + 4, showmetrics[4]);
				OutputDebugString(buf);
			}
			sprintf(buf,"Decimate: in frm %d\n", inframe);
			OutputDebugString(buf);
			if (last_forced == false)
				sprintf(buf,"Decimate: chose %d, blending %d and %d\n", dropframe, inframe, nextfrm);
			else
				sprintf(buf,"Decimate: chose %d, blending %d and %d, forced!\n", dropframe, inframe, nextfrm);
			OutputDebugString(buf);
		}
		GETFRAME(nextfrm, next);
		dst = data; //env->NewVideoFrame(vi);
		pitchY = _info.width; //src->GetPitch(PLANAR_Y);
		dpitchY = _info.width; //dst->GetPitch(PLANAR_Y);
		wY = _info.width; //src->GetRowSize(PLANAR_Y);
		hY = _info.height; //src->GetHeight(PLANAR_Y);
		pitchUV = _info.width>>1;// src->GetPitch(PLANAR_V);
		dpitchUV =_info.width>>1;// dst->GetPitch(PLANAR_V);
		wUV = _info.width>>1;//src->GetRowSize(PLANAR_V);
		hUV = _info.height>>1;//src->GetHeight(PLANAR_V);
		
		nextrpY = YPLANE(next); //next->GetReadPtr(PLANAR_Y);
		dstwpY = YPLANE( dst); //dst->GetWritePtr(PLANAR_Y);
#ifdef DECIMATE_MMX_BUILD_PLANE
		if (CpuCaps::hasSSE()) 
		{
			isse_blend_decimate_plane(dstwpY, srcrpY, nextrpY, wY, hY);
		} else {
#endif
			for (y = 0; y < hY; y++)
			{
				for (x = 0; x < wY; x++)
				{
					dstwpY[x] = ((int)srcrpY[x] + (int)nextrpY[x] ) >> 1;  
				}
				srcrpY += pitchY;
				nextrpY += pitchY;
				dstwpY += dpitchY;
			}
#ifdef DECIMATE_MMX_BUILD_PLANE
		}
#endif
		srcrpU =   UPLANE(src);//->GetReadPtr(PLANAR_U);
		nextrpU =   UPLANE(next);//->GetReadPtr(PLANAR_U);
		dstwpU =  UPLANE(dst);//->GetWritePtr(PLANAR_U);
#ifdef DECIMATE_MMX_BUILD_PLANE
		if (CpuCaps::hasSSE()) 
		{
			isse_blend_decimate_plane(dstwpU, srcrpU, nextrpU, wUV, hUV);
		} else {
#endif
			for (y = 0; y < hUV; y++)
			{
				for (x = 0; x < wUV; x++)
				{
					dstwpU[x] = ((int)srcrpU[x] + (int)nextrpU[x]) >> 1;
				}
				srcrpU += pitchUV;
				nextrpU += pitchUV;
				dstwpU += dpitchUV;
			}
#ifdef DECIMATE_MMX_BUILD_PLANE
		}
#endif
		srcrpV =   VPLANE(src);//->GetReadPtr(PLANAR_V);
		nextrpV =   VPLANE(next);//->GetReadPtr(PLANAR_V);
		dstwpV =   VPLANE(dst);//->GetWritePtr(PLANAR_V);

#ifdef DECIMATE_MMX_BUILD_PLANE
		if (CpuCaps::hasSSE()) { 
			isse_blend_decimate_plane(dstwpV, srcrpV, nextrpV, wUV, hUV );
		} else {
#endif
			for (y = 0; y < hUV; y++)
			{
				for (x = 0; x < wUV; x++)
				{
					dstwpV[x] = ((int)srcrpV[x] + + (int)nextrpV[x]) >> 1;
				}
				srcrpV += pitchUV;
				nextrpV += pitchUV;
				dstwpV += dpitchUV;
			}
#ifdef DECIMATE_MMX_BUILD_PLANE
		}
#endif
		if (show == true)
		{

			sprintf(buf, "Decimate %s", 0);
			DrawString(dst, 0, 0, buf);
			sprintf(buf, "Copyright 2003 Donald Graft");
			DrawString(dst, 0, 1, buf);
			sprintf(buf,"%d: %3.2f", start, showmetrics[0]);
			DrawString(dst, 0, 3, buf);
			sprintf(buf,"%d: %3.2f", start + 1, showmetrics[1]);
			DrawString(dst, 0, 4, buf);
			sprintf(buf,"%d: %3.2f", start + 2, showmetrics[2]);
			DrawString(dst, 0, 5, buf);
			sprintf(buf,"%d: %3.2f", start + 3, showmetrics[3]);
			DrawString(dst, 0, 6, buf);
			sprintf(buf,"%d: %3.2f", start + 4, showmetrics[4]);
			DrawString(dst, 0, 7, buf);
			sprintf(buf,"infrm %d", inframe);
			DrawString(dst, 0, 8, buf);
			if (last_forced == false)
				sprintf(buf,"chose %d, blending %d and %d",dropframe, inframe, nextfrm);
			else
				sprintf(buf,"chose %d, blending %d and %d, forced!", dropframe, inframe, nextfrm);
			DrawString(dst, 0, 9, buf);
		}
		//return dst;
		//memcpy(data,dst,*len);

		data->duplicate(dst);
		vidCache->unlockAll();		
		return 1;
	}
	else if (_param->mode == 2)
	{
		bool forced = false;

		/* Delete the duplicate in the longest string of duplicates. */
		useframe = inframe + inframe / (_param->cycle - 1);
		FindDuplicate2((useframe / _param->cycle) * _param->cycle, &dropframe, &forced);
		if (useframe >= dropframe) useframe++;
		GETFRAME(useframe, src);
		if (show == true)
		{
			int start = (useframe / _param->cycle) * _param->cycle;


			sprintf(buf, "Decimate %s", 0);
			DrawString(src, 0, 0, buf);
			sprintf(buf, "Copyright 2003 Donald Graft");
			DrawString(src, 0, 1, buf);
			sprintf(buf,"in frm %d, use frm %d", inframe, useframe);
			DrawString(src, 0, 3, buf);
			sprintf(buf,"%d: %3.2f (%s)", start, showmetrics[0],
					Dshow[0] ? "new" : "dup");
			DrawString(src, 0, 4, buf);
			sprintf(buf,"%d: %3.2f (%s)", start + 1, showmetrics[1],
					Dshow[1] ? "new" : "dup");
			DrawString(src, 0, 5, buf);
			sprintf(buf,"%d: %3.2f (%s)", start + 2, showmetrics[2],
					Dshow[2] ? "new" : "dup");
			DrawString(src, 0, 6, buf);
			sprintf(buf,"%d: %3.2f (%s)", start + 3, showmetrics[3],
					Dshow[3] ? "new" : "dup");
			DrawString(src, 0, 7, buf);
			sprintf(buf,"%d: %3.2f (%s)", start + 4, showmetrics[4],
					Dshow[4] ? "new" : "dup");
			DrawString(src, 0, 8, buf);
			sprintf(buf,"Dropping frm %d%s", dropframe, last_forced == true ? " forced!" : "");
			DrawString(src, 0, 9, buf);
		}
		if (debug)
		{	
			sprintf(buf,"Decimate: inframe %d useframe %d\n", inframe, useframe);
			OutputDebugString(buf);
		}
	    //return src;
	    	//memcpy(data,src,*len);

		data->duplicate(src);
		vidCache->unlockAll();
		return 1;
	}
	else if (_param->mode == 3)
	{
		bool forced = false;

		/* Decimate by removing a duplicate from film cycles and doing a
		   blend rate conversion on the video cycles. */
		if (_param->cycle != 5)//	env->ThrowError("Decimate: mode=3 requires cycle=5");
		{
			printf("Decimate: mode=3 requires cycle=5\n");
			return 0;
		}
		useframe = inframe + inframe / (_param->cycle - 1);
		FindDuplicate((useframe / _param->cycle) * _param->cycle, &dropframe, &metric, &forced);
		/* Use hints from Telecide about film versus video. Also use the difference
		   metric of the most similar frame in the cycle; if it exceeds threshold,
		   assume it's a video cycle. */
		if (!(inframe % 4))
		{
			all_video_cycle = false;
			if (_param->threshold && metric > _param->threshold)
			{
				all_video_cycle = true;
			}
			if ((hints_invalid == false) &&
				(!(hints[0] & PROGRESSIVE) ||
				 !(hints[1] & PROGRESSIVE) ||
				 !(hints[2] & PROGRESSIVE) ||
				 !(hints[3] & PROGRESSIVE) ||
				 !(hints[4] & PROGRESSIVE)))
			{
				all_video_cycle = true;
			}
		}
		if (all_video_cycle == false)
		{
			/* It's film, so decimate in the normal way. */
			if (useframe >= dropframe) useframe++;
			GETFRAME(useframe, src);
			DrawShow(src, useframe, forced, dropframe, metric, inframe);			
			//memcpy(data,src,*len);

			data->duplicate(src);
		
			vidCache->unlockAll();		
			return 1; // return src;
		}
		else if ((inframe % 4) == 0)
		{
			/* It's a video cycle. Output the first frame of the cycle. */
			GETFRAME(useframe, src);
			DrawShow(src, 0, forced, dropframe, metric, inframe);
			//return src;
			//memcpy(data,src,*len);

			data->duplicate(src);
		
			vidCache->unlockAll();		
			return 1; // return src;
		}
		else if ((inframe % 4) == 3)
		{
			/* It's a video cycle. Output the last frame of the cycle. */
			GETFRAME(useframe+1, src);
			DrawShow(src, 0, forced, dropframe, metric, inframe);
			//return src;
			//memcpy(data,src,*len);

			data->duplicate(src);
		
			vidCache->unlockAll();		
			return 1; // return src;
		}
		else if ((inframe % 4) == 1 || (inframe % 4) == 2)
		{
			/* It's a video cycle. Make blends for the remaining frames. */
			if ((inframe % 4) == 1)
			{
				GETFRAME(useframe, src);
				if (useframe < num_frames_hi - 1)
					nextfrm = useframe + 1;
				else
					nextfrm = _in->getInfo()->nb_frames - 1;
				GETFRAME(nextfrm, next);
			}
			else
			{
				GETFRAME(useframe + 1, src);
				nextfrm = useframe;
				GETFRAME(nextfrm, next);
			}
			dst = data; //env->NewVideoFrame(vi);
			pitchY = _info.width; //src->GetPitch(PLANAR_Y);
			dpitchY = _info.width; //dst->GetPitch(PLANAR_Y);
			wY = _info.width; //src->GetRowSize(PLANAR_Y);
			hY = _info.height; //src->GetHeight(PLANAR_Y);
			pitchUV = _info.width>>1; //src->GetPitch(PLANAR_V);
			dpitchUV =_info.width>>1; // dst->GetPitch(PLANAR_V);
			wUV = _info.width>>1; //src->GetRowSize(PLANAR_V);
			hUV = _info.height>>1; //src->GetHeight(PLANAR_V);
			
			srcrpY = YPLANE( src); //src->GetReadPtr(PLANAR_Y);
			nextrpY = YPLANE( next); //next->GetReadPtr(PLANAR_Y);
			dstwpY = YPLANE( dst); //dst->GetWritePtr(PLANAR_Y);
#ifdef DECIMATE_MMX_BUILD_PLANE
			if (CpuCaps::hasSSE()) { 
				isse_blend_decimate_plane(dstwpY, srcrpY, nextrpY, wY, hY);
			} else {
#endif
				for (y = 0; y < hY; y++)
				{
					for (x = 0; x < wY; x++)
					{
						dstwpY[x] = ((int)srcrpY[x] + (int)nextrpY[x]) >> 1;
					}
					srcrpY += pitchY;
					nextrpY += pitchY;
					dstwpY += dpitchY;
				}
#ifdef DECIMATE_MMX_BUILD_PLANE
			}
#endif
			srcrpU =   UPLANE(src);//->GetReadPtr(PLANAR_U);
			nextrpU =  UPLANE( next);//->GetReadPtr(PLANAR_U);
			dstwpU =   UPLANE(dst);//->GetWritePtr(PLANAR_U);
#ifdef DECIMATE_MMX_BUILD_PLANE
			if (CpuCaps::hasSSE()) { 
				isse_blend_decimate_plane(dstwpU, srcrpU, nextrpU, wUV, hUV);
			} else {
#endif
				for (y = 0; y < hUV; y++)
				{
					for (x = 0; x < wUV; x++)
					{
						dstwpU[x] = ((int)srcrpU[x] + (int)nextrpU[x]) >> 1;
					}
					srcrpU += pitchUV;
					nextrpU += pitchUV;
					dstwpU += dpitchUV;
				}
#ifdef DECIMATE_MMX_BUILD_PLANE
			}
#endif
			srcrpV =   VPLANE(src);//->GetReadPtr(PLANAR_V);
			nextrpV =  VPLANE( next);//->GetReadPtr(PLANAR_V);
			dstwpV =   VPLANE(dst);//->GetWritePtr(PLANAR_V);
#ifdef DECIMATE_MMX_BUILD_PLANE
			if (CpuCaps::hasSSE()) { 
				isse_blend_decimate_plane(dstwpV, srcrpV, nextrpV, wUV, hUV);
			} else {
#endif
				for (y = 0; y < hUV; y++)
				{
					for (x = 0; x < wUV; x++)
					{
						dstwpV[x] = ((int)srcrpV[x] + (int)nextrpV[x]) >> 1;
					}
					srcrpV += pitchUV;
					nextrpV += pitchUV;
					dstwpV += dpitchUV;
				}
#ifdef DECIMATE_MMX_BUILD_PLANE
			}
#endif
			DrawShow(dst, 0, forced, dropframe, metric, inframe);
			vidCache->unlockAll();
			//return dst;
			//memcpy(data,dst,*len);

			data->duplicate(dst);
			vidCache->unlockAll();		
			return 1; // return src;			
		}
		//return src;
		//memcpy(data,src,*len);

                GETFRAME(useframe, src); // MEANX : not sure (jw detected a problem here)
		data->duplicate(src);
		vidCache->unlockAll();		
		return 1; // return src;			
	}
	//env->ThrowError("Decimate: invalid mode option (0-3)");
	printf("Decimate: invalid mode option (0-3)\n");
	/* Avoid compiler warning. */
	return 0;
}
//____________________________________________________
void Decimate::FindDuplicate(int frame, int *chosen, double *metric, bool *forced)
{
	int f;
	ADMImage  * store[MAX_CYCLE_SIZE+1];
	const unsigned char *storepY[MAX_CYCLE_SIZE+1];
	const unsigned char *storepU[MAX_CYCLE_SIZE+1];
	const unsigned char *storepV[MAX_CYCLE_SIZE+1];
	const unsigned char *prevY, *prevU, *prevV, *currY, *currU, *currV;
	int x, y, lowest_index, div;
	unsigned int count[MAX_CYCLE_SIZE], lowest;
	bool found;
	unsigned int highest_sum=0;

	/* Only recalculate differences when a new set is needed. */
	if (frame == last_request)
	{
		*chosen = last_result;
		*metric = last_metric;
		return;
	}
	last_request = frame;

	/* Get cycle+1 frames starting at the one before the asked-for one. */
	for (f = 0; f <= _param->cycle; f++)
	{
		GETFRAME(frame + f - 1, store[f]);
		storepY[f] = YPLANE(store[f]);//->GetReadPtr(PLANAR_Y);
		hints_invalid = GetHintingData((unsigned char *) storepY[f], &hints[f]);
		if (_param->quality == 1 || _param->quality == 3)
		{
			storepU[f] = UPLANE(store[f]);//->GetReadPtr(PLANAR_U);
			storepV[f] = VPLANE(store[f]);//->GetReadPtr(PLANAR_V);
		}
	}

    pitchY = _info.width; //store[0]->GetPitch(PLANAR_Y);
    row_sizeY = _info.width; //store[0]->GetRowSize(PLANAR_Y);
    heightY = _info.height; //store[0]->GetHeight(PLANAR_Y);
	if (_param->quality == 1 || _param->quality == 3)
	{
		pitchUV = _info.width>>1; //store[0]->GetPitch(PLANAR_V);
		row_sizeUV = _info.width>>1;//store[0]->GetRowSize(PLANAR_V);
		heightUV = _info.height>>1;//store[0]->GetHeight(PLANAR_V);
	}

	int use_quality=_param->quality;


	switch (use_quality)
	{
	case 0: // subsample, luma only
		div = (BLKSIZE * BLKSIZE / 4) * 219;
		break;
	case 1: // subsample, luma and chroma
		div = (BLKSIZE * BLKSIZE / 4) * 219 + ( (BLKSIZE * BLKSIZE / 8)) * 224;
		break;
	case 2: // fully sample, luma only
		div = (BLKSIZE * BLKSIZE) * 219;
		break;
	case 3: // fully sample, luma and chroma
		div = (BLKSIZE * BLKSIZE) * 219 + ( BLKSIZE * BLKSIZE/2) * 224;
		break;
	}

	xblocks = row_sizeY / BLKSIZE;
	if (row_sizeY % BLKSIZE) xblocks++;
	yblocks = heightY / BLKSIZE;
	if (heightY % BLKSIZE) yblocks++;

	/* Compare each frame to its predecessor. */
	for (f = 1; f <= _param->cycle; f++)
	{
		prevY = storepY[f-1];
		currY = storepY[f];
		for (y = 0; y < yblocks; y++)
		{
			for (x = 0; x < xblocks; x++)
			{
				sum[y*xblocks+x] = 0;
			}
		}
		for (y = 0; y < heightY; y++)
		{
			for (x = 0; x < row_sizeY;)
			{
				sum[(y/BLKSIZE)*xblocks+x/BLKSIZE] += abs((int)currY[x] - (int)prevY[x]);
				x++;
				if (_param->quality == 0 || _param->quality == 1)
				{
					if (!(x%4)) x += 12;
				}
			}
			prevY += pitchY;
			currY += pitchY;
		}
		if (_param->quality == 1 || _param->quality == 3)
		{
			prevU = storepU[f-1];
			prevV = storepV[f-1];
			currU = storepU[f];
			currV = storepV[f];
			for (y = 0; y < heightUV; y++)
			{
				for (x = 0; x < row_sizeUV;)
				{
					sum[((2*y)/BLKSIZE)*xblocks+(2*x)/BLKSIZE] += abs((int)currU[x] - (int)prevU[x]);
					sum[((2*y)/BLKSIZE)*xblocks+(2*x)/BLKSIZE] += abs((int)currV[x] - (int)prevV[x]);
					x++;
					if (_param->quality == 1)
					{
						if (!(x%4)) x += 12;
					}
				}
				prevU += pitchUV;
				currU += pitchUV;
				prevV += pitchUV;
				currV += pitchUV;
			}
		}
		highest_sum = 0;
		for (y = 0; y < yblocks; y++)
		{
			for (x = 0; x < xblocks; x++)
			{
				if (sum[y*xblocks+x] > highest_sum)
				{
					highest_sum = sum[y*xblocks+x];
				}
			}
		}
		count[f-1] = highest_sum;
		showmetrics[f-1] = (count[f-1] * 100.0) / div;
	}

	/* Find the frame with the lowest difference count but
	   don't use the artificial duplicate at frame 0. */
	if (frame == 0)
	{
		lowest = count[1];
		lowest_index = 1;
	}
	else
	{
		lowest = count[0];
		lowest_index = 0;
	}
	for (x = 1; x < _param->cycle; x++)
	{
		if (count[x] < lowest)
		{
			lowest = count[x];
			lowest_index = x;
		}
	}
	last_result = frame + lowest_index;
	if (_param->quality == 1 || _param->quality == 3)
		last_metric = (lowest * 100.0) / div;
	else
		last_metric = (lowest * 100.0) / div;
	*chosen = last_result;
	*metric = last_metric;

	
	found = false;
	last_forced = false;	

}
//____________________________________________________
void Decimate::FindDuplicate2(int frame, int *chosen, bool *forced)
{
	int f, g, fsum, bsum, highest, highest_index;
	ADMImage * store[MAX_CYCLE_SIZE+1];
	const unsigned char *storepY[MAX_CYCLE_SIZE+1];
	const unsigned char *storepU[MAX_CYCLE_SIZE+1];
	const unsigned char *storepV[MAX_CYCLE_SIZE+1];
	const unsigned char *prevY, *prevU, *prevV, *currY, *currU, *currV;
	int x, y;
	double lowest;
	unsigned int lowest_index;
	char buf[255];
	unsigned int highest_sum;
	bool found;
#define BLKSIZE 32

	/* Only recalculate differences when a new cycle is started. */
	if (frame == last_request)
	{
		*chosen = last_result;
		*forced = last_forced;
		return;
	}
	last_request = frame;

	if (firsttime == true || frame == 0)
	{
		firsttime = false;
		for (f = 0; f < MAX_CYCLE_SIZE; f++) Dprev[f] = -1;
		GETFRAME(frame, store[0]);
		storepY[0] = YPLANE(store[0]);//->GetReadPtr(PLANAR_Y);
		if (_param->quality == 1 || _param->quality == 3)
		{
			storepU[0] = UPLANE(store[0]);//->GetReadPtr(PLANAR_U);
			storepV[0] = VPLANE(store[0]);//->GetReadPtr(PLANAR_V);
		}

		for (f = 1; f <= _param->cycle; f++)
		{
			GETFRAME(frame + f - 1, store[f]);
			storepY[f] =YPLANE( store[f]);//->GetReadPtr(PLANAR_Y);
			if (_param->quality == 1 || _param->quality == 3)
			{
				storepU[f] = UPLANE(store[f]);//->GetReadPtr(PLANAR_U);
				storepV[f] = VPLANE(store[f]);//->GetReadPtr(PLANAR_V);
			}
		}

		pitchY = _info.width; //store[0]->GetPitch(PLANAR_Y);
		row_sizeY = _info.width; //store[0]->GetRowSize(PLANAR_Y);
		heightY = _info.height; //store[0]->GetHeight(PLANAR_Y);
		if (_param->quality == 1 || _param->quality == 3)
		{
			pitchUV = _info.width>>1; //store[0]->GetPitch(PLANAR_V);
			row_sizeUV = _info.width>>1; //store[0]->GetRowSize(PLANAR_V);
			heightUV = _info.height>>1; //store[0]->GetHeight(PLANAR_V);
		}
		switch (_param->quality)
		{
		case 0: // subsample, luma only
			div = (BLKSIZE * BLKSIZE / 4) * 219;
			break;
		case 1: // subsample, luma and chroma
			div = (BLKSIZE * BLKSIZE / 4) * 219 + (BLKSIZE * BLKSIZE / 8) * 224;
			break;
		case 2: // fully sample, luma only
			div = (BLKSIZE * BLKSIZE) * 219;
			break;
		case 3: // fully sample, luma and chroma
			div = (BLKSIZE * BLKSIZE) * 219 + (BLKSIZE * BLKSIZE / 2) * 224;
			break;
		}
		xblocks = row_sizeY / BLKSIZE;
		if (row_sizeY % BLKSIZE) xblocks++;
		yblocks = heightY / BLKSIZE;
		if (heightY % BLKSIZE) yblocks++;

		/* Compare each frame to its predecessor. */
		for (f = 1; f <= _param->cycle; f++)
		{
			for (y = 0; y < yblocks; y++)
			{
				for (x = 0; x < xblocks; x++)
				{
					sum[y*xblocks+x] = 0;
				}
			}
			prevY = storepY[f-1];
			currY = storepY[f];
			for (y = 0; y < heightY; y++)
			{
				for (x = 0; x < row_sizeY;)
				{
					sum[(y/BLKSIZE)*xblocks+x/BLKSIZE] += abs((int)currY[x] - (int)prevY[x]);
					x++;
					if (_param->quality == 0 || _param->quality == 1)
					{
						if (!(x%4)) x += 12;
					}
				}
				prevY += pitchY;
				currY += pitchY;
			}
			if (_param->quality == 1 || _param->quality == 3)
			{
				prevU = storepU[f-1];
				currU = storepU[f];
				prevV = storepV[f-1];
				currV = storepV[f];
				for (y = 0; y < heightUV; y++)
				{
					for (x = 0; x < row_sizeUV;)
					{
						sum[((2*y)/BLKSIZE)*xblocks+(2*x)/BLKSIZE] += abs((int)currU[x] - (int)prevU[x]);
						sum[((2*y)/BLKSIZE)*xblocks+(2*x)/BLKSIZE] += abs((int)currV[x] - (int)prevV[x]);
						x++;
						if (_param->quality == 0 || _param->quality == 1)
						{
							if (!(x%4)) x += 12;
						}
					}
					prevU += pitchUV;
					currU += pitchUV;
					prevV += pitchUV;
					currV += pitchUV;
				}
			}
			highest_sum = 0;
			for (y = 0; y < yblocks; y++)
			{
				for (x = 0; x < xblocks; x++)
				{
					if (sum[y*xblocks+x] > highest_sum)
					{
						highest_sum = sum[y*xblocks+x];
					}
				}
			}
			metrics[f-1] = (highest_sum * 100.0) / div;
		}

		Dcurr[0] = 1;
		for (f = 1; f < _param->cycle; f++)
		{
			if (metrics[f] < _param->threshold2) Dcurr[f] = 0;
			else Dcurr[f] = 1;
		}

		if (debug)
		{
			sprintf(buf,"Decimate: %d: %3.2f %3.2f %3.2f %3.2f %3.2f\n",
					0, metrics[0], metrics[1], metrics[2], metrics[3], metrics[4]);
			OutputDebugString(buf);
		}
	}
 	else if (frame >= num_frames_hi - 1)
	{
		GETFRAME(num_frames_hi - 1, store[0]);
		storepY[0] = YPLANE(store[0]);//->GetReadPtr(PLANAR_Y);
		if (_param->quality == 1 || _param->quality == 3)
		{
			storepU[0] = UPLANE(store[0]);//->GetReadPtr(PLANAR_U);
			storepV[0] = VPLANE(store[0]);//->GetReadPtr(PLANAR_V);
		}
		for (f = 0; f < MAX_CYCLE_SIZE; f++) Dprev[f] = Dcurr[f];
		for (f = 0; f < MAX_CYCLE_SIZE; f++) Dcurr[f] = Dnext[f];
	}
	else
	{
		GETFRAME(frame + _param->cycle - 1, store[0]);
		storepY[0] = YPLANE(store[0]);//->GetReadPtr(PLANAR_Y);
		if (_param->quality == 1 || _param->quality == 3)
		{
			storepU[0] = UPLANE(store[0]);//->GetReadPtr(PLANAR_U);
			storepV[0] = VPLANE(store[0]);//->GetReadPtr(PLANAR_V);
		}
		for (f = 0; f < MAX_CYCLE_SIZE; f++) Dprev[f] = Dcurr[f];
		for (f = 0; f < MAX_CYCLE_SIZE; f++) Dcurr[f] = Dnext[f];
	}
	for (f = 0; f < MAX_CYCLE_SIZE; f++) Dshow[f] = Dcurr[f];
	for (f = 0; f < MAX_CYCLE_SIZE; f++) showmetrics[f] = metrics[f];

	for (f = 1; f <= _param->cycle; f++)
	{
		GETFRAME(frame + f + _param->cycle - 1, store[f]);
		storepY[f] =YPLANE( store[f]);//->GetReadPtr(PLANAR_Y);
		if (_param->quality == 1 || _param->quality == 3)
		{
			storepU[f] = UPLANE(store[f]);//->GetReadPtr(PLANAR_U);
			storepV[f] = VPLANE(store[f]);//->GetReadPtr(PLANAR_V);
		}
	}

	/* Compare each frame to its predecessor. */
	for (f = 1; f <= _param->cycle; f++)
	{
		prevY = storepY[f-1];
		currY = storepY[f];
		for (y = 0; y < yblocks; y++)
		{
			for (x = 0; x < xblocks; x++)
			{
				sum[y*xblocks+x] = 0;
			}
		}
		for (y = 0; y < heightY; y++)
		{
			for (x = 0; x < row_sizeY;)
			{
				sum[(y/BLKSIZE)*xblocks+x/BLKSIZE] += abs((int)currY[x] - (int)prevY[x]);
				x++;
				if (_param->quality == 0 || _param->quality == 1)
				{
					if (!(x%4)) x += 12;
				}
			}
			prevY += pitchY;
			currY += pitchY;
		}
		if (_param->quality == 1 || _param->quality == 3)
		{
			prevU = storepU[f-1];
			currU = storepU[f];
			prevV = storepV[f-1];
			currV = storepV[f];
			for (y = 0; y < heightUV; y++)
			{
				for (x = 0; x < row_sizeUV;)
				{
					sum[((2*y)/BLKSIZE)*xblocks+(2*x)/BLKSIZE] += abs((int)currU[x] - (int)prevU[x]);
					sum[((2*y)/BLKSIZE)*xblocks+(2*x)/BLKSIZE] += abs((int)currV[x] - (int)prevV[x]);
					x++;
					if (_param->quality == 0 || _param->quality == 1)
					{
						if (!(x%4)) x += 12;
					}
				}
				prevU += pitchUV;
				currU += pitchUV;
				prevV += pitchUV;
				currV += pitchUV;
			}
		}
		highest_sum = 0;
		for (y = 0; y < yblocks; y++)
		{
			for (x = 0; x < xblocks; x++)
			{
				if (sum[y*xblocks+x] > highest_sum)
				{
					highest_sum = sum[y*xblocks+x];
				}
			}
		}
		metrics[f-1] = (highest_sum * 100.0) / div;
	}

	/* Find the frame with the lowest difference count but
	   don't use the artificial duplicate at frame 0. */
	if (frame == 0)
	{
		lowest = metrics[1];
		lowest_index = 1;
	}
	else
	{
		lowest = metrics[0];
		lowest_index = 0;
	}
	for (f = 1; f < _param->cycle; f++)
	{
		if (metrics[f] < lowest)
		{
			lowest = metrics[f];
			lowest_index = f;
		}
	}

	for (f = 0; f < _param->cycle; f++)
	{
		if (metrics[f] < _param->threshold2) Dnext[f] = 0;
		else Dnext[f] = 1;
	}

	if (debug)
	{
		sprintf(buf,"Decimate: %d: %3.2f %3.2f %3.2f %3.2f %3.2f\n",
		        frame + 5, metrics[0], metrics[1], metrics[2], metrics[3], metrics[4]);
		OutputDebugString(buf);
	}

	if (debug)
	{
		sprintf(buf,"Decimate: %d: %d %d %d %d %d\n",
		        frame, Dcurr[0], Dcurr[1], Dcurr[2], Dcurr[3], Dcurr[4]);
//		sprintf(buf,"Decimate: %d: %d %d %d %d %d - %d %d %d %d %d - %d %d %d %d %d\n",
//		        frame, Dprev[0], Dprev[1], Dprev[2], Dprev[3], Dprev[4],
//					   Dcurr[0], Dcurr[1], Dcurr[2], Dcurr[3], Dcurr[4],
//					   Dnext[0], Dnext[1], Dnext[2], Dnext[3], Dnext[4]);
		OutputDebugString(buf);
	}

	/* Find the longest strings of duplicates and decimate a frame from it. */
	highest = -1;
	for (f = 0; f < _param->cycle; f++)
	{
		if (Dcurr[f] == 1)
		{
			bsum = 0;
			fsum = 0;
		}
		else
		{
			bsum = 1;
			g = f;
			while (--g >= 0)
			{
				if (Dcurr[g] == 0)
				{
					bsum++;
				}
				else break;
			}
			if (g < 0)
			{
				g = _param->cycle;
				while (--g >= 0)
				{
					if (Dprev[g] == 0)
					{
						bsum++;
					}
					else break;
				}
			}
			fsum = 1;
			g = f;
			while (++g < _param->cycle)
			{
				if (Dcurr[g] == 0)
				{
					fsum++;
				}
				else break;
			}
			if (g >= _param->cycle)
			{
				g = -1;
				while (++g < _param->cycle)
				{
					if (Dnext[g] == 0)
					{
						fsum++;
					}
					else break;
				}
			}
		}
		if (bsum + fsum > highest)
		{
			highest = bsum + fsum;
			highest_index = f;
		}
//		sprintf(buf,"Decimate: bsum %d, fsum %d\n", bsum, fsum);
//		OutputDebugString(buf);
	}

	f = highest_index;
	if (Dcurr[f] == 1)
	{
		/* No duplicates were found! Act as if mode=0. */
		*chosen = last_result = frame + lowest_index;
	}
	else
	{
		/* Prevent this decimated frame from being considered again. */ 
		Dcurr[f] = 1;
		*chosen = last_result = frame + highest_index;
	}
	last_forced = false;
	if (debug)
	{
		sprintf(buf,"Decimate: dropping frame %d\n", last_result);
		OutputDebugString(buf);
	}

	
	found = false;
	
	if (found == true)
	{
		*chosen = last_result ;
		*forced = last_forced = true;
		if (debug)
		{
			sprintf(buf,"Decimate: overridden drop frame -- drop %d\n", last_result);
			OutputDebugString(buf);
		}
	}
}
#ifdef DECIMATE_MMX_BUILD_PLANE
//
//
//
//
void isse_blend_decimate_plane(uint8_t * dst, uint8_t* src,  uint8_t* src_next, 
			int w, int h)
{
uint32_t x;
	if (!h) return;  // Height == 0 - avoid silly crash.
	
	x=w>>3; // 8 pixels at a time
	for(;x>0;x--)
	{
	 __asm__(
                ADM_ALIGN16
	 	"movq  (%1), %%mm0 \n"
		"movq  (%2), %%mm2 \n"
		"pavgb %%mm0,%%mm1 \n"
		"movq  %%mm1,(%0) \n"

                   : : "r" (dst), "r" (src), "r" (src_next));
		
		dst+=8;
		src+=8;
		src_next+=8;
  	}
    	__asm__("emms");
  
}
int isse_scenechange_32(const uint8_t *c_plane, const uint8_t *tplane, int height, int width) 
{
  int wp=width>>5;
  int hp=height;
  int returnvalue=0xbadbad00;
    
    __asm__(
    ADM_ALIGN16
    "pxor %%mm6,%%mm6\n"
    "pxor %%mm7,%%mm7\n"
    ::);
    for(uint32_t y=0;y<hp;y++)
    {
	for(uint32_t x=0;x<wp;x++)
	{
		__asm__(
    		ADM_ALIGN16
    		"movq (%0),%%mm0 \n"
		"movq 8(%0),%%mm2 \n"
		"movq (%1),%%mm1 \n"
		"movq 8(%1),%%mm3 \n"
		"psadbw %%mm1,%%mm0\n"
		"psadbw %%mm3,%%mm2\n"
		"paddd %%mm0,%%mm6 \n"
		"paddd %%mm2,%%mm7 \n"
		
		"movq 16(%0),%%mm0 \n"
		"movq 24(%0),%%mm2 \n"
		"movq 16(%1),%%mm1 \n"
		"movq 24(%1),%%mm3 \n"
		"psadbw %%mm1,%%mm0\n"
		"psadbw %%mm3,%%mm2\n"
		"paddd %%mm0,%%mm6 \n"
		"paddd %%mm2,%%mm7 \n"
		
		
		: : "r" (c_plane) , "r" (tplane)
		);
		c_plane+=32;
		tplane+=32;
	}    
    
    	c_plane+=width-wp*32;
	tplane+=width-wp*32;
    }
    __asm__(
    ADM_ALIGN16
    "paddd %%mm6,%%mm7\n"
    "movd %%mm7,(%0)\n"
    "emms \n"
    : : "r" (&returnvalue)
    );
  
  return returnvalue;
}
int isse_scenechange_16(const uint8_t *c_plane, const uint8_t *tplane, int height, int width) 
{
  int wp=width>>4;
  int hp=height;
  int returnvalue=0xbadbad00;
    
    __asm__(
    ADM_ALIGN16
    "pxor %%mm6,%%mm6\n"
    "pxor %%mm7,%%mm7\n"
    ::);
    for(uint32_t y=0;y<hp;y++)
    {
	for(uint32_t x=0;x<wp;x++)
	{
		__asm__(
    		ADM_ALIGN16
    		"movq (%0),%%mm0 \n"
		"movq 8(%0),%%mm2 \n"
		"movq (%1),%%mm1 \n"
		"movq 8(%1),%%mm3 \n"
		"psadbw %%mm1,%%mm0\n"
		"psadbw %%mm3,%%mm2\n"
		"paddd %%mm0,%%mm6 \n"
		"paddd %%mm2,%%mm7 \n"				
		
		
		: : "r" (c_plane) , "r" (tplane)
		);
		c_plane+=16;
		tplane+=16;
	}    
    
    	c_plane+=width-wp*16;
	tplane+=width-wp*16;
    }
    __asm__(
    ADM_ALIGN16
    "paddd %%mm6,%%mm7\n"
    "movd %%mm7,(%0)\n"
    "emms \n"
    : : "r" (&returnvalue)
    );
  
  return returnvalue;
}
int isse_scenechange_8(const uint8_t *c_plane, const uint8_t *tplane, int height, int width) 
{
  int wp=width>>3;
  int hp=height;
  int returnvalue=0xbadbad00;
    
    __asm__(
    ADM_ALIGN16
    "pxor %%mm6,%%mm6\n"
    "pxor %%mm7,%%mm7\n"
    ::);
    for(uint32_t y=0;y<hp;y++)
    {
	for(uint32_t x=0;x<wp;x++)
	{
		__asm__(
    		ADM_ALIGN16
    		"movq (%0),%%mm0 \n"		
		"movq (%1),%%mm1 \n"		
		"psadbw %%mm1,%%mm0\n"		
		"paddd %%mm0,%%mm6 \n"
		
		: : "r" (c_plane) , "r" (tplane)
		);
		c_plane+=8;
		tplane+=8;
	}    
    
    	c_plane+=width-wp*8;
	tplane+=width-wp*8;
    }
    __asm__(
    ADM_ALIGN16
    "movd %%mm6,(%0)\n"
    "emms \n"
    : : "r" (&returnvalue)
    );
  
  return returnvalue;
}

#endif
