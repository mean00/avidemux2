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
//#include "ADM_libraries/ADM_utilities/avidemutils.h"


#define aprintf(...) {}

#ifdef ADM_DEBUG
	//#define OPENDML_VERBOSE
#endif
#define DEPACK_VERBOSE
#define QT_TR_NOOP(x) x
typedef struct vopS
{
	uint32_t offset;
	uint32_t type;
        uint32_t vopCoded;
        uint32_t modulo;
        uint32_t timeInc;
}vopS;
#define MAX_VOP 10

/* Forward declaration */
uint8_t ADM_findMpegStartCode(uint8_t *start, uint8_t *end,uint8_t *outstartcode,uint32_t *offset);
uint8_t extractVopInfo(uint8_t *data, uint32_t len,uint32_t timeincbits,uint32_t *vopType,uint32_t *modulo, uint32_t *time_inc,uint32_t *vopcoded);
uint8_t extractMpeg4Info(uint8_t *data,uint32_t dataSize,uint32_t *w,uint32_t *h,uint32_t *time_inc);

/* Static ones */
static uint32_t searchVop(uint8_t *begin, uint8_t *end,uint32_t *nb, vopS *vop,uint32_t *timeincbits);
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

	vopS	myVops[MAX_VOP]; // should be enough
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

	uint32_t originalPriority = getpriority(PRIO_PROCESS, 0);
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
            printf("[Avi] Error could not get frame %"LU"\n",img);
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
                if(!searchVop(buffer,buffer+image.dataLength,&nbVop,myVops,&timcincbits))
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
	printf("[Avi] Initial # of images : %"LU", now we have %"LU" \n",nbFrame,targetIndex);
	nbFrame=targetIndex;

	setpriority(PRIO_PROCESS, 0, originalPriority);

	return ret;
}
// Search a start vop in it
// and return also the vop type
// needed to update the index
uint32_t searchVop(uint8_t *begin, uint8_t *end,uint32_t *nb, vopS *vop,uint32_t *timeincbits)
{

	uint32_t off=0;
	uint32_t globalOff=0;
	uint32_t voptype;
	uint8_t code;
        uint32_t w,h,t;
        uint32_t modulo,time_inc,vopcoded,vopType;
	*nb=0;
	while(begin<end-3)
	{
    	if( ADM_findMpegStartCode(begin, end,&code,&off))
    	{
        	if(code==0xb6)
			{
				// Analyse a bit the vop header
				uint8_t coding_type=begin[off];
				coding_type>>=6;
				aprintf("\t at %u %d Img type:%s\n",off,*nb,s_voptype[coding_type]);
				switch(coding_type)
				{
					case 0: voptype=AVI_KEY_FRAME;break;
					case 1: voptype=0;break;
					case 2: voptype=AVI_B_FRAME;break;
					case 3: printf("[Avi] Glouglou\n");voptype=0;break;

				}
        	                vop[*nb].offset=globalOff+off-4;
				vop[*nb].type=voptype;



                                /* Get more info */
                                if( extractVopInfo(begin+off, end-begin-off, *timeincbits,&vopType,&modulo, &time_inc,&vopcoded))
                                {
                                    aprintf(" frame found: vopType:%x modulo:%d time_inc:%d vopcoded:%d\n",vopType,modulo,time_inc,vopcoded);
                                    vop[*nb].modulo=modulo;
                                    vop[*nb].timeInc=time_inc;
                                    vop[*nb].vopCoded=vopcoded;
                                }
                                *nb=(*nb)+1;
                                begin+=off+1;
				globalOff+=off+1;
				continue;

			}
                else if(code==0x20 && off>=4	) // Vol start
                {

                   if(extractMpeg4Info(begin+off-4,end+4-off-begin,&w,&h,timeincbits))
                   {
                      aprintf("Found Vol header : w:%d h:%d timeincbits:%d\n",w,h,*timeincbits);
                   }

                }
        	begin+=off;
        	globalOff+=off;
        	continue;
    	}
    	return 1;
    }
	return 1;
}
