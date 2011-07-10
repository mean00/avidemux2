
/***************************************************************************
                          ADM_vidDecTelecide  -  description
                             -------------------
    
    email                : fixounet@free.fr

    Port of Donal Graft Telecide which is (c) Donald Graft
    http://www.neuron2.net
    http://puschpull.org/avisynth/decomb_reference_manual.html

 ***************************************************************************/
/*
	Telecide plugin for Avisynth -- recovers original progressive
	frames from telecined streams. The filter operates by matching
	fields and automatically adapts to phase/pattern changes.

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
#include "Telecide.h"
#include "DIA_factory.h"

#define aprintf(...) {}

/**
    \fn PutChosen
*/
void Telecide::PutChosen(int frame, unsigned int chosen)
        {
                int f;

                f = frame % CACHE_SIZE;
                if (frame < 0 /*|| frame > _info.nb_frames - 1*/ || cache[f].frame != frame)
                        return;
                cache[f].chosen = chosen;
        }
/**
    \fn CacheInsert
*/

void Telecide::CacheInsert(int frame, unsigned int p, unsigned int pblock,
                                                                unsigned int c, unsigned int cblock)
{
        int f;

        f = frame % CACHE_SIZE;
        if (frame < 0 )//|| frame > _info.nb_frames - 1)
                ADM_assert(0);
        cache[f].frame = frame;
        cache[f].metrics[P] = p;
        if (f) cache[f-1].metrics[N] = p;
        cache[f].metrics[C] = c;
        cache[f].metrics[PBLOCK] = pblock;
        cache[f].metrics[CBLOCK] = cblock;
        cache[f].chosen = 0xff;
}
/**
    \fn CacheQuery
*/
bool Telecide::CacheQuery(int frame, unsigned int *p, unsigned int *pblock,
                                                                unsigned int *c, unsigned int *cblock)
{
        int f;

        f = frame % CACHE_SIZE;
        if (frame < 0) // || frame > _info.nb_frames - 1)
        {
                printf("Frame %d is out! \n",frame); //,_info.nb_frames-1);
                ADM_assert(0);
        }
        if (cache[f].frame != frame)
        {
                return false;
        }
        *p = cache[f].metrics[P];
        *c = cache[f].metrics[C];
        *pblock = cache[f].metrics[PBLOCK];
        *cblock = cache[f].metrics[CBLOCK];
        return true;
}
/**
    \fn Show
*/
void Telecide::Show(ADMImage *dst, int frame)
{
	char use;
	teleCide *_param=&configuration;

	if (chosen == P) use = 'p';
	else if (chosen == C) use = 'c';
	else use = 'n';

	sprintf(buf, "Telecide %s", "ADM"); // VERSION
	DrawString(dst, 0, 0, buf);

	sprintf(buf, "Copyright 2003 Donald A. Graft");
	DrawString(dst, 0, 1, buf);

	sprintf(buf,"frame %d:", frame);
	DrawString(dst, 0, 3, buf);

	sprintf(buf, "matches: %d  %d  %d", p, c, np);
	DrawString(dst, 0, 4, buf);

	if (_param->post != POST_NONE)
	{
		sprintf(buf,"vmetrics: %d  %d  %d [chosen=%d]", pblock, cblock, npblock, vmetric);
		DrawString(dst, 0, 5, buf);
	}

	if (_param->guide != GUIDE_NONE)
	{
		sprintf(buf, "pattern mismatch=%0.2f%%", mismatch); 
		DrawString(dst, 0, 5 + (_param->post != POST_NONE), buf);
	}

	sprintf(buf,"[%s %c]%s %s",
		found == true ? "forcing" : "using", use,
		_param->post != POST_NONE ? (film == true ? " [progressive]" : " [interlaced]") : "",
		_param->guide != GUIDE_NONE ? status : "");
	DrawString(dst, 0, 5 + (_param->post != POST_NONE) + (_param->guide != GUIDE_NONE), buf);
}
/**
    \fn Debug
*/
void Telecide::Debug(int frame)
{
	char use;

	if (chosen == P) use = 'p';
	else if (chosen == C) use = 'c';
	else use = 'n';
	sprintf(buf,"Telecide: frame %d: matches: %d %d %d", frame, p, c, np);
	OutputDebugString(buf);
	if (configuration.post != POST_NONE)
	{
		sprintf(buf,"Telecide: frame %d: vmetrics: %d %d %d [chosen=%d]", frame, pblock, cblock, npblock, vmetric);
		OutputDebugString(buf);
	}
	sprintf(buf,"Telecide: frame %d: [%s %c]%s %s", frame, found == true ? "forcing" : "using", use,
		configuration.post  != POST_NONE ? (film == true ? " [progressive]" : " [interlaced]") : "",
		configuration.guide != GUIDE_NONE ? status : "");
	OutputDebugString(buf);
}
/**
    \fn configure
*/
bool Telecide::configure(void)
{
#define PX(x) &(configuration.x)
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )

    teleCide *_param=&configuration;
        
    ELEM_TYPE_FLOAT vthresh=(ELEM_TYPE_FLOAT)_param->vthresh;
    ELEM_TYPE_FLOAT bthresh=(ELEM_TYPE_FLOAT)_param->bthresh;
    ELEM_TYPE_FLOAT dthresh=(ELEM_TYPE_FLOAT)_param->dthresh;
    ELEM_TYPE_FLOAT gthresh=(ELEM_TYPE_FLOAT)_param->gthresh;

         diaMenuEntry tStrategy[]={
                             {GUIDE_NONE,   QT_TR_NOOP("No strategy"),NULL},
                             {GUIDE_32,     QT_TR_NOOP("3:2 pulldown"),NULL},
                             {GUIDE_22,     QT_TR_NOOP("PAL/SECAM"),NULL},
                             {GUIDE_32322,  QT_TR_NOOP("NTSC converted from PAL"),NULL}
                          };
                          
          diaMenuEntry tField[]={
                             {1,QT_TR_NOOP("Top"),NULL},
                             {0,QT_TR_NOOP("Bottom"),NULL}
          };
          
          diaMenuEntry tBackward[]={
                             {NO_BACK,QT_TR_NOOP("Never"),NULL},
                             {BACK_ON_COMBED,QT_TR_NOOP("If still combed"),NULL},
                             {ALWAYS_BACK,QT_TR_NOOP("Always"),NULL}
          };
          
          diaMenuEntry tPostproc[]={
                             {POST_NONE,      QT_TR_NOOP("None"),NULL},
                             {POST_METRICS,   QT_TR_NOOP("None but compute"),NULL},
                             {POST_FULL,      QT_TR_NOOP("Postproc on best match"),NULL},
                             {POST_FULL_MAP,  QT_TR_NOOP("Postproc and show zones (debug)"),NULL},
                             {POST_FULL_NOMATCH,QT_TR_NOOP("Process image (not fields)"),NULL},
                             {POST_FULL_NOMATCH_MAP,QT_TR_NOOP("Process image (not fields), debug"),NULL}
          };
                             
          
    diaElemMenu menuMode(PX(guide),   QT_TR_NOOP("_Strategy:"), SZT(tStrategy),tStrategy);
    diaElemMenu menuField(PX(order),  QT_TR_NOOP("_Field order:"), SZT(tField),tField);
    diaElemMenu menuPost(PX(post),    QT_TR_NOOP("_Postprocessing:"), SZT(tPostproc),tPostproc);
    diaElemMenu menuBackward(PX(back),QT_TR_NOOP("_Try backward:"), SZT(tBackward),tBackward);
    
    diaElemFloat direct(&dthresh,QT_TR_NOOP("_Direct threshold:"),0,200. );
    diaElemFloat backward(&bthresh,QT_TR_NOOP("_Backward threshold:"),0,200. );
    diaElemFloat noise(&gthresh,QT_TR_NOOP("_Noise threshold:"),0,200. );
    diaElemFloat post(&vthresh,QT_TR_NOOP("Postp_rocessing threshold:"),0,200. );
    
    diaElemToggle chroma(PX(chroma),QT_TR_NOOP("_Use chroma to decide"));
    diaElemToggle show(PX(show),QT_TR_NOOP("Sho_w info"));
    diaElemToggle debug(PX(debug),QT_TR_NOOP("Debu_g"));
    diaElemToggle blend(PX(blend),QT_TR_NOOP("Bl_end"));
    
    
    
    diaElem *elems[]={&menuMode,&menuField,&menuPost,&menuBackward,
        &direct,&backward,&noise,&post,&blend,
        &chroma,&show,&debug    };
    
  if(diaFactoryRun(QT_TR_NOOP("Decomb Telecide"),12,elems))
  {
    
      _param->vthresh=(float)vthresh;
      _param->bthresh=(float)bthresh;
      _param->dthresh=(float)dthresh;
      _param->gthresh=(float)gthresh;

    return 1; 
  }
  return 0;        
}
/**
    \fn getConfiguration
*/
const char *Telecide::getConfiguration( void )
{
static char buf[100];
 snprintf(buf,99," Decomb telecide");
 return buf;  
}

