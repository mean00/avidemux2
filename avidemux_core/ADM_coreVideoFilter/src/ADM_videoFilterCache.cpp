/** *************************************************************************
        \file                  ADM_videoFilterCache.cpp
        \brief Cache/buffer for video filter
		\author (c) 2008/2010 Mean, fixounet@free.fr

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
#include "ADM_videoFilterCache.h"
#include "ADM_coreVideoFilter.h"
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

VideoCache::VideoCache(uint32_t nb,ADM_coreVideoFilter *in)
{
uint32_t sz;
	nbEntry=nb;
	incoming=in;
	// Ready buffers
	entry=new vidCacheEntry[nbEntry];
        uint32_t w=in->getInfo()->width;
        uint32_t h=in->getInfo()->height;

	sz=(w*h*3)>>1;
	for(uint32_t i=0;i<nbEntry;i++)
	{
		entry[i].image	=new ADMImage(w,h);	
		entry[i].frameNum	=0xffff0000;
		entry[i].frameLock	=0;
	}	
	counter=0;
}
//_____________________________________________
VideoCache::~ VideoCache()
{
	for(uint32_t i=0;i<nbEntry;i++)
	{
		delete  entry[i].image;
	}
	delete [] entry;
	
}
//_____________________________________________
int32_t VideoCache::searchFrame( uint32_t frame)
{
	for(uint32_t i=0;i<nbEntry;i++)
	{
		if(entry[i].frameNum==frame) return i;
	}
	return -1;
}
//_____________________________________________
int32_t 	 VideoCache::searchPtr( ADMImage *ptr)
{
	for(uint32_t i=0;i<nbEntry;i++)
	{
		if(entry[i].image==ptr) return i;
	}
	return -1;
}
//_____________________________________________
uint8_t  VideoCache::unlockAll(void)
{
	for(uint32_t i=0;i<nbEntry;i++)
	{		
		entry[i].frameLock=0;		
	}
	return 1;
}
//_____________________________________________
uint8_t  VideoCache::unlock(ADMImage *frame)
{
int32_t k;
	k=searchPtr(frame) ;
	ADM_assert(k>=0);
	entry[k].frameLock--;	
	return 1;	
}
//_____________________________________________
uint8_t  VideoCache::flush(void)
{
	for(uint32_t i=0;i<nbEntry;i++)
	{		
		entry[i].frameLock=0;
		entry[i].frameNum=0xffff0000;	
		entry[i].lastUse=0xffff0000;	
	}
	return 1;

}
/**
    \fn getImage
*/
ADMImage *VideoCache::getImage(uint32_t frame)
{
int32_t i;
uint32_t tryz=nbEntry;
uint32_t len,flags;

	// Already there ?
	if((i=searchFrame(frame))>=0)
	{
        ADMImage *img=entry[i].image;
		aprintf("[cache]  old image  frame %d with PTS=%"LLU"\n",(int)frame,img->Pts);
		entry[i].frameLock++;
		entry[i].lastUse=counter;
		counter++;
		return img;	
	}
	// Else get it!
	
	// First elect a new buffer, we do it by 
	// using a RLU scheme
	
	uint32_t deltamax=0,delta;
	uint32_t target=0xfff;
	aprintf("Cache : Cache miss %d\n",frame);
	//for(uint32_t i=0;i<nbEntry;i++) printf("%d(%d) ",frameNum[i],frameLock[i]);printf("\n");
	for(uint32_t i=0;i<nbEntry;i++)
	{
		if(entry[i].frameLock) continue; 	// don"t consider locked frames
		
		
		delta=abs((int)counter-(int)entry[i].lastUse);
		if(delta>deltamax)
		{
			deltamax=delta;
			target=i;
		}
	}
	ADM_assert(target!=0xfff);
	// Target is the new cache we will use


        uint32_t nb;
        ADMImage *img=entry[target].image;
        if(!incoming->getNextFrame(&nb,img)) return NULL;
        ADM_assert(nb==frame);
        aprintf("[cache] New image Got frame %d with PTS=%"LLU"\n",(int)nb,img->Pts);
	// Update LRU info
	entry[target].frameLock++;
	entry[target].frameNum=nb;
	entry[target].lastUse=counter;
	counter++;	
	return img;
	
}
// EOF
