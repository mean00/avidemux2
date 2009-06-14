/*
 *  TwoLAME: an optimized MPEG Audio Layer Two encoder
 *
 *  Copyright (C) 2001-2004 Michael Cheng
 *  Copyright (C) 2004-2005 The TwoLAME Project
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 */


#include <stdio.h>
#include <stdlib.h>

#include "twolame.h"
#include "common.h"
#include "bitbuffer.h"
#include "energy.h"


void do_energy_levels(twolame_options *glopts, bit_stream *bs)
//void do_energy_levels(twolame_options *glopts, unsigned char *mp2buffer, int frameEnd, short int *leftpcm, short int *rightpcm)
{
  /* Reference:
     Using the BWF Energy Levels in AudioScience Bitstreams
     http://www.audioscience.com/internet/technical/app0001.pdf

     The absolute peak of the PCM file for the left and right
     channel in this frame are written into the last 5 bytes of the frame.
     
     The last 5 bytes *must* be reserved for this to work correctly (otherwise
     you'll be overwriting mpeg audio data)
  */
     

/*  *** FIXME removed due to code re-arrangement ***


  short int *leftpcm = glopts->buffer[0];
  short int *rightpcm = glopts->buffer[1];   
     
  int i;
  int leftMax, rightMax;
  unsigned char rhibyte, rlobyte, lhibyte, llobyte;

  // find the maximum in the left and right channels
  leftMax = rightMax = -1;
  for (i=0; i<1152;i++) { 
    if (abs(leftpcm[i])>leftMax)
      leftMax = abs(leftpcm[i]);
    if (abs(rightpcm[i])>rightMax)
      rightMax = abs(rightpcm[i]);
  }

  // convert max value to hi/lo bytes and write into buffer
  lhibyte = leftMax/256;
  llobyte = leftMax - 256*lhibyte;
  mp2buffer[frameEnd-1] = llobyte;
  mp2buffer[frameEnd-2] = lhibyte;
  
  if (glopts->mode!=MONO) {
    // Only write the right channel energy info
    // if we're in stereo mode.
    mp2buffer[frameEnd-3] = 0;
    
    rhibyte = rightMax/256;
    rlobyte = rightMax - 256*rhibyte;
    mp2buffer[frameEnd-4] = rlobyte;
    mp2buffer[frameEnd-5] = rhibyte;
  }
  
*/
}


