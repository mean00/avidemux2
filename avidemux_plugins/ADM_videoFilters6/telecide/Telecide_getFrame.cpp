
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
#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif

#define GETFRAME(g, fp) { \
        int no=g; fp=NULL;\
        if (no < 0) no = 0; \
        aprintf("Cache query : %d - %d\n",no,__LINE__);\
        fp=vidCache->getImage(no); }

#define guide _param->guide
#define order _param->order
#define back  _param->back

#define back_saved  _param->back_saved
#define guide       _param->guide
#define gthresh     _param->gthresh
#define post        _param->post
#define chroma      _param->chroma
#define vthresh     _param->vthresh

#define vthresh_saved _param->vthresh_saved
#define hints      _param->hints
#define show       _param->show
#define debug      _param->debug

#define bthresh      _param->bthresh
#define dthresh      _param->dthresh
#define blend        _param->blend

#define nt      _param->nt
#define y0      _param->y0
#define y1      _param->y1

/**
    \fn copyField
*/
static bool copyField(ADMImage *target, ADMImage *source, bool top)
{
    for(int i=0;i<3;i++)
    {
        ADM_PLANE plane=(ADM_PLANE )i;
        uint8_t *dest=target->GetWritePtr(plane);
        uint8_t *src=source->GetReadPtr(plane);

        uint32_t sPitch=source->GetPitch(plane);
        uint32_t dPitch=target->GetPitch(plane);
        
        if(false==top)
        {
            dest=dest+dPitch;
            src=src+sPitch;
        }


        uint32_t h=target->GetHeight(plane);
        uint32_t w=target->GetWidth(plane);

        // copy one line out of two
        h>>=1;
        dPitch*=2;
        sPitch*=2;

        BitBlit(dest,dPitch,src,sPitch,w,h);

    }
    return true;
}

