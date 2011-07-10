
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
#if 1
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif

/**
    \fn getNextFrame
*/
bool Telecide::getNextFrame(uint32_t *frameNumber,ADMImage *output_image)
{
ADMImage *fc;
uint8_t *fcrp;
uint8_t *fcrpU,*fcrpV;

uint32_t pframe,nframe;

ADMImage *fp;
uint8_t *fprp;
uint8_t *fprpU,*fprpV;

ADMImage *fn;
uint8_t *fnrp;
uint8_t *fnrpU,*fnrpV;

ADMImage *lc;
uint8_t *crp;
uint8_t *crpU,*crpV;

ADMImage *lp;
uint8_t *prp;
uint8_t *prpU,*prpV;

ADMImage *dst;
uint8_t *dstp;
uint8_t *dstpU,*dstpV;

ADMImage *final;
uint8_t *finalp;
uint8_t *finalpU,*finalpV;

teleCide *_param=&configuration;
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

        
        bool lastFrame=false;
        aprintf("telecide : frame %d\n",(int)nextFrame);
        // Get the current frame.
        uint32_t frame=nextFrame;
        if (frame < 0) frame = 0;
        GETFRAME(frame, fc);
        if(!fc)
        {
            ADM_info("Telecide:Cannot get frame\n");
            return false;
        }
        nextFrame++;
        output_image->copyInfo(fc);
        fcrp = (unsigned char *) fc->GetReadPtr(PLANAR_Y);
        //if (vi.IsYV12())
        {
                fcrpU = (unsigned char *) fc->GetReadPtr(PLANAR_U);
                fcrpV = (unsigned char *) fc->GetReadPtr(PLANAR_V);
        }

        // Get the previous frame.
        pframe = frame == 0 ? 0 : frame - 1;
        GETFRAME(pframe, fp);
        fprp = (unsigned char *) fp->GetReadPtr(PLANAR_Y);
        //if (vi.IsYV12())
        {
                fprpU = (unsigned char *) fp->GetReadPtr(PLANAR_U);
                fprpV = (unsigned char *) fp->GetReadPtr(PLANAR_V);
        }

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
        fnrp = (unsigned char *) fn->GetReadPtr(PLANAR_Y);
        //if (vi.IsYV12())
        {
                fnrpU = (unsigned char *) fn->GetReadPtr(PLANAR_U);
                fnrpV = (unsigned char *) fn->GetReadPtr(PLANAR_V);
        }

        pitch = fc->GetPitch(PLANAR_Y);
        pitchover2 = pitch >> 1;
        pitchtimes4 = pitch << 2;
        w = fc->GetRowSize(PLANAR_Y);
        h = fc->GetHeight(PLANAR_Y);
/*
        if (vi.IsYUY2() && ((w/2) & 1))
                env->ThrowError("Telecide: width must be a multiple of 2; use Crop");
        if (vi.IsYV12() && (w & 1))
                env->ThrowError("Telecide: width must be a multiple of 2; use Crop");
        if (h & 1)
                env->ThrowError("Telecide: height must be a multiple of 2; use Crop");
*/
        wover2 = w/2;
        hover2 = h/2;
        hplus1over2 = (h+1)/2;
        hminus2= h - 2;
        //dst = env->NewVideoFrame(vi);
        dst=output_image;
        dpitch = dst->GetPitch(PLANAR_Y);

        // Ensure that the metrics for the frames
        // after the current frame are in the cache. They will be used for
        // pattern guidance.
        if (guide != GUIDE_NONE)
        {
                for (y = frame + 1; y <= frame + cycle + 1; y++)
                {
                        if (lastFrame==true ) break;
                        if (CacheQuery(y, &p, &pblock, &c, &cblock) == false)
                        {
                                GETFRAME(y, lc);
                                crp = (unsigned char *) lc->GetReadPtr(PLANAR_Y);
                                //if (vi.IsYV12())
                                {
                                        crpU = (unsigned char *) lc->GetReadPtr(PLANAR_U);
                                        crpV = (unsigned char *) lc->GetReadPtr(PLANAR_V);
                                }
                                GETFRAME(y == 0 ? 1 : y - 1, lp);
                                prp = (unsigned char *) lp->GetReadPtr(PLANAR_Y);
                                //if (vi.IsYV12())
                                {
                                        prpU = (unsigned char *) lp->GetReadPtr(PLANAR_U);
                                        prpV = (unsigned char *) lp->GetReadPtr(PLANAR_V);
                                }
                                CalculateMetrics(y, crp, crpU, crpV, prp, prpU, prpV);
                        }
                }
        }

        /* Check for manual overrides of the field matching. */
        
        found = false;
        film = true;
        
        inpattern = false;
        vthresh = vthresh_saved;
        back = back_saved;
#if 0
        overrides_p = overrides;
        override = false;
        if (overrides_p != NULL)
        {
                while (*overrides_p < 0xffffffff)
                {
                        // If the frame is in range...
                        if (((unsigned int) frame >= *overrides_p) && ((unsigned int) frame <= *(overrides_p+1)))
                        {
                                // and it's a single specifier. 
                                if (*(overrides_p+3) == 'p' || *(overrides_p+3) == 'c' || *(overrides_p+3) == 'n')
                                {
                                        // Get the match specifier and stop parsing.
                                        switch(*(overrides_p+3))
                                        {
                                        case 'p': chosen = P; lowest = cache[frame%CACHE_SIZE].metrics[P]; found = true; break;
                                        case 'c': chosen = C; lowest = cache[frame%CACHE_SIZE].metrics[C]; found = true; break;
                                        case 'n': chosen = N; lowest = cache[(frame+1)%CACHE_SIZE].metrics[P]; found = true; break;
                                        }
                                }
                                else if (*(overrides_p+3) == 'b')
                                {
                                        back = *(overrides_p+2);
                                }
                                else if (*(overrides_p+3) == 'm')
                                {
                                        // It's a multiple match specifier.
                                        found = true;
                                        // Get the pointer to the specifier string.
                                        unsigned int *x = (unsigned int *) *(overrides_p+2);
                                        // Get the index into the specification string.
                                        // Remember, the count is first followed by the specifiers.
                                        int ndx = ((frame - *overrides_p) % *x);
                                        // Point to the specifier string.
                                        x++;
                                        // Load the specifier for this frame and stop parsing.
                                        switch(x[ndx])
                                        {
                                        case 'p': chosen = P; lowest = cache[frame%CACHE_SIZE].metrics[P]; break;
                                        case 'c': chosen = C; lowest = cache[frame%CACHE_SIZE].metrics[C]; break;
                                        case 'n': chosen = N; lowest = cache[(frame+1)%CACHE_SIZE].metrics[P]; break;
                                        }
                                }
                        }
                        // Next override line.
                        overrides_p += 4;
                }
        }
#endif
        // Get the metrics for the current-previous (p), current-current (c), and current-next (n) match candidates.
        if (CacheQuery(frame, &p, &pblock, &c, &cblock) == false)
        {
                CalculateMetrics(frame, fcrp, fcrpU, fcrpV, fprp, fprpU, fprpV);
                CacheQuery(frame, &p, &pblock, &c, &cblock);
        }
        if (CacheQuery(nframe, &np, &npblock, &nc, &ncblock) == false)
        {
                CalculateMetrics(nframe, fnrp, fnrpU, fnrpV, fcrp, fcrpU, fcrpV);
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
                hard = false;
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
#if 0
        overrides_p = overrides;
        if (overrides_p != NULL)
        {
                while (*overrides_p < 0xffffffff)
                {
                        // If the frame is in range...
                        if (((unsigned int) frame >= *overrides_p) && ((unsigned int) frame <= *(overrides_p+1)))
                        {
                                if (*(overrides_p+3) == 'v')
                                {
                                        vthresh = *(overrides_p+2);
                                }
                        }
                        // Next override line.
                        overrides_p += 4;
                }
        }
#endif
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
//    if (vi.IsYV12())
        {
                dstpU = dst->GetWritePtr(PLANAR_U);
                dstpV = dst->GetWritePtr(PLANAR_V);
        }
        if (chosen == N)
        {
                // The best match was with the next frame.
                if (tff == true)
                {
                        BitBlt(dstp, 2 * dpitch, fnrp, 2 * pitch, w, hover2);
                        BitBlt(dstp + dpitch, 2 * dpitch, fcrp + pitch, 2 * pitch, w, hover2);
                        //if (vi.IsYV12())
                        {
                                BitBlt(dstpU, dpitch, fnrpU, pitch, w/2, h/4);
                                BitBlt(dstpV, dpitch, fnrpV, pitch, w/2, h/4);
                                BitBlt(dstpU + dpitch/2, dpitch, fcrpU + pitch/2, pitch, w/2, h/4);
                                BitBlt(dstpV + dpitch/2, dpitch, fcrpV + pitch/2, pitch, w/2, h/4);
                        }
                }
                else
                {
                        BitBlt(dstp, 2 * dpitch, fcrp, 2 * pitch, w, hplus1over2);
                        BitBlt(dstp + dpitch, 2 * dpitch, fnrp + pitch, 2 * pitch, w, hover2);
                        //if (vi.IsYV12())
                        {
                                BitBlt(dstpU, dpitch, fcrpU, pitch, w/2, h/4);
                                BitBlt(dstpV, dpitch, fcrpV, pitch, w/2, h/4);
                                BitBlt(dstpU + dpitch/2, dpitch, fnrpU + pitch/2, pitch, w/2, h/4);
                                BitBlt(dstpV + dpitch/2, dpitch, fnrpV + pitch/2, pitch, w/2, h/4);
                        }
                }
        }
        else if (chosen == C)
        {
                // The best match was with the current frame.
                BitBlt(dstp, 2 * dpitch, fcrp, 2 * pitch, w, hplus1over2);
                BitBlt(dstp + dpitch, 2 * dpitch, fcrp + pitch, 2 * pitch, w, hover2);
                //if (vi.IsYV12())
                {
                        BitBlt(dstpU, dpitch, fcrpU, pitch, w/2, h/4);
                        BitBlt(dstpV, dpitch, fcrpV, pitch, w/2, h/4);
                        BitBlt(dstpU + dpitch/2, dpitch, fcrpU + pitch/2, pitch, w/2, h/4);
                        BitBlt(dstpV + dpitch/2, dpitch, fcrpV + pitch/2, pitch, w/2, h/4);
                }
        }
        else if (tff == false)
        {
                // The best match was with the previous frame.
                BitBlt(dstp, 2 * dpitch, fprp, 2 * pitch, w, hplus1over2);
                BitBlt(dstp + dpitch, 2 * dpitch, fcrp + pitch, 2 * pitch, w, hover2);
               // if (vi.IsYV12())
                {
                        BitBlt(dstpU, dpitch, fprpU, pitch, w/2, h/4);
                        BitBlt(dstpV, dpitch, fprpV, pitch, w/2, h/4);
                        BitBlt(dstpU + dpitch/2, dpitch, fcrpU + pitch/2, pitch, w/2, h/4);
                        BitBlt(dstpV + dpitch/2, dpitch, fcrpV + pitch/2, pitch, w/2, h/4);
                }
        }
        else
        {
                // The best match was with the previous frame.
                BitBlt(dstp, 2 * dpitch, fcrp, 2 * pitch, w, hplus1over2);
                BitBlt(dstp + dpitch, 2 * dpitch, fprp + pitch, 2 * pitch, w, hover2);
               // if (vi.IsYV12())
                {
                        BitBlt(dstpU, dpitch, fcrpU, pitch, w/2, h/4);
                        BitBlt(dstpV, dpitch, fcrpV, pitch, w/2, h/4);
                        BitBlt(dstpU + dpitch/2, dpitch, fprpU + pitch/2, pitch, w/2, h/4);
                        BitBlt(dstpV + dpitch/2, dpitch, fprpV + pitch/2, pitch, w/2, h/4);
                }
        }
        if (guide != GUIDE_NONE) PutChosen(frame, chosen);

        /* Check for manual overrides of the deinterlacing. */
#if 0
        overrides_p = overrides;
        force = 0;
        if (overrides_p != NULL)
        {
                while (*overrides_p < 0xffffffff)
                {
                        // Is the frame in range...
                        if (((unsigned int) frame >= *overrides_p) && ((unsigned int) frame <= *(overrides_p+1)) &&
                                // and is it a single specifier...
                                (*(overrides_p+2) == 0) &&
                                // and is it a deinterlacing specifier?
                                (*(overrides_p+3) == '+' || *(overrides_p+3) == '-'))
                        {
                                // Yes, load the specifier and stop parsing.
                                overrides_p += 3;
                                force = *overrides_p;
                                break;
                        }
                        // Next specification record.
                        overrides_p += 4;
                }
        }
#endif
        // Do postprocessing if enabled and required for this frame.
        if (post == POST_NONE || post == POST_METRICS)
        {
                if (force == '+') film = false;
                else if (force == '-') film = true;
        }
        else if ((force == '+') ||
                ((post == POST_FULL || post == POST_FULL_MAP || post == POST_FULL_NOMATCH || post == POST_FULL_NOMATCH_MAP)
                         && (film == false && force != '-')))
        {
                unsigned char *dstpp, *dstpn;
                int v1, v2, z;

                if (blend == true)
                {
                        // Blend mode.
                        final = output_image; //env->NewVideoFrame(vi);
                        // Do first and last lines.
                        finalp = final->GetWritePtr(PLANAR_Y);
                        dstp = dst->GetWritePtr(PLANAR_Y);
                        dstpn = dstp + dpitch;
                        for (x = 0; x < w; x++)
                        {
                                finalp[x] = (((int)dstp[x] + (int)dstpn[x]) >> 1);
                        }
                        finalp = final->GetWritePtr(PLANAR_Y) + (h-1)*dpitch;
                        dstp = dst->GetWritePtr(PLANAR_Y) + (h-1)*dpitch;
                        dstpp = dstp - dpitch;
                        for (x = 0; x < w; x++)
                        {
                                finalp[x] = (((int)dstp[x] + (int)dstpp[x]) >> 1);
                        }
                        // Now do the rest.
                        dstp = dst->GetWritePtr(PLANAR_Y) + dpitch;
                        dstpp = dstp - dpitch;
                        dstpn = dstp + dpitch;
                        finalp = final->GetWritePtr(PLANAR_Y) + dpitch;
                        for (y = 1; y < h - 1; y++)
                        {
                                for (x = 0; x < w; x++)
                                {
                                        v1 = (int)(dstp[x] - dthresh);
                                        if (v1 < 0) v1 = 0; 
                                        v2 = (int) (dstp[x] + dthresh);
                                        if (v2 > 235) v2 = 235; 
                                        if ((v1 > dstpp[x] && v1 > dstpn[x]) || (v2 < dstpp[x] && v2 < dstpn[x]))
                                        {
                                                if (post == POST_FULL_MAP || post == POST_FULL_NOMATCH_MAP)
                                                {
                                                        if (0) //(vi.IsYUY2())
                                                        {
                                                                if (x & 1) finalp[x] = 128;
                                                                else finalp[x] = 235;
                                                        }
                                                        else
                                                        {
                                                                finalp[x] = 235;
                                                        }
                                                }
                                                else
                                                        finalp[x] = ((int)dstpp[x] + (int)dstpn[x] + (int)dstp[x] + (int)dstp[x]) >> 2;
                                        }
                                        else finalp[x] = dstp[x];
                                }
                                finalp += dpitch;
                                dstp += dpitch;
                                dstpp += dpitch;
                                dstpn += dpitch;
                        }

                      //  if (vi.IsYV12())
                        {
                                // Chroma planes.
                                for (z = 0; z < 2; z++)
                                {
                                        if (z == 0)
                                        {
                                                // Do first and last lines.
                                                finalp = final->GetWritePtr(PLANAR_U);
                                                dstp = dst->GetWritePtr(PLANAR_U);
                                                dstpn = dstp + dpitch/2;
                                                for (x = 0; x < wover2; x++)
                                                {
                                                        finalp[x] = (((int)dstp[x] + (int)dstpn[x]) >> 1);
                                                }
                                                finalp = final->GetWritePtr(PLANAR_U) + (hover2-1)*dpitch/2;
                                                dstp = dst->GetWritePtr(PLANAR_U) + (hover2-1)*dpitch/2;
                                                dstpp = dstp - dpitch/2;
                                                for (x = 0; x < wover2; x++)
                                                {
                                                        finalp[x] = (((int)dstp[x] + (int)dstpp[x]) >> 1);
                                                }
                                                // Now do the rest.
                                                finalp = final->GetWritePtr(PLANAR_U) + dpitch/2;
                                                dstp = dst->GetWritePtr(PLANAR_U) + dpitch/2;
                                        }
                                        else
                                        {
                                                // Do first and last lines.
                                                finalp = final->GetWritePtr(PLANAR_V);
                                                dstp = dst->GetWritePtr(PLANAR_V);
                                                dstpn = dstp + dpitch/2;
                                                for (x = 0; x < wover2; x++)
                                                {
                                                        finalp[x] = (((int)dstp[x] + (int)dstpn[x]) >> 1);
                                                }
                                                finalp = final->GetWritePtr(PLANAR_V) + (hover2-1)*dpitch/2;
                                                dstp = dst->GetWritePtr(PLANAR_V) + (hover2-1)*dpitch/2;
                                                dstpp = dstp - dpitch/2;
                                                for (x = 0; x < wover2; x++)
                                                {
                                                        finalp[x] = (((int)dstp[x] + (int)dstpp[x]) >> 1);
                                                }
                                                // Now do the rest.
                                                finalp = final->GetWritePtr(PLANAR_V) + dpitch/2;
                                                dstp = dst->GetWritePtr(PLANAR_V) + dpitch/2;
                                        }
                                        dstpp = dstp - dpitch/2;
                                        dstpn = dstp + dpitch/2;
                                        for (y = 1; y < hover2 - 1; y++)
                                        {
                                                for (x = 0; x < wover2; x++)
                                                {
                                                        v1 = (int)( dstp[x] - dthresh);
                                                        if (v1 < 0) v1 = 0; 
                                                        v2 = (int)( dstp[x] + dthresh);
                                                        if (v2 > 235) v2 = 235; 
                                                        if ((v1 > dstpp[x] && v1 > dstpn[x]) || (v2 < dstpp[x] && v2 < dstpn[x]))
                                                        {
                                                                if (post == POST_FULL_MAP || post == POST_FULL_NOMATCH_MAP)
                                                                {
                                                                        finalp[x] = 128;
                                                                }
                                                                else
                                                                        finalp[x] = ((int)dstpp[x] + (int)dstpn[x] + (int)dstp[x] + (int)dstp[x]) >> 2;
                                                        }
                                                        else finalp[x] = dstp[x];
                                                }
                                                finalp += dpitch/2;
                                                dstp += dpitch/2;
                                                dstpp += dpitch/2;
                                                dstpn += dpitch/2;
                                        }
                                }
                        }
                        if (show == true) Show(final, frame);
                        if (debug == true) Debug(frame);
                        if (hints == true) WriteHints(final->GetWritePtr(PLANAR_Y), film, inpattern);
                       // return final;
                        vidCache->unlockAll();
                        return 1;
                }

                // Interpolate mode.
                // Luma plane.
                dstp = dst->GetWritePtr(PLANAR_Y) + dpitch;
                dstpp = dstp - dpitch;
                dstpn = dstp + dpitch;
                for (y = 1; y < h - 1; y+=2)
                {
                        for (x = 0; x < w; x++)
                        {
                                v1 = (int) (dstp[x] - dthresh);
                                if (v1 < 0) v1 = 0; 
                                v2 = (int) dstp[x] + dthresh;
                                if (v2 > 235) v2 = 235; 
                                if ((v1 > dstpp[x] && v1 > dstpn[x]) || (v2 < dstpp[x] && v2 < dstpn[x]))
                                {
                                        if (post == POST_FULL_MAP || post == POST_FULL_NOMATCH_MAP)
                                        {
                                                if(0) // (vi.IsYUY2())
                                                {
                                                        if (x & 1) dstp[x] = 128;
                                                        else dstp[x] = 235;
                                                }
                                                else
                                                {
                                                        dstp[x] = 235;
                                                }
                                        }
                                        else
                                                dstp[x] = (dstpp[x] + dstpn[x]) >> 1;
                                }
                        }
                        dstp += 2*dpitch;
                        dstpp += 2*dpitch;
                        dstpn += 2*dpitch;
                }

               // if (vi.IsYV12())
                {
                        // Chroma planes.
                        for (z = 0; z < 2; z++)
                        {
                                if (z == 0) dstp = dst->GetWritePtr(PLANAR_U) + dpitch/2;
                                else dstp = dst->GetWritePtr(PLANAR_V) + dpitch/2;
                                dstpp = dstp - dpitch/2;
                                dstpn = dstp + dpitch/2;
                                for (y = 1; y < hover2 - 1; y+=2)
                                {
                                        for (x = 0; x < wover2; x++)
                                        {
                                                v1 = (int) dstp[x] - dthresh;
                                                if (v1 < 0) v1 = 0; 
                                                v2 = (int) dstp[x] + dthresh;
                                                if (v2 > 235) v2 = 235; 
                                                if ((v1 > dstpp[x] && v1 > dstpn[x]) || (v2 < dstpp[x] && v2 < dstpn[x]))
                                                {
                                                        if (post == POST_FULL_MAP || post == POST_FULL_NOMATCH_MAP)
                                                        {
                                                                dstp[x] = 128;
                                                        }
                                                        else
                                                                dstp[x] = (dstpp[x] + dstpn[x]) >> 1;
                                                }
                                        }
                                        dstp += dpitch;
                                        dstpp += dpitch;
                                        dstpn += dpitch;
                                }
                        }
                }
        }

        if (show == true) Show(dst, frame);
        if (debug == true) Debug(frame);
        if (hints == true) WriteHints(dst->GetWritePtr(PLANAR_Y), film, inpattern);
        vidCache->unlockAll();
        //return dst;
        return 1;
}
// EOF
