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
#include "decimate.h"




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

#define GETFRAME(g, fp) \
{ \
	int GETFRAMEf; \
	GETFRAMEf = (g); \
	if (GETFRAMEf < 0) GETFRAMEf = 0; \
	if (GETFRAMEf > num_frames_hi - 1) GETFRAMEf = num_frames_hi - 1; \
	(fp) = vidCache->getImage(GETFRAMEf); \
}
//________________________________


#define aprintf(...) {}
#include "DIA_factory.h"


// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   Decimate,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "decimate",            // internal name (must be uniq!)
                        "Decomb decimate",            // Display name
                        "Donald Graft decimate. Remove duplicate after telecide." // Description
                    );


/**
    \fn configure
*/
bool Decimate::configure(void)
{
	deciMate *_param=&configuration;
#define PX(x) &(configuration.x)
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
/**
    \fn getConfiguration
*/
const char   *Decimate::getConfiguration(void)
{
    const char strparam[255];
 	snprintf(strparam,254," Decomb Decimate cycle:%d",configuration.cycle);
    return strparam;
}

/**
    \fn Ctor
*/       
Decimate::Decimate(	ADM_coreVideoFilter *in,CONFcouple *couples)      : ADM_coreVideoFilter(in,couples)
{
		
		int count = 0;
		char buf[80];
		unsigned int *p;	
        deciMate *_param=&configuration;
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
		if(!couples || !ADM_paramLoad(couples,deciMate_param,&configuration))
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
#warning make it a function
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
			OutputDebugString( "Decimate %s by Donald Graft, Copyright 2003\n", 0); // VERSION
		}
	}
}
/**
    \fn getCoupledConf
*/ 
bool         Decimate::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, deciMate_param,&configuration);
}
/**
    \fn dtor
*/
Decimate::~Decimate(void)
{
		if (sum != NULL) ADM_dealloc(sum);
		if(vidCache) delete vidCache;

		vidCache=NULL;
		sum=NULL;
}
/**
    \fn DrawShow
*/
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

bool                Decimate::goToTime(uint64_t usSeek)
{
    return ADM_coreVideoFilter::goToTime(usSeek);
}
// EOF