/**
    \fn getNextFrame
*/
bool Telecide::getNextFrame(uint32_t *frameNumber,ADMImage *output_image)
{
uint32_t pframe,nframe;

ADMImage *fc=NULL;
ADMImage *fp=NULL;
ADMImage *fn=NULL;
ADMImage *dst=NULL;
uint8_t *dstp;

ADMImage *final=NULL;
uint8_t *finalp;

unsigned int lowest;
unsigned int predicted;
unsigned int predicted_metric;
teleCide *_param=&configuration;


        *frameNumber=nextFrame;
        bool lastFrame=false;
        aprintf("telecide : frame %d\n",(int)nextFrame);
        // Get the current frame.
        uint32_t frame=nextFrame;
        if (frame < 0) frame = 0;
        
        GETFRAME(frame, fc);
        if(!fc)
        {
            ADM_info("Telecide:Cannot get frame\n");
            vidCache->unlockAll();
            return false;
        }
        nextFrame++;
        output_image->copyInfo(fc); // copy timing information...

        // Get the previous frame.
        pframe = frame == 0 ? 0 : frame - 1;
        GETFRAME(pframe, fp);

        // Get the next frame metrics if we might need them.
        nframe = frame + 1;
        GETFRAME(nframe, fn);
        if(!fn)
        {
            nframe=frame;
            GETFRAME(nframe, fn);
            ADM_assert(fn);
            lastFrame=true;
        }

        int pitch = fc->GetPitch(PLANAR_Y);
        
        
        int w = info.width;
        int h = info.height; 
        int hover2 = h >>1;
        int wover2 = w >>1;

        dst=output_image;
        int dpitch = dst->GetPitch(PLANAR_Y);

        // Ensure that the metrics for the frames
        // after the current frame are in the cache. They will be used for
        // pattern guidance.
        if (guide != GUIDE_NONE)
        {
                aprintf("Loop starting at %d +1, cycle=%d\n",frame,cycle);
                for (int y = frame + 1; y <= frame + cycle + 1; y++)
                {
                        if (lastFrame==true ) break;
                        if (CacheQuery(y, &p, &pblock, &c, &cblock) == false)
                        {
                                ADMImage *lc,*lp;

                                GETFRAME(y, lc);
                                GETFRAME(y == 0 ? 1 : y - 1, lp);
                                if(lc && lp)
                                    CalculateMetrics(y, lc,lp); //crp, crpU, crpV, prp, prpU, prpV);
                        }
                }
        }

        /* Check for manual overrides of the field matching. */
        
        found = false;
        film = true;
        
        inpattern = false;
        vthresh = vthresh_saved;
        back = back_saved;
        // Get the metrics for the current-previous (p), current-current (c), and current-next (n) match candidates.
        if (CacheQuery(frame, &p, &pblock, &c, &cblock) == false)
        {
                CalculateMetrics(frame, fc, fp); //fcrp, fcrpU, fcrpV, fprp, fprpU, fprpV);
                CacheQuery(frame, &p, &pblock, &c, &cblock);
        }
        if (CacheQuery(nframe, &np, &npblock, &nc, &ncblock) == false)
        {
                CalculateMetrics(nframe, fn,fc); //fnrp, fnrpU, fnrpV, fcrp, fcrpU, fcrpV);
                CacheQuery(nframe, &np, &npblock, &nc, &ncblock);
        }

        // Determine the best candidate match.
        if (found != true)
        {
                lowest = c;
                chosen = C;
                if (back == ALWAYS_BACK && p < lowest)
                {
                        lowest = p;
                        chosen = P;
                }
                if (np < lowest)
                {
                        lowest = np;
                        chosen = N;
                }
        }
        if ((frame == 0 && chosen == P) || (lastFrame==true && chosen == N))
        {
                chosen = C;
                lowest = c;
        }

        // See if we can apply pattern guidance.
        mismatch = 100.0;
        if (guide != GUIDE_NONE)
        {
                bool hard = false;
                if (frame >= cycle && PredictHardYUY2(frame, &predicted, &predicted_metric) == true)
                {
                        inpattern = true;
                        mismatch = 0.0;
                        hard = true;
                        if (chosen != predicted)
                        {
                                // The chosen frame doesn't match the prediction.
                                if (predicted_metric == 0) mismatch = 0.0;
                                else mismatch = (100.0*abs(predicted_metric - lowest))/predicted_metric;
                                if (mismatch < gthresh)
                                {
                                        // It's close enough, so use the predicted one.
                                        if (found != true)
                                        {
                                                chosen = predicted;
                                                override = true;
                                        }
                                }
                                else
                                {
                                        hard = false;
                                        inpattern = false;
                                }
                        }
                }

                if (hard == false && guide != GUIDE_22)
                {
                        int i;
                        
                        struct PREDICTION *pred = PredictSoftYUY2(frame);
                        
                        if (/*(frame <= _info.nb_frames - 1 - cycle) &&  */   (pred[0].metric != 0xffffffff))
                        {
                                // Apply pattern guidance.
                                // If the predicted match metric is within defined percentage of the
                                // best calculated one, then override the calculated match with the
                                // predicted match.
                                i = 0;
                                while (pred[i].metric != 0xffffffff)
                                {
                                        predicted = pred[i].predicted;
                                        predicted_metric = pred[i].predicted_metric;
#ifdef DEBUG_PATTERN_GUIDANCE
                                        sprintf(buf, "%d: predicted = %d\n", frame, predicted);
                                        OutputDebugString(buf);
#endif
                                        if (chosen != predicted)
                                        {
                                                // The chosen frame doesn't match the prediction.
                                                if (predicted_metric == 0) mismatch = 0.0;
                                                else mismatch = (100.0*abs(predicted_metric - lowest))/predicted_metric;
                                                if ((int) mismatch <= gthresh)
                                                {
                                                        // It's close enough, so use the predicted one.
                                                        if (found != true)
                                                        {
                                                                chosen = predicted;
                                                                override = true;
                                                        }
                                                        inpattern = true;
                                                        break;
                                                }
                                                else
                                                {
                                                        // Looks like we're not in a predictable pattern.
                                                        inpattern = false;
                                                }
                                        }
                                        else
                                        {
                                                inpattern = true;
                                                mismatch = 0.0;
                                                break;
                                        }
                                        i++;
                                }
                        }
                }
        }

        // Check for overrides of vthresh.
        // Check the match for progressive versus interlaced.
        if (post != POST_NONE)
        {
                if (chosen == P) vmetric = pblock;
                else if (chosen == C) vmetric = cblock;
                else if (chosen == N) vmetric = npblock;

                if (found == false && back == BACK_ON_COMBED && vmetric > bthresh && p < lowest)
                {
                        // Backward match.
                        vmetric = pblock;
                        chosen = P;
                        inpattern = false;
                        mismatch = 100;
                }
                if (vmetric > vthresh)
                {
                        // After field matching and pattern guidance the frame is still combed.
                        film = false;
                        if (found == false && (post == POST_FULL_NOMATCH || post == POST_FULL_NOMATCH_MAP))
                        {
                                chosen = C;
                                vmetric = cblock;
                                inpattern = false;
                                mismatch = 100;
                        }
                }
        }
        vthresh = vthresh_saved;

        // Setup strings for debug info.
        if (inpattern == true && override == false) strcpy(status, "[in-pattern]");
        else if (inpattern == true && override == true) strcpy(status, "[in-pattern*]");
        else strcpy(status, "[out-of-pattern]");

        // Assemble and output the reconstructed frame according to the final match.
        dstp = dst->GetWritePtr(PLANAR_Y);
        if (chosen == N)
        {
                // The best match was with the next frame.
                if (tff == true)
                {
                        copyField(dst, fn,true);
                        copyField(dst, fc,false);
                }
                else
                {
                        copyField(dst, fc,true);
                        copyField(dst, fn,false);
                }
        }
        else if (chosen == C)
        {
                // The best match was with the current frame.
                copyField(dst, fc,true);
                copyField(dst, fc,false);

        }
        else if (tff == false)
        {
                // The best match was with the previous frame.
                copyField(dst, fp,true);
                copyField(dst, fc,false);
        }
        else
        {
                // The best match was with the previous frame.
                copyField(dst, fc,true);
                copyField(dst, fp,false);

        }
        if (guide != GUIDE_NONE) PutChosen(frame, chosen);

        /* Check for manual overrides of the deinterlacing. */
        // Do postprocessing if enabled and required for this frame.
         if ((post == POST_FULL || post == POST_FULL_MAP || post == POST_FULL_NOMATCH || post == POST_FULL_NOMATCH_MAP)
                         && (film == false ))
        {
                unsigned char *dstpp, *dstpn;
                int v1, v2, z;
                #warning blend in place is wrong!
                final=dst;
                // MeanX:We should copy here as we blend from source and destination
                // for the moment we do it in place, it is wrong.
                if (blend == true)
                {
                        blendPlane(final,dst,PLANAR_Y);
                        blendPlane(final,dst,PLANAR_U);
                        blendPlane(final,dst,PLANAR_V);
                       
                        if (show == true)  Show(final, frame);
                        if (debug == true) Debug(frame);
                        if (hints == true) WriteHints(final->GetWritePtr(PLANAR_Y), film, inpattern);
                       // return final;
                        vidCache->unlockAll();
                        return 1;
                }
                interpolatePlane(final,PLANAR_Y);
                interpolatePlane(final,PLANAR_U);
                interpolatePlane(final,PLANAR_V);
        }
        if (show == true) Show(dst, frame);
        if (debug == true) Debug(frame);
        if (hints == true) WriteHints(dst->GetWritePtr(PLANAR_Y), film, inpattern);
        vidCache->unlockAll();
        //return dst;
        return 1;
}
// EOF
