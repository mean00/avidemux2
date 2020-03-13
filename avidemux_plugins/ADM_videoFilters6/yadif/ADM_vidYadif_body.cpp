/*
 * Copyright (C) 2006-2011 Michael Niedermayer <michaelni@gmx.at>
 *               2010      James Darnley <james.darnley@gmail.com>

 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


#define MAX_ALIGN 8

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
bool yadifFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{

        int mode;
        int parity;
        int tff;
        int iplane;
        int n;
        ADMImage *src, *dst, * prev, *next;
        
    
        mode = configuration.mode;

        if (mode & 1) 
        {
                n = (nextFrame>>1); // bob
        }
        else
                n = nextFrame;

        src = vidCache->getImage(n);
        *fn=nextFrame;
        if(!src) return false;
        
  
        // If possible get previous image...
        if (n>0)
                prev =  vidCache->getImage( n-1); // get previous frame
        else
                prev= src; // get very first frame

        ADM_assert(prev);
        next=vidCache->getImage(n+1);
        if(!next) next=src;
        ADM_assert(next);
        
        dst = image;
        dst->copyInfo(src);

        if(!prev || !src || !next)
        {
            printf("Failed to read frame for frame %u\n",nextFrame);
            vidCache->unlockAll();
            return 0;
        }
        
  // Construct a frame based on the information of the current frame
  // contained in the "vi" struct.
#if 0 //MEANX
        if (configuration.order == -1)
//		tff = avs_is_tff(&p->vi) == 0 ? 0 : 1; // 0 or 1
                tff = avs_get_parity(p->child, n) ? 1 : 0; // 0 or 1
        else
                tff = configuration.parity;	
#endif
        tff = (configuration.parity > 0) ? 0 : 1;
        parity = (mode & 1) ? (nextFrame & 1) ^ (1^tff) : (tff ^ 1);  // 0 or 1

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
      
      if (mode & 1) 
      {
            if(nextFrame&1)
                image->Pts+= info.frameIncrement;
      }
      //printf("out PTs=%"PRIu64", nextFrame=%d,inc=%d\n",image->Pts,(int)nextFrame,(int)info.frameIncrement);
      nextFrame++;
      filter_end();
      return 1;
}


void yadifFilter::filter_plane(int mode, uint8_t *dst, int dst_stride, const uint8_t *prev0, const uint8_t *cur0, const uint8_t *next0, int refs, int w, int h, int parity, int tff, int mmx)
{
        int df = 1;
        int pix_3 = 3 * df;
        int edge = 3 + MAX_ALIGN / df - 1;
        //memcpy(dst, cur0, w);
        //memcpy(dst + dst_stride, cur0 + refs, w);
        for(int y=0; y<h; y++){
            if(((y ^ parity) & 1)){
                const uint8_t *prev= prev0 + y*refs;
                const uint8_t *cur = cur0 + y*refs;
                const uint8_t *next= next0 + y*refs;
                uint8_t *dst2= dst + y*dst_stride;
                int mode = y == 1 || y + 2 == h ? 2 : 0;
                filter_line(dst2 + pix_3, prev + pix_3, cur + pix_3,   next + pix_3, 
                             w - edge,
                           y + 1 < h ? refs : -refs,
                           y ? -refs : refs,
                           parity ^ tff, mode);
                filter_edges(dst2, prev, cur, next, 
                             w,
                            y + 1 < h ? refs : -refs,
                            y ? -refs : refs,
                            parity ^ tff, mode);                
                
            }else{
                memcpy(dst + y*dst_stride, cur0 + y*refs, w);
            }
        }
        //memcpy(dst + (h-1)*dst_stride, cur0 + (h-1)*refs, w);

}

//--- ff ---
#define CHECK(j)\
    {   int score = FFABS(cur[mrefs - 1 + (j)] - cur[prefs - 1 - (j)])\
                  + FFABS(cur[mrefs  +(j)] - cur[prefs  -(j)])\
                  + FFABS(cur[mrefs + 1 + (j)] - cur[prefs + 1 - (j)]);\
        if (score < spatial_score) {\
            spatial_score= score;\
            spatial_pred= (cur[mrefs  +(j)] + cur[prefs  -(j)])>>1;\

/* The is_not_edge argument here controls when the code will enter a branch
 * which reads up to and including x-3 and x+3. */