#define PROGRESSIVE  0x00000001
#define IN_PATTERN   0x00000002

/*
  uint8_t PutHintingData(unsigned char *video, unsigned int hint);
  uint8_t GetHintingData(unsigned char *video, unsigned int *hint);
*/
/**
    \fn WriteHints
*/  
void Telecide::WriteHints(unsigned char *dst, bool film, bool inpattern)
        {
                unsigned int hint;

                if (GetHintingData(dst, &hint) == true) hint = 0;
                if (film == true) hint |= PROGRESSIVE;
                else hint &= ~PROGRESSIVE;
                if (inpattern == true) hint |= IN_PATTERN;
                else hint &= ~IN_PATTERN;
                PutHintingData(dst, hint);
        }
/**
    \fn PredictHardYUY2
*/
bool Telecide::PredictHardYUY2(int frame, unsigned int *predicted, unsigned int *predicted_metric)
        {
                // Look for pattern in the actual delivered matches of the previous cycle of frames.
                // If a pattern is found, use that to predict the current match.
                if (configuration.guide == GUIDE_22)
                {
                        if (cache[(frame-cycle)%CACHE_SIZE  ].chosen == 0xff ||
                                cache[(frame-cycle+1)%CACHE_SIZE].chosen == 0xff)
                                return false;
                        switch ((cache[(frame-cycle)%CACHE_SIZE  ].chosen << 4) +
                                        (cache[(frame-cycle+1)%CACHE_SIZE].chosen))
                        {
                        case 0x11:
                                *predicted = C;
                                *predicted_metric = cache[frame%CACHE_SIZE].metrics[C];
                                break;
                        case 0x22:
                                *predicted = N;
                                *predicted_metric = cache[frame%CACHE_SIZE].metrics[N];
                                break;
                        default: return false;
                        }
                }
                else if (configuration.guide == GUIDE_32)
                {
                        if (cache[(frame-cycle)%CACHE_SIZE  ].chosen == 0xff ||
                                cache[(frame-cycle+1)%CACHE_SIZE].chosen == 0xff ||
                                cache[(frame-cycle+2)%CACHE_SIZE].chosen == 0xff ||
                                cache[(frame-cycle+3)%CACHE_SIZE].chosen == 0xff ||
                                cache[(frame-cycle+4)%CACHE_SIZE].chosen == 0xff)
                                return false;

                        switch ((cache[(frame-cycle)%CACHE_SIZE  ].chosen << 16) +
                                        (cache[(frame-cycle+1)%CACHE_SIZE].chosen << 12) +
                                        (cache[(frame-cycle+2)%CACHE_SIZE].chosen <<  8) +
                                        (cache[(frame-cycle+3)%CACHE_SIZE].chosen <<  4) +
                                        (cache[(frame-cycle+4)%CACHE_SIZE].chosen))
                        {
                        case 0x11122:
                        case 0x11221:
                        case 0x12211:
                        case 0x12221: 
                        case 0x21122: 
                        case 0x11222: 
                                *predicted = C;
                                *predicted_metric = cache[frame%CACHE_SIZE].metrics[C];
                                break;
                        case 0x22111:
                        case 0x21112:
                        case 0x22112: 
                        case 0x22211: 
                                *predicted = N;
                                *predicted_metric = cache[frame%CACHE_SIZE].metrics[N];
                                break;
                        default: return false;
                        }
                }
                else if (configuration.guide == GUIDE_32322)
                {
                        if (cache[(frame-cycle)%CACHE_SIZE  ].chosen == 0xff ||
                                cache[(frame-cycle+1)%CACHE_SIZE].chosen == 0xff ||
                                cache[(frame-cycle+2)%CACHE_SIZE].chosen == 0xff ||
                                cache[(frame-cycle+3)%CACHE_SIZE].chosen == 0xff ||
                                cache[(frame-cycle+4)%CACHE_SIZE].chosen == 0xff ||
                                cache[(frame-cycle+5)%CACHE_SIZE].chosen == 0xff)
                                return false;

                        switch ((cache[(frame-cycle)%CACHE_SIZE  ].chosen << 20) +
                                        (cache[(frame-cycle+1)%CACHE_SIZE].chosen << 16) +
                                        (cache[(frame-cycle+2)%CACHE_SIZE].chosen << 12) +
                                        (cache[(frame-cycle+3)%CACHE_SIZE].chosen <<  8) +
                                        (cache[(frame-cycle+4)%CACHE_SIZE].chosen <<  4) +
                                        (cache[(frame-cycle+5)%CACHE_SIZE].chosen))
                        {
                        case 0x111122:
                        case 0x111221:
                        case 0x112211:
                        case 0x122111:
                        case 0x111222: 
                        case 0x112221:
                        case 0x122211:
                        case 0x222111: 
                                *predicted = C;
                                *predicted_metric = cache[frame%CACHE_SIZE].metrics[C];
                                break;
                        case 0x221111:
                        case 0x211112:

                        case 0x221112: 
                        case 0x211122: 
                                *predicted = N;
                                *predicted_metric = cache[frame%CACHE_SIZE].metrics[N];
                                break;
                        default: return false;
                        }
                }
#ifdef DEBUG_PATTERN_GUIDANCE
                sprintf(buf, "%d: HARD: predicted = %d\n", frame, *predicted);
                OutputDebugString(buf);
#endif
                return true;
        }
