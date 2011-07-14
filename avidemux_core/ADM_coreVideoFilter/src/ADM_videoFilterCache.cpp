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
/**
    \fn ctor
*/
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
		entry[i].image	=new ADMImageDefault(w,h);	
		entry[i].frameNum	=0xffff0000;
		entry[i].frameLock	=0;
        entry[i].freeEntry=true;
	}	
	counter=0;
}
/**
    \fn dtor
*/
VideoCache::~ VideoCache()
{
	for(uint32_t i=0;i<nbEntry;i++)
	{
		delete  entry[i].image;
	}
	delete [] entry;
    entry=NULL;
	
}
/**
    \fn searchFrame
    \brief Search an entry by its frameNumber
*/
int32_t VideoCache::searchFrame( uint32_t frame)
{
	for(uint32_t i=0;i<nbEntry;i++)
	{
		if(entry[i].frameNum==frame&& entry[i].freeEntry==false) return i;
	}
	return -1;
}
//_____________________________________________
int32_t 	 VideoCache::searchPtr( ADMImage *ptr)
{
	for(uint32_t i=0;i<nbEntry;i++)
	{
		if(entry[i].image==ptr && entry[i].freeEntry==false) return i;
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
/**
    \fn flush
    \brief Empty cache
*/
uint8_t  VideoCache::flush(void)
{
    printf("Flushing video Cache\n");
	for(uint32_t i=0;i<nbEntry;i++)
	{		
		entry[i].frameLock=0;
		entry[i].frameNum=0xffff0000;	
		entry[i].lastUse=0xffff0000;	
        entry[i].freeEntry=true;
	}
	return 1;

}
/**
     \fn searchFreeEntry

*/
int VideoCache::searchFreeEntry(void)
{
    // Search a free one
    for(uint32_t i=0;i<nbEntry;i++)
	{
        if(entry[i].freeEntry==true) return i;
    }
    // Search the oldest one
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
    return target;
}

/**
    \fn getImageBase
*/
ADMImage *VideoCache::getImageBase(uint32_t frame)
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
	int target=searchFreeEntry();
    uint32_t nb;
    ADMImage *img=entry[target].image;
    if(!incoming->getNextFrameAs(ADM_HW_ANY,&nb,img)) return NULL;
    if(nb!=frame)
    {
        ADM_error("Cache inconsistency :\n");
        ADM_error("Expected to get frame %d from filter, got frame %d instead\n",(int)frame,(int)nb);
        dump();
        ADM_assert(0);
    }
    ADM_assert(nb==frame);
    aprintf(">>>>>>>>>>>>>>>>>>>>>>>>>>[cache] New image Got frame %d with PTS=%"LLU"\n",(int)nb,img->Pts);
    // Update LRU info
    entry[target].frameLock++;
    entry[target].frameNum=nb;
    entry[target].lastUse=counter;
    entry[target].freeEntry=false;
    counter++;	
    return img;
}
/**
    \fn getImageAs
*/
ADMImage *VideoCache::getImage(uint32_t frame)
{
    return getImageAs(ADM_HW_NONE,frame);
}

/**
    \fn getImageAs
*/
ADMImage *VideoCache::getImageAs(ADM_HW_IMAGE type,uint32_t frame)
{
    ADMImage *img=getImageBase(frame);
    if(!img) return NULL;
    if(type==ADM_HW_ANY) return img;
    if(type!=img->refType)
    {
        img->hwDownloadFromRef();
    }
    return img;
}   

/**
    \fn dump
*/
void VideoCache::dump(void)
{
	for(uint32_t i=0;i<nbEntry;i++)
	{		
        printf("Entry %d/%d, frameNum %d lock %d lastUse %d\n",
                i,nbEntry,
                
                (int)entry[i].frameNum,
                (int)entry[i].frameLock,
                (int)entry[i].lastUse);
	}

}
// EOF
