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
    \fn FindDuplicate
*/
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
/**
    \fn FindDuplicate2
*/
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
// EOF

