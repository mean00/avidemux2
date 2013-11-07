/***************************************************************************

    \file ADM_openDMLDepack
    \brief Removed packed bitstream stuff
    copyright            : (C) 2001/2008 by mean
    email                : fixounet@free.fr

This class deals with a chunked / not chunked stream
It is an fopen/fwrite lookalike interface to chunks



 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_Video.h"


#include "fourcc.h"
#include "ADM_openDML.h"

#include "DIA_working.h"
#include "ADM_videoInfoExtractor.h"


#define aprintf(...) {}

#ifdef ADM_DEBUG
	//#define OPENDML_VERBOSE
#endif
#define DEPACK_VERBOSE

#define MAX_VOP 200

/* Static ones */

static const char *s_voptype[4]={"I frame","P frame","B frame","D frame"};
/**
    \fn unpackPacked
    \brief Removed packed bitstream hack
*/
uint8_t OpenDMLHeader::unpackPacked( void )
{
	uint32_t nbFrame;
	uint8_t ret=0;
	uint32_t firstType, secondType,thirdType;
	uint32_t targetIndex=0,nbVop;
	uint32_t nbDuped=0;
    uint32_t timcincbits=16;  /* Nb bits used to code time_inc 16 is a safe default */

	ADM_vopS	myVops[MAX_VOP]; // should be enough
	// here we got the vidHeader to get the file easily
	// we only deal with avi now, so cast it to its proper type (i.e. avi)


	// now we are ready to rumble
	// First get a unpack buffe
	uint8_t *buffer=new uint8_t [2*getWidth()*getHeight()];

	// For each frame we lookup x times the VOP header
	// the first one become frame n, the second one becomes frame N+1
	// Image contaning only VOP header are royally ignored
	// We should get about the same number of in/out frame

	nbFrame=getMainHeader()->dwTotalFrames;

	odmlIndex *newIndex=new odmlIndex[nbFrame+MAX_VOP]; // Due to the packed vop, we may end up with more images
							// Assume MAX_VOP Bframes maximum
	ADM_assert(newIndex);

	#ifndef __HAIKU__
	uint32_t originalPriority = getpriority(PRIO_PROCESS, 0);
	#endif
	uint32_t priorityLevel;
#if 0
	prefs->get(PRIORITY_INDEXING,&priorityLevel);
	setpriority(PRIO_PROCESS, 0, ADM_getNiceValue(priorityLevel));
#endif
	printf("[Avi] Trying to unpack the stream\n");
	DIA_workingBase *working=createWorking(QT_TR_NOOP("Unpacking bitstream"));
	ADMCompressedImage image;
    image.data=buffer;
	uint32_t img=0;
    uint32_t modulo,time_inc,vopcoded,vopType;
    uint32_t timeincbits=16;
    uint32_t oldtimecode=0xffffffff;
	while(img<nbFrame)
	{
        ADM_assert(nbDuped<2);
		working->update(img,nbFrame);
		if(!getFrame(img,&image))
        {
            printf("[Avi] Error could not get frame %"PRIu32"\n",img);
            goto _abortUnpack;
        }
		aprintf("--Frame:%lu/%lu, len %lu, nbDuped%u\n",img,nbFrame,image.dataLength,nbDuped);

		if(image.dataLength<=2)
                {
                  if(nbDuped)
                  {
                    aprintf("Skipping null frame\n");
                    nbDuped--;
                    img++;
                    continue;
                  }
                }
		if(image.dataLength<6) // Too short to contain a valid vop header, just copy
		{
                                memcpy(&newIndex[targetIndex],&_idx[img],sizeof(_idx[0]));
				aprintf("TOO SMALL\n");
				targetIndex++;
                                img++;
                                continue;
                }
                /* Cannot find vop, corrupted or WTF ...*/
                if(!ADM_searchVop(buffer,buffer+image.dataLength,&nbVop,myVops,&timcincbits))
                {
                    printf("[Avi] img :%u failed to find vop!\n",img);
                    memcpy(&newIndex[targetIndex],&_idx[img],sizeof(_idx[0]));
                    targetIndex++;
                    img++;
                    continue;

                }
                /* We have one or more vop inside it...*/
                if(nbVop==1 && nbDuped) // only one vop, could it be an evil duplicate ?
                {
                        if(myVops[0].timeInc==oldtimecode && !myVops[0].vopCoded)
                        {
                          aprintf("Frame has same timecode and is not vop coded; skipping\n");
                          img++;
                          nbDuped--;
                          continue;
                        }
                }

		// more than one vop, do up to the n-1th
		// the 1st image starts at 0
		myVops[0].offset=0;
		myVops[nbVop].offset=image.dataLength;


		uint32_t place;
                //if(nbVop>2)
                {
                        aprintf("At %u, %d vop!\n",img,nbVop);
                }
                /* The 1st one becomes our new timecode reference, a dupe will have the same timebase */
                if(myVops[0].type!=AVI_B_FRAME)
                    oldtimecode=myVops[0].timeInc;

                for(uint32_t j=0;j<nbVop;j++)
                {


                          if(!j)
                                newIndex[targetIndex].intra=myVops[j].type;
                        else
                                newIndex[targetIndex].intra=AVI_B_FRAME;
                        newIndex[targetIndex].size=myVops[j+1].offset-myVops[j].offset;
                        newIndex[targetIndex].offset=_idx[img].offset+myVops[j].offset;

                        if(j)
                        {
                          if(nbDuped)
                          {
                              printf("[Avi] WARNING*************** Missing one NVOP, dropping one b frame instead  at image %u\n",img);
                              nbDuped--;
                          }else
                          {
                              nbDuped++;
                              targetIndex++;
                          }
                        } else
                        {
                         targetIndex++;
                        }
                }

                img++;


	}
	newIndex[0].intra=AVI_KEY_FRAME; // Force
	ret=1;
_abortUnpack:
	delete [] buffer;
	delete working;
#if 0
	for(uint32_t k=0;k<nbFrame;k++)
	{
		printf("%d old:%lu new: %lu \n",_idx[k].size,newIndex[k].size);
	}
#endif
	if(ret==1)
	{
		printf("[Avi] Sucessfully unpacked the bitstream\n");

		delete [] _idx;
		_idx=newIndex;
	}
	else
	{
		delete [] newIndex;
		printf("[Avi] Could not unpack this...\n");
	}
	printf("[Avi] Initial # of images : %"PRIu32", now we have %"PRIu32" \n",nbFrame,targetIndex);
	nbFrame=targetIndex;

	#ifndef __HAIKU__
	setpriority(PRIO_PROCESS, 0, originalPriority);
	#endif

	return ret;
}