#define FILTER(start, end, is_not_edge) \
    for (x = start;  x < end; x++) { \
        int c = cur[mrefs]; \
        int d = (prev2[0] + next2[0])>>1; \
        int e = cur[prefs]; \
        int temporal_diff0 = FFABS(prev2[0] - next2[0]); \
        int temporal_diff1 =(FFABS(prev[mrefs] - c) + FFABS(prev[prefs] - e) )>>1; \
        int temporal_diff2 =(FFABS(next[mrefs] - c) + FFABS(next[prefs] - e) )>>1; \
        int diff = FFMAX3(temporal_diff0 >> 1, temporal_diff1, temporal_diff2); \
        int spatial_pred = (c+e) >> 1; \
 \
        if (is_not_edge) {\
            int spatial_score = FFABS(cur[mrefs - 1] - cur[prefs - 1]) + FFABS(c-e) \
                              + FFABS(cur[mrefs + 1] - cur[prefs + 1]) - 1; \
            CHECK(-1) CHECK(-2) }} }} \
            CHECK( 1) CHECK( 2) }} }} \
        }\
 \
        if (!(mode&2)) { \
            int b = (prev2[2 * mrefs] + next2[2 * mrefs])>>1; \
            int f = (prev2[2 * prefs] + next2[2 * prefs])>>1; \
            int max = FFMAX3(d - e, d - c, FFMIN(b - c, f - e)); \
            int min = FFMIN3(d - e, d - c, FFMAX(b - c, f - e)); \
 \
            diff = FFMAX3(diff, min, -max); \
        } \
 \
        if (spatial_pred > d + diff) \
           spatial_pred = d + diff; \
        else if (spatial_pred < d - diff) \
           spatial_pred = d - diff; \
 \
        dst[0] = spatial_pred; \
 \
        dst++; \
        cur++; \
        prev++; \
        next++; \
        prev2++; \
        next2++; \
    }

void filter_line_c(uint8_t  *dst1,
                          const uint8_t *prev1, const uint8_t *cur1, const uint8_t *next1,
                          int w, int prefs, int mrefs, int parity, int mode)
{
    uint8_t *dst  = (uint8_t *) dst1;
    uint8_t *prev = (uint8_t *)prev1;
    uint8_t *cur  = (uint8_t *)cur1;
    uint8_t *next = (uint8_t *)next1;
    int x;
    uint8_t *prev2 = parity ? prev : cur ;
    uint8_t *next2 = parity ? cur  : next;

    /* The function is called with the pointers already pointing to data[3] and
     * with 6 subtracted from the width.  This allows the FILTER macro to be
     * called so that it processes all the pixels normally.  A constant value of
     * true for is_not_edge lets the compiler ignore the if statement. */
    FILTER(0, w, 1)
}


void filter_edges_c(uint8_t  *dst1, const uint8_t *prev1, const  uint8_t *cur1, const  uint8_t *next1,
                         int w, int prefs, int mrefs, int parity, int mode)
{
    uint8_t *dst  = (uint8_t *)dst1;
    uint8_t *prev = (uint8_t *)prev1;
    uint8_t *cur  = (uint8_t *)cur1;
    uint8_t *next = (uint8_t *)next1;
    int x;
    uint8_t *prev2 = parity ? prev : cur ;
    uint8_t *next2 = parity ? cur  : next;

    const int edge = MAX_ALIGN - 1;

    /* Only edge pixels need to be processed here.  A constant value of false
     * for is_not_edge should let the compiler ignore the whole branch. */
    FILTER(0, 3, 0)

    dst  = (uint8_t*)dst1  + w - edge;
    prev = (uint8_t*)prev1 + w - edge;
    cur  = (uint8_t*)cur1  + w - edge;
    next = (uint8_t*)next1 + w - edge;
    prev2 = (uint8_t*)(parity ? prev : cur);
    next2 = (uint8_t*)(parity ? cur  : next);

    FILTER(w - edge, w - 3, 1)
    FILTER(w - 3, w, 0)
}