/**
    \fn PredictSoftYUY2
*/
struct PREDICTION *Telecide::PredictSoftYUY2(int frame)
        {
                // Use heuristics to look forward for a match.
                int i, j, y, c, n, phase;
                unsigned int metric;

                pred[0].metric = 0xffffffff;
                if (frame < 0 /*|| frame > _info.nb_frames - 1 - cycle*/) return pred;

                // Look at the next cycle of frames.
                for (y = frame + 1; y <= frame + cycle; y++)
                {
                        // Look for a frame where the current and next match values are
                        // very close. Those are candidates to predict the phase, because
                        // that condition should occur only once per cycle. Store the candidate
                        // phases and predictions in a list sorted by goodness. The list will
                        // be used by the caller to try the phases in order.
                        c = cache[y%CACHE_SIZE].metrics[C]; 
                        n = cache[y%CACHE_SIZE].metrics[N];
                        if (c == 0) c = 1;
                        metric = (100 * abs (c - n)) / c;
                        phase = y % cycle;
                        if (metric < 5)
                        {
                                // Place the new candidate phase in sorted order in the list.
                                // Find the insertion point.
                                i = 0;
                                while (metric > pred[i].metric) i++;
                                // Find the end-of-list marker.
                                j = 0;
                                while (pred[j].metric != 0xffffffff) j++;
                                // Shift all items below the insertion point down by one to make
                                // room for the insertion.
                                j++;
                                for (; j > i; j--)
                                {
                                        pred[j].metric = pred[j-1].metric;
                                        pred[j].phase = pred[j-1].phase;
                                        pred[j].predicted = pred[j-1].predicted;
                                        pred[j].predicted_metric = pred[j-1].predicted_metric;
                                }
                                // Insert the new candidate data.
                                pred[j].metric = metric;
                                pred[j].phase = phase;
                                if (configuration.guide == GUIDE_32)
                                {
                                        switch ((frame % cycle) - phase)
                                        {
                                        case -4: pred[j].predicted = N; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[N]; break; 
                                        case -3: pred[j].predicted = N; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[N]; break; 
                                        case -2: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        case -1: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        case  0: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        case +1: pred[j].predicted = N; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[N]; break; 
                                        case +2: pred[j].predicted = N; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[N]; break; 
                                        case +3: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        case +4: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        }
                                }
                                else if (configuration.guide == GUIDE_32322)
                                {
                                        switch ((frame % cycle) - phase)
                                        {
                                        case -5: pred[j].predicted = N; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[N]; break; 
                                        case -4: pred[j].predicted = N; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[N]; break; 
                                        case -3: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        case -2: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        case -1: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        case  0: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        case +1: pred[j].predicted = N; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[N]; break; 
                                        case +2: pred[j].predicted = N; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[N]; break; 
                                        case +3: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        case +4: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        case +5: pred[j].predicted = C; pred[j].predicted_metric = cache[frame%CACHE_SIZE].metrics[C]; break; 
                                        }
                                }
                        }
#ifdef DEBUG_PATTERN_GUIDANCE
                        sprintf(buf,"%d: metric = %d phase = %d\n", frame, metric, phase);
                        OutputDebugString(buf);
#endif
                }
                return pred;
        }

