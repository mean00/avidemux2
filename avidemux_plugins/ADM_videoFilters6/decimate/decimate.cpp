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
#include "DIA_factory.h"

#include "dec_desc.cpp"

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
//                             {1, QT_TR_NOOP("Fast (partial luma and chroma)"),NULL},
                             {2, QT_TR_NOOP("Medium (full luma, no chroma)"),NULL},
//                             {3, QT_TR_NOOP("Slow (full luma and chroma)"),NULL}
                          };
  
    
    diaElemMenu menuMode(PX(mode),QT_TR_NOOP("_Mode:"), 4,tMode);
    diaElemMenu menuQuality(PX(quality),QT_TR_NOOP("_Quality:"), sizeof(tQuality)/sizeof(diaMenuEntry),tQuality);
    diaElemFloat menuThresh1(&t1,QT_TR_NOOP("_Threshold 1:"),0,100.);
    diaElemFloat menuThresh2(&t2,QT_TR_NOOP("T_hreshold 2:"),0,100.);
    diaElemUInteger cycle(PX(cycle),QT_TR_NOOP("C_ycle:"),2,40);
    diaElemToggle show(PX(show),QT_TR_NOOP("Sho_w"));
    diaElem *elems[]={&cycle,&menuMode,&menuQuality,&menuThresh1,&menuThresh2,&show};
    
  if(diaFactoryRun(QT_TR_NOOP("Decomb Decimate"),6,elems))
  {
    _param->threshold=(double )t1;
    _param->threshold2=(double )t2;
    updateInfo();
    reset();
    return 1; 
  }
  return 0;        
}
/**
    \fn getConfiguration
*/
const char   *Decimate::getConfiguration(void)
{
    static char strparam[255]={0};
 	snprintf(strparam,254," Decomb Decimate cycle:%d",configuration.cycle);
    return strparam;
}
/**
    \fn updateInfo
*/
void Decimate::updateInfo(void)
{
        if(configuration.cycle<2)
        {
            ADM_error("Telecide:bad configuration! cycle<2\n");
            return;
        }
        double inc=info.frameIncrement;
        inc*=configuration.cycle;
        inc/=configuration.cycle-1;
        info.frameIncrement=inc;

}
/**
    \fn reset
    \brief reset counters. Must be called each time a change is made (params/seek)
*/
void Decimate::reset(void)
{
		last_request = -1;
		firsttime = true;
        all_video_cycle = true;
        hints_invalid=false;
        vidCache->flush();
}
/**
    \fn Ctor
*/       
Decimate::Decimate(	ADM_coreVideoFilter *in,CONFcouple *couples)      : ADM_coreVideoFilter(in,couples)
{
		
		char buf[80];
		unsigned int *p;	
        deciMate *_param=&configuration;
		//		
		// Init here
		if(!couples || !ADM_paramLoad(couples,deciMate_param,&configuration))
  		{
			_param->cycle=5;
			_param->mode=3;
            _param->show=false;
            _param->debug=false;
			_param->quality=2;
			_param->threshold=0;
			_param->threshold2=3.0;
		}
		
		ADM_assert(_param->cycle);
		vidCache=new VideoCache(_param->cycle*2+1,in);
		
		if (_param->mode == 0 || _param->mode == 2 || _param->mode == 3)
		{
                    updateInfo();
		}
		sum = (unsigned int *) ADM_alloc(MAX_BLOCKS * MAX_BLOCKS * sizeof(unsigned int));
		ADM_assert(sum);		
	

		if (configuration.debug)
		{
			OutputDebugString( "Decimate %s by Donald Graft, Copyright 2003\n", 0); // VERSION
		}

        reset();
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
    \fn getNextFrame
*/

bool Decimate::getNextFrame(uint32_t *fn,ADMImage *data)
{
    switch(configuration.mode)
    {
        case 0: return get0(fn,data);break;
        case 1: return get1(fn,data);break;
        case 2: return get2(fn,data);break;
        case 3: return get3(fn,data);break;     
        default: ADM_assert(0);
    }
    return false;
}
/**
        \fn get0
        \brief A B C... if B is close enough to A discard it.
*/
bool Decimate::get0(uint32_t *fn,ADMImage *data)
{
    
    bool forced = false;
    ADMImage *src,*next;
    double metric;
    char buf[256];
    int useframe,dropframe;
    int start;
    deciMate *_param=&configuration;
    /* Normal decimation. Remove the frame most similar to its preceding frame. */
    /* Determine the correct frame to use and get it. */
    int cycle=configuration.cycle;
    int sourceFrame = (nextFrame*cycle)/(cycle-1);
    int cycleStartFrame = (sourceFrame / cycle) * cycle;
    int inframe=nextFrame;
    *fn=nextFrame;
    GETFRAME(sourceFrame, src);
    if(!src)
    {
        ADM_info("Decimate: End of video stream, cannot get frame %d\n",useframe);
        vidCache->unlockAll();
        return false;
    }
    nextFrame++;

    
    FindDuplicate(cycleStartFrame, &dropframe, &metric, &forced);
    if (sourceFrame >= dropframe) sourceFrame++;
    GETFRAME(sourceFrame, src);
    if(!src)
    {
        vidCache->unlockAll();
        return false;
    }
    data->duplicate(src);
    vidCache->unlockAll();

    if (configuration.show == true)
    {
        sprintf(buf, "Decimate %d", 0);			                DrawString(data, 0, 0, buf);
        sprintf(buf, "Copyright 2003 Donald Graft");			DrawString(data, 0, 1, buf);
        sprintf(buf,"%d: %3.2f", start, showmetrics[0]);		DrawString(data, 0, 3, buf);
        sprintf(buf,"%d: %3.2f", start + 1, showmetrics[1]);	DrawString(data, 0, 4, buf);
        sprintf(buf,"%d: %3.2f", start + 2, showmetrics[2]);	DrawString(data, 0, 5, buf);
        sprintf(buf,"%d: %3.2f", start + 3, showmetrics[3]);	DrawString(data, 0, 6, buf);
        sprintf(buf,"%d: %3.2f", start + 4, showmetrics[4]);	DrawString(data, 0, 7, buf);
        sprintf(buf,"in frm %d, use frm %d", inframe, useframe);DrawString(data, 0, 8, buf);
        sprintf(buf,"dropping frm %d%s", dropframe, last_forced == true ? ", forced!" : "");
        DrawString(data, 0, 9, buf);
    }
    if (configuration.debug)
    {	
        if (!(inframe % _param->cycle))
        {
            OutputDebugString("Decimate: %d: %3.2f\n", start, showmetrics[0]);
            OutputDebugString("Decimate: %d: %3.2f\n", start + 1, showmetrics[1]);
            OutputDebugString("Decimate: %d: %3.2f\n", start + 2, showmetrics[2]);
            OutputDebugString("Decimate: %d: %3.2f\n", start + 3, showmetrics[3]);
            OutputDebugString("Decimate: %d: %3.2f\n", start + 4, showmetrics[4]);
        }
        OutputDebugString("Decimate: in frm %d, use frm %d\n", inframe, useframe);
        OutputDebugString("Decimate: dropping frm %d%s\n", dropframe, last_forced == true ? ", forced!" : "");
    }		  
    return true;
}
/**
        \fn get1
        \brief mode=1, A B C D => A  BC D, i.e. (B,C) is replaced by BC, blend between B & C
*/

bool Decimate::get1(uint32_t *fn,ADMImage *data)
{
    bool    forced = false;
    ADMImage *src,*next;
    int useframe,dropframe;
    deciMate *_param=&configuration;
    int cycle=configuration.cycle;
    int sourceFrame = (nextFrame*cycle)/(cycle-1);
    int  cycleStartFrame = (sourceFrame / cycle) * cycle;
    unsigned int hint, film = 1;
    int inframe=nextFrame;
    double metric;
    char buf[256];
    
    GETFRAME(sourceFrame, src);
    if(!src)
    {
        ADM_info("Decimate: End of video stream, cannot get frame %d\n",useframe);
        vidCache->unlockAll();
        return false;
    }
    *fn=nextFrame;
    nextFrame++;


    if (GetHintingData(YPLANE(src), &hint) == false)
    {
        film = hint & PROGRESSIVE;
    }

    /* Find the most similar frame as above but replace it with a blend of
       the preceding and following frames. */
    FindDuplicate(cycleStartFrame, &dropframe, &metric, &forced);
    if (!film || sourceFrame != dropframe || (_param->threshold && metric > _param->threshold))
    {
        data->duplicate(src);
        vidCache->unlockAll();
        if (configuration.show == true)
        {

            sprintf(buf, "Decimate %d", 0);				DrawString(data, 0, 0, buf);
            sprintf(buf, "Copyright 2003 Donald Graft");				DrawString(data, 0, 1, buf);
            sprintf(buf,"%d: %3.2f", cycleStartFrame, showmetrics[0]);		    DrawString(data, 0, 3, buf);
            sprintf(buf,"%d: %3.2f", cycleStartFrame + 1, showmetrics[1]);		DrawString(data, 0, 4, buf);
            sprintf(buf,"%d: %3.2f", cycleStartFrame + 2, showmetrics[2]);		DrawString(data, 0, 5, buf);
            sprintf(buf,"%d: %3.2f", cycleStartFrame + 3, showmetrics[3]);		DrawString(data, 0, 6, buf);
            sprintf(buf,"%d: %3.2f", cycleStartFrame + 4, showmetrics[4]);		DrawString(data, 0, 7, buf);
            sprintf(buf,"infrm %d", inframe);
            DrawString(data, 0, 8, buf);
            if (last_forced == false)
                sprintf(buf,"chose %d, passing through", dropframe);
            else
                sprintf(buf,"chose %d, passing through, forced!", dropframe);
            DrawString(data, 0, 9, buf);
        }
        if (configuration.debug)
        {
            if (!(inframe % _param->cycle))
            {
                OutputDebugString("Decimate: %d: %3.2f\n", start, showmetrics[0]);
                OutputDebugString("Decimate: %d: %3.2f\n", start + 1, showmetrics[1]);
                OutputDebugString("Decimate: %d: %3.2f\n", start + 2, showmetrics[2]);
                OutputDebugString("Decimate: %d: %3.2f\n", start + 3, showmetrics[3]);
                OutputDebugString("Decimate: %d: %3.2f\n", start + 4, showmetrics[4]);
            }
            OutputDebugString("Decimate: in frm %d\n", inframe);
            
            if (last_forced == false)
            {
                OutputDebugString("Decimate: chose %d, passing through\n", dropframe);
            }
            else
            {
                OutputDebugString("Decimate: chose %d, passing through, forced!\n", dropframe);
            }
        }
        return true;
    }
    if (configuration.debug)
    {
        if (!(inframe % _param->cycle))
        {
            OutputDebugString( "Decimate: %d: %3.2f\n", start, showmetrics[0]);
            OutputDebugString( "Decimate: %d: %3.2f\n", start + 1, showmetrics[1]);
            OutputDebugString( "Decimate: %d: %3.2f\n", start + 2, showmetrics[2]);
            OutputDebugString( "Decimate: %d: %3.2f\n", start + 3, showmetrics[3]);
            OutputDebugString( "Decimate: %d: %3.2f\n", start + 4, showmetrics[4]);
        }
        OutputDebugString("Decimate: in frm %d\n", inframe);
        
        if (last_forced == false)
        {
            OutputDebugString("Decimate: chose %d, blending %d and %d\n", dropframe, inframe, nextfrm);
        }
        else
        {
            OutputDebugString("Decimate: chose %d, blending %d and %d, forced!\n", dropframe, inframe, nextfrm);
        }			
    }
    // Blend current frame with next frame
    GETFRAME(sourceFrame+1, next);
    if(!next)
        data->duplicate(src);
    else
        data->blend(src,next);
    vidCache->unlockAll();		
    if (configuration.show == true)
    {

        sprintf(buf, "Decimate %d", 0);			DrawString(data, 0, 0, buf);
        sprintf(buf, "Copyright 2003 Donald Graft");			DrawString(data, 0, 1, buf);
        sprintf(buf,"%d: %3.2f", cycleStartFrame, showmetrics[0]);		DrawString(data, 0, 3, buf);
        sprintf(buf,"%d: %3.2f", cycleStartFrame + 1, showmetrics[1]);	DrawString(data, 0, 4, buf);
        sprintf(buf,"%d: %3.2f", cycleStartFrame + 2, showmetrics[2]);	DrawString(data, 0, 5, buf);
        sprintf(buf,"%d: %3.2f", cycleStartFrame + 3, showmetrics[3]);	DrawString(data, 0, 6, buf);
        sprintf(buf,"%d: %3.2f", cycleStartFrame + 4, showmetrics[4]);	DrawString(data, 0, 7, buf);
        sprintf(buf,"infrm %d", inframe);
        DrawString(data, 0, 8, buf);
        if (last_forced == false)
            sprintf(buf,"chose %d, blending %d and %d",dropframe, sourceFrame, sourceFrame+1);
        else
            sprintf(buf,"chose %d, blending %d and %d, forced!", dropframe, sourceFrame, sourceFrame+1);
        DrawString(data, 0, 9, buf);
    }
    
    return true;
}
/**
        \fn get2
        \fn remove one frame from longest duplicate (anime)
*/

bool Decimate::get2(uint32_t *fn,ADMImage *data)
{
    bool forced = false;
    double metric;
    char buf[256];
    
    deciMate *_param=&configuration;
    int cycle=configuration.cycle;
    int sourceFrame = (nextFrame*cycle)/(cycle-1);
    int cycleStartFrame = (sourceFrame / cycle) * cycle;
    int useframe,dropframe;
    *fn=nextFrame;
    int inframe=nextFrame;
    ADMImage *src,*next;
    GETFRAME(sourceFrame, src);
    if(!src)
    {
        ADM_info("Decimate: End of video stream, cannot get frame %d\n",useframe);
        vidCache->unlockAll();
        return false;
    }
    nextFrame++;
    /* Delete the duplicate in the longest string of duplicates. */
    FindDuplicate2(cycleStartFrame, &dropframe, &forced);
    if (sourceFrame >= dropframe) 
        sourceFrame++;
    GETFRAME(sourceFrame, src);
    if(!src)
    {
        vidCache->unlockAll();
        return false;
    }
    data->duplicate(src);
    vidCache->unlockAll();
    if (configuration.show == true)
    {
        int start = (useframe / _param->cycle) * _param->cycle;


        sprintf(buf, "Decimate %d", 0);			DrawString(data, 0, 0, buf);
        sprintf(buf, "Copyright 2003 Donald Graft");			    DrawString(data, 0, 1, buf);
        sprintf(buf,"in frm %d, use frm %d", inframe, useframe);	DrawString(data, 0, 3, buf);
        sprintf(buf,"%d: %3.2f (%s)", cycleStartFrame, showmetrics[0], Dshow[0] ? "new" : "dup");	DrawString(data, 0, 4, buf);
        sprintf(buf,"%d: %3.2f (%s)", cycleStartFrame + 1, showmetrics[1],Dshow[1] ? "new" : "dup");DrawString(data, 0, 5, buf);
        sprintf(buf,"%d: %3.2f (%s)", cycleStartFrame + 2, showmetrics[2],Dshow[2] ? "new" : "dup");DrawString(data, 0, 6, buf);
        sprintf(buf,"%d: %3.2f (%s)", cycleStartFrame + 3, showmetrics[3],Dshow[3] ? "new" : "dup");DrawString(data, 0, 7, buf);
        sprintf(buf,"%d: %3.2f (%s)", cycleStartFrame + 4, showmetrics[4],Dshow[4] ? "new" : "dup");DrawString(data, 0, 8, buf);
        sprintf(buf,"Dropping frm %d%s", dropframe, last_forced == true ? " forced!" : "");
        DrawString(data, 0, 9, buf);
    }
    if (configuration.debug)
    {	
        sprintf(buf,"Decimate: inframe %d useframe %d\n", inframe, useframe);
        OutputDebugString(buf);
    }
    return true;
}
/**
        \fn get3
        \brief ivtc (after telecide)
*/

bool Decimate::get3(uint32_t *fn,ADMImage *data)
{
    bool forced = false;
    ADMImage *src,*next;
    int     dropframe;
    double  metric;
    char buf[256];
    
    deciMate *_param=&configuration;
    /* Decimate by removing a duplicate from film cycles and doing a
       blend rate conversion on the video cycles. */
    if (_param->cycle != 5)//	env->ThrowError("Decimate: mode=3 requires cycle=5");
    {
        ADM_error("Decimate: mode=3 requires cycle=5\n");
        return false;
    }
    int     sourceFrame = (nextFrame*5)/4;
    int     cycleStartFrame = (sourceFrame /5) * 5;
    
    *fn=nextFrame;
    GETFRAME(sourceFrame, src);
    if(!src)
    {
        ADM_info("Decimate: End of video stream, cannot get frame %d\n",sourceFrame);
        vidCache->unlockAll();
        return false;
    }
    int inframe=nextFrame;
    nextFrame++;
    FindDuplicate(cycleStartFrame, &dropframe, &metric, &forced);
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
        if (sourceFrame >= dropframe) sourceFrame++;
        GETFRAME(sourceFrame, src);
        if(!src)
        {
            vidCache->unlockAll();
            return false;
        }
        data->duplicate(src);    
        vidCache->unlockAll();		
        DrawShow(data, sourceFrame, forced, dropframe, metric, inframe);			
        return true; // return src;
    }
    else 
    {
        switch(inframe %4)
        {
            case 0:
                /* It's a video cycle. Output the first frame of the cycle. */
                GETFRAME(sourceFrame, src);
                data->duplicate(src);
                vidCache->unlockAll();		
                break;
            case 3:
                /* It's a video cycle. Output the last frame of the cycle. */
                GETFRAME(sourceFrame+1, src);
                if(!src)
                {
                    vidCache->unlockAll();		
                    return false;
                }
                data->duplicate(src);
                vidCache->unlockAll();		
                break;
            case 1: case 2:
                /* It's a video cycle. Make blends for the remaining frames. */
                if ((inframe % 4) == 1)  // MEANX dont understand the difference ?
                {
                    GETFRAME(sourceFrame, src);
                    GETFRAME(sourceFrame+1, next);
                    if(!next) next=src;
                }
                else
                {
                    GETFRAME(sourceFrame + 1, src);
                    GETFRAME(sourceFrame, next);
                    if(!src) src=next;
                }
                data->blend(src,next);
                vidCache->unlockAll();
                break;
            default:
                ADM_assert(0);break;
        }
        DrawShow(data, 0, forced, dropframe, metric, inframe);
        return true; // return src;			
    }
    GETFRAME(sourceFrame, src); // MEANX : not sure (jw detected a problem here)
    data->duplicate(src);
    vidCache->unlockAll();		
    DrawShow(data, 0, forced, dropframe, metric, inframe);
	return true;
}

/**
    \fn goToTime
*/
bool                Decimate::goToTime(uint64_t usSeek)
{
    reset();
    return ADM_coreVideoFilter::goToTime(usSeek);
}
// EOF
