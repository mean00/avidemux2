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
/**
    \fn computeDiff
    \brief compute difference between image and its predecessor
*/
uint32_t Decimate::computeDiff(ADMImage *current,ADMImage *previous)
{
    uint8_t *prevY = previous->GetReadPtr(PLANAR_Y);
    uint8_t *currY = current->GetReadPtr(PLANAR_Y);
    uint32_t prevPitch=previous->GetPitch(PLANAR_Y);
    uint32_t curPitch=current->GetPitch(PLANAR_Y);
    deciMate *_param=&configuration;
    // Zero
    for (int y = 0; y < yblocks; y++)
    {
        for (int x = 0; x < xblocks; x++)
        {
            sum[y*xblocks+x] = 0;
        }
    }
    // Raw diff
    int height=info.height;
    int width=info.width;
    int inc=1;
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width;)
        {
            sum[(y/BLKSIZE)*xblocks+x/BLKSIZE] += abs((int)currY[x] - (int)prevY[x]);
            x++;
            if(!(x&3))
                if (_param->quality == 0 || _param->quality == 1)
                {
                    x+=12;
                }
        }
        prevY += prevPitch;
        currY += curPitch;
    }
    if (_param->quality == 1 || _param->quality == 3)
    {
#warning DO CHROMA SAMPLING
#if 0
        // also do u & v
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
#endif
    }
    uint32_t highest_sum = 0;
    for (int y = 0; y < yblocks; y++)
    {
        for (int x = 0; x < xblocks; x++)
        {
            if (sum[y*xblocks+x] > highest_sum)
            {
                highest_sum = sum[y*xblocks+x];
            }
        }
    }
    return highest_sum;
}
/**
    \fn FindDuplicate
*/
void Decimate::FindDuplicate(int frame, int *chosen, double *metric, bool *forced)
{
	int f;
	ADMImage  * store[MAX_CYCLE_SIZE+1];
    deciMate  *_param=&configuration;
	int          lowest_index, div;
	unsigned int count[MAX_CYCLE_SIZE], lowest;
	bool         found;
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
    ADMImage *lastImage=NULL;
	for (f = 0; f <= _param->cycle; f++)
	{
		GETFRAME(frame + f - 1, store[f]);
        if(store[f]) lastImage=store[f];
            else store[f]=lastImage;
        hints_invalid=GetHintingData(lastImage->GetReadPtr(PLANAR_Y),&hints[f]);
	}

    if(!lastImage) 
    {
        *chosen=-1;
        ADM_info("Cannot get input image\n");
        return;
    }

    int row_sizeY = info.width; //store[0]->GetRowSize(PLANAR_Y);
    int heightY = info.height; //store[0]->GetHeight(PLANAR_Y);

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
		count[f-1] = computeDiff(store[f],store[f-1]);
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
	for (int x = 1; x < _param->cycle; x++)
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
    return;
}
/**
    \fn FindDuplicate2
*/
void Decimate::FindDuplicate2(int frame, int *chosen, bool *forced)
{
	int f, g, fsum, bsum, highest, highest_index;
	ADMImage * store[MAX_CYCLE_SIZE+1];
	const unsigned char *prevY, *prevU, *prevV, *currY, *currU, *currV;
	int x, y;
	double lowest;
	unsigned int lowest_index;
	char buf[255];
	unsigned int highest_sum;
	bool found;
#define BLKSIZE 32
    deciMate *_param=&configuration;
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
		for (f = 1; f <= _param->cycle; f++)
		{
			GETFRAME(frame + f - 1, store[f]);
		}

		int row_sizeY = info.width; //store[0]->GetRowSize(PLANAR_Y);
		int heightY = info.height; //store[0]->GetHeight(PLANAR_Y);

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
			highest_sum = computeDiff(store[f],store[f-1]);
			metrics[f-1] = (highest_sum * 100.0) / div;
		}

		Dcurr[0] = 1;
		for (f = 1; f < _param->cycle; f++)
		{
			if (metrics[f] < _param->threshold2) Dcurr[f] = 0;
			else Dcurr[f] = 1;
		}

		if (configuration.debug)
		{
			OutputDebugString(buf,"Decimate: %d: %3.2f %3.2f %3.2f %3.2f %3.2f\n",
					0, metrics[0], metrics[1], metrics[2], metrics[3], metrics[4]);
			
		}
	} // / !frame || first time
	else
	{
		GETFRAME(frame + _param->cycle - 1, store[0]);
		for (f = 0; f < MAX_CYCLE_SIZE; f++) Dprev[f] = Dcurr[f];
		for (f = 0; f < MAX_CYCLE_SIZE; f++) Dcurr[f] = Dnext[f];
	}
	for (f = 0; f < MAX_CYCLE_SIZE; f++) Dshow[f] = Dcurr[f];
	for (f = 0; f < MAX_CYCLE_SIZE; f++) showmetrics[f] = metrics[f];

	for (f = 1; f <= _param->cycle; f++)
	{
		GETFRAME(frame + f + _param->cycle - 1, store[f]);
	}

	/* Compare each frame to its predecessor. */
	for (f = 1; f <= _param->cycle; f++)
	{
        highest_sum=computeDiff(store[f],store[f-1]);
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

	if (configuration.debug)
	{
		OutputDebugString("Decimate: %d: %3.2f %3.2f %3.2f %3.2f %3.2f\n",
		        frame + 5, metrics[0], metrics[1], metrics[2], metrics[3], metrics[4]);
		
	}

	if (configuration.debug)
	{
		OutputDebugString("Decimate: %d: %d %d %d %d %d\n",
		        frame, Dcurr[0], Dcurr[1], Dcurr[2], Dcurr[3], Dcurr[4]);
//		sprintf(buf,"Decimate: %d: %d %d %d %d %d - %d %d %d %d %d - %d %d %d %d %d\n",
//		        frame, Dprev[0], Dprev[1], Dprev[2], Dprev[3], Dprev[4],
//					   Dcurr[0], Dcurr[1], Dcurr[2], Dcurr[3], Dcurr[4],
//					   Dnext[0], Dnext[1], Dnext[2], Dnext[3], Dnext[4]);
		
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
	if (configuration.debug)
	{
		OutputDebugString("Decimate: dropping frame %d\n", last_result);
		
	}

	
	found = false;
	
	if (found == true)
	{
		*chosen = last_result ;
		*forced = last_forced = true;
		if (configuration.debug)
		{
			OutputDebugString("Decimate: overridden drop frame -- drop %d\n", last_result);
		}
	}
}
/**
    \fn DrawShow
*/
void Decimate::DrawShow(ADMImage  *src, int useframe, bool forced, int dropframe,
						double metric, int inframe)
{
	char buf[80];
    deciMate *_param=&configuration;
	int start = (useframe / _param->cycle) * _param->cycle;

	if (configuration.show == true)
	{
		sprintf(buf, "Decimate %d", 0); 	DrawString(src, 0, 0, buf);
		sprintf(buf, "Copyright 2003 Donald Graft");	    DrawString(src, 0, 1, buf);
		sprintf(buf,"%d: %3.2f", start, showmetrics[0]);	DrawString(src, 0, 3, buf);
		sprintf(buf,"%d: %3.2f", start + 1, showmetrics[1]);DrawString(src, 0, 4, buf);
		sprintf(buf,"%d: %3.2f", start + 2, showmetrics[2]);DrawString(src, 0, 5, buf);
		sprintf(buf,"%d: %3.2f", start + 3, showmetrics[3]);DrawString(src, 0, 6, buf);
		sprintf(buf,"%d: %3.2f", start + 4, showmetrics[4]);DrawString(src, 0, 7, buf);
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
			sprintf(buf,"in frm %d", inframe);			                    DrawString(src, 0, 8, buf);
			sprintf(buf,"chose %d, decimating all-video cycle", dropframe);	DrawString(src, 0, 9, buf);
		}
	}
	if (configuration.debug)
	{
		if (!(inframe%_param->cycle))
		{
			OutputDebugString(buf,"Decimate: %d: %3.2f\n", start, showmetrics[0]);
			OutputDebugString(buf,"Decimate: %d: %3.2f\n", start + 1, showmetrics[1]);
			OutputDebugString(buf,"Decimate: %d: %3.2f\n", start + 2, showmetrics[2]);
			OutputDebugString(buf,"Decimate: %d: %3.2f\n", start + 3, showmetrics[3]);
			OutputDebugString(buf,"Decimate: %d: %3.2f\n", start + 4, showmetrics[4]);
			
		}
		if (all_video_cycle == false)
		{
			OutputDebugString(buf,"Decimate: in frm %d useframe %d\n", inframe, useframe);
			if (forced == false)
            {
				OutputDebugString("Decimate: chose %d, dropping\n", dropframe);
            }
			else
            {
				OutputDebugString("Decimate: chose %d, dropping, forced!\n", dropframe);
            }
		}
		else
		{
			OutputDebugString("Decimate: in frm %d\n", inframe);
			OutputDebugString("Decimate: chose %d, decimating all-video cycle\n", dropframe);
		}
	}
}
// EOF