/**
    \fn CalculateMetrics
*/
#define nt      configuration.nt
#define y0      configuration.y0
#define y1      configuration.y1
#define chroma  configuration.chroma
#define post    configuration.post


void Telecide::CalculateMetrics(int frame, 
                                unsigned char *fcrp,  unsigned char *fcrpU, 
                                unsigned char *fcrpV, unsigned char *fprp, 
                                unsigned char *fprpU, unsigned char *fprpV)
{
        int x, y, p, c, tmp1, tmp2, skip;
        bool vc;
        unsigned char *currbot0, *currbot2, *prevbot0, *prevbot2;
        unsigned char *prevtop0, *prevtop2, *prevtop4, *currtop0, *currtop2, *currtop4;
        unsigned char *a0, *a2, *b0, *b2, *b4;
        unsigned int diff, index;
#define T 4

        /* Clear the block sums. */
        for (y = 0; y < yblocks; y++)
        {
                for (x = 0; x < xblocks; x++)
                {
#ifdef WINDOWED_MATCH
                        matchp[y*xblocks+x] = 0;
                        matchc[y*xblocks+x] = 0;
#endif
                        sump[y*xblocks+x] = 0;
                        sumc[y*xblocks+x] = 0;
                }
        }

        /* Find the best field match. Subsample the frames for speed. */
        currbot0  = fcrp + pitch;
        currbot2  = fcrp + 3 * pitch;
        currtop0 = fcrp;
        currtop2 = fcrp + 2 * pitch;
        currtop4 = fcrp + 4 * pitch;
        prevbot0  = fprp + pitch;
        prevbot2  = fprp + 3 * pitch;
        prevtop0 = fprp;
        prevtop2 = fprp + 2 * pitch;
        prevtop4 = fprp + 4 * pitch;
        if (tff == true)
        {
                a0 = prevbot0;
                a2 = prevbot2;
                b0 = currtop0;
                b2 = currtop2;
                b4 = currtop4;
        }
        else
        {
                a0 = currbot0;
                a2 = currbot2;
                b0 = prevtop0;
                b2 = prevtop2;
                b4 = prevtop4;
        }
        p = c = 0;

        // Calculate the field match and film/video metrics.
        //if (vi.IsYV12()) skip = 1;
        if(1) skip=1;
        else skip = 1 + (chroma == false);
        for (y = 0, index = 0; y < h - 4; y+=4)
        {
                /* Exclusion band. Good for ignoring subtitles. */
                if (y0 == y1 || y < y0 || y > y1)
                {
                        for (x = 0; x < w;)
                        {
                                if (1) //vi.IsYV12())
                                        index = (y/BLKSIZE)*xblocks + x/BLKSIZE;
                                else
                                        index = (y/BLKSIZE)*xblocks + x/BLKSIZE_TIMES2;

                                // Test combination with current frame.
                                tmp1 = ((long)currbot0[x] + (long)currbot2[x]);
//                              diff = abs((long)currtop0[x] - (tmp1 >> 1));
                                diff = abs((((long)currtop0[x] + (long)currtop2[x] + (long)currtop4[x])) - (tmp1 >> 1) - tmp1);
                                if (diff > nt)
                                {
                                        c += diff;
#ifdef WINDOWED_MATCH
                                        matchc[index] += diff;
#endif
                                }

                                tmp1 = currbot0[x] + T;
                                tmp2 = currbot0[x] - T;
                                vc = (tmp1 < currtop0[x] && tmp1 < currtop2[x]) ||
                                         (tmp2 > currtop0[x] && tmp2 > currtop2[x]);
                                if (vc)
                                {
                                        sumc[index]++;
                                }

                                // Test combination with previous frame.
                                tmp1 = ((long)a0[x] + (long)a2[x]);
                                diff = abs((((long)b0[x] + (long)b2[x] + (long)b4[x])) - (tmp1 >> 1) - tmp1);
                                if (diff > nt)
                                {
                                        p += diff;
#ifdef WINDOWED_MATCH
                                        matchp[index] += diff;
#endif
                                }

                                tmp1 = a0[x] + T;
                                tmp2 = a0[x] - T;
                                vc = (tmp1 < b0[x] && tmp1 < b2[x]) ||
                                         (tmp2 > b0[x] && tmp2 > b2[x]);
                                if (vc)
                                {
                                        sump[index]++;
                                }

                                x += skip;
                                if (!(x&3)) x += 4;
                        }
                }
                currbot0 += pitchtimes4;
                currbot2 += pitchtimes4;
                currtop0 += pitchtimes4;
                currtop2 += pitchtimes4;
                currtop4 += pitchtimes4;
                a0               += pitchtimes4;
                a2               += pitchtimes4;
                b0               += pitchtimes4;
                b2               += pitchtimes4;
                b4               += pitchtimes4;
        }

       // if (vi.IsYV12() && chroma == true)
        if ( chroma == true)
        {
                int z;

                for (z = 0; z < 2; z++)
                {
                        // Do the same for the U plane.
                        if (z == 0)
                        {
                                currbot0  = fcrpU + pitchover2;
                                currbot2  = fcrpU + 3 * pitchover2;
                                currtop0 = fcrpU;
                                currtop2 = fcrpU + 2 * pitchover2;
                                currtop4 = fcrpU + 4 * pitchover2;
                                prevbot0  = fprpU + pitchover2;
                                prevbot2  = fprpU + 3 * pitchover2;
                                prevtop0 = fprpU;
                                prevtop2 = fprpU + 2 * pitchover2;
                                prevtop4 = fprpU + 4 * pitchover2;
                        }
                        else
                        {
                                currbot0  = fcrpV + pitchover2;
                                currbot2  = fcrpV + 3 * pitchover2;
                                currtop0 = fcrpV;
                                currtop2 = fcrpV + 2 * pitchover2;
                                currtop4 = fcrpV + 4 * pitchover2;
                                prevbot0  = fprpV + pitchover2;
                                prevbot2  = fprpV + 3 * pitchover2;
                                prevtop0 = fprpV;
                                prevtop2 = fprpV + 2 * pitchover2;
                                prevtop4 = fprpV + 4 * pitchover2;
                        }
                        if (tff == true)
                        {
                                a0 = prevbot0;
                                a2 = prevbot2;
                                b0 = currtop0;
                                b2 = currtop2;
                                b4 = currtop4;
                        }
                        else
                        {
                                a0 = currbot0;
                                a2 = currbot2;
                                b0 = prevtop0;
                                b2 = prevtop2;
                                b4 = prevtop4;
                        }

                        for (y = 0, index = 0; y < hover2 - 4; y+=4)
                        {
                                /* Exclusion band. Good for ignoring subtitles. */
                                if (y0 == y1 || y < y0/2 || y > y1/2)
                                {
                                        for (x = 0; x < wover2;)
                                        {
                                                if (1) //vi.IsYV12())
                                                        index = (y/BLKSIZE)*xblocks + x/BLKSIZE;
                                                else
                                                        index = (y/BLKSIZE)*xblocks + x/BLKSIZE_TIMES2;

                                                // Test combination with current frame.
                                                tmp1 = ((long)currbot0[x] + (long)currbot2[x]);
                                                diff = abs((((long)currtop0[x] + (long)currtop2[x] + (long)currtop4[x])) - (tmp1 >> 1) - tmp1);
                                                if (diff > nt)
                                                {
                                                        c += diff;
#ifdef WINDOWED_MATCH
                                                        matchc[index] += diff;
#endif
                                                }

                                                tmp1 = currbot0[x] + T;
                                                tmp2 = currbot0[x] - T;
                                                vc = (tmp1 < currtop0[x] && tmp1 < currtop2[x]) ||
                                                         (tmp2 > currtop0[x] && tmp2 > currtop2[x]);
                                                if (vc)
                                                {
                                                        sumc[index]++;
                                                }

                                                // Test combination with previous frame.
                                                tmp1 = ((long)a0[x] + (long)a2[x]);
                                                diff = abs((((long)b0[x] + (long)b2[x] + (long)b4[x])) - (tmp1 >> 1) - tmp1);
                                                if (diff > nt)
                                                {
                                                        p += diff;
#ifdef WINDOWED_MATCH
                                                        matchp[index] += diff;
#endif
                                                }

                                                tmp1 = a0[x] + T;
                                                tmp2 = a0[x] - T;
                                                vc = (tmp1 < b0[x] && tmp1 < b2[x]) ||
                                                         (tmp2 > b0[x] && tmp2 > b2[x]);
                                                if (vc)
                                                {
                                                        sump[index]++;
                                                }

                                                x ++;
                                                if (!(x&3)) x += 4;
                                        }
                                }
                                currbot0 += 4*pitchover2;
                                currbot2 += 4*pitchover2;
                                currtop0 += 4*pitchover2;
                                currtop2 += 4*pitchover2;
                                currtop4 += 4*pitchover2;
                                a0               += 4*pitchover2;
                                a2               += 4*pitchover2;
                                b0               += 4*pitchover2;
                                b2               += 4*pitchover2;
                                b4               += 4*pitchover2;
                        }
                }
        }

        // Now find the blocks that have the greatest differences.
#ifdef WINDOWED_MATCH
        highest_matchp = 0;
        for (y = 0; y < yblocks; y++)
        {
                for (x = 0; x < xblocks; x++)
                {
if (frame == 45 && matchp[y * xblocks + x] > 2500)
{
        sprintf(buf, "%d/%d = %d\n", x, y, matchp[y * xblocks + x]);
        OutputDebugString(buf);
}
                        if (matchp[y * xblocks + x] > highest_matchp)
                        {
                                highest_matchp = matchp[y * xblocks + x];
                        }
                }
        }
        highest_matchc = 0;
        for (y = 0; y < yblocks; y++)
        {
                for (x = 0; x < xblocks; x++)
                {
if (frame == 44 && matchc[y * xblocks + x] > 2500)
{
        sprintf(buf, "%d/%d = %d\n", x, y, matchc[y * xblocks + x]);
        OutputDebugString(buf);
}
                        if (matchc[y * xblocks + x] > highest_matchc)
                        {
                                highest_matchc = matchc[y * xblocks + x];
                        }
                }
        }
#endif
        if (post != POST_NONE)
        {
                highest_sump = 0;
                for (y = 0; y < yblocks; y++)
                {
                        for (x = 0; x < xblocks; x++)
                        {
                                if (sump[y * xblocks + x] > highest_sump)
                                {
                                        highest_sump = sump[y * xblocks + x];
                                }
                        }
                }
                highest_sumc = 0;
                for (y = 0; y < yblocks; y++)
                {
                        for (x = 0; x < xblocks; x++)
                        {
                                if (sumc[y * xblocks + x] > highest_sumc)
                                {
                                        highest_sumc = sumc[y * xblocks + x];
                                }
                        }
                }
        }
#ifdef WINDOWED_MATCH
        CacheInsert(frame, highest_matchp, highest_sump, highest_matchc, highest_sumc);
#else
        CacheInsert(frame, p, highest_sump, c, highest_sumc);
#endif
}
// EOF
