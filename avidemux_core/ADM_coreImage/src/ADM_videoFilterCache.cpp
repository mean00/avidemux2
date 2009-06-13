//
// C++ Implementation: ADM_cache
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ADM_default.h"

//#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

VideoCache::VideoCache(uint32_t nb,AVDMGenericVideoStream *in)
{
uint32_t sz;
	nbEntry=nb;
	incoming=in;
	memcpy(&info,in->getInfo(),sizeof(info));
	// Ready buffers
	entry=new vidCacheEntry[nbEntry];
	sz=(info.width*info.height*3)>>1;
	for(uint32_t i=0;i<nbEntry;i++)
	{
		entry[i].frameBuffer	=new ADMImage(info.width,info.height);	
		entry[i].frameNum	=0xffff0000;
		entry[i].frameLock	=0;
		entry[i].lastUse	=0xffff0000;
	}	
	counter=0;
}
//_____________________________________________
VideoCache::~ VideoCache()
{
	for(uint32_t i=0;i<nbEntry;i++)
	{
		delete  entry[i].frameBuffer;
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
		if(entry[i].frameBuffer==ptr) return i;
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
uint8_t  VideoCache::purge(void)
{
	for(uint32_t i=0;i<nbEntry;i++)
	{		
		entry[i].frameLock=0;
		entry[i].frameNum=0xffff0000;	
		entry[i].lastUse=0xffff0000;	
	}
	return 1;

}
//_____________________________________________
ADMImage *VideoCache::getImage(uint32_t frame)
{
int32_t i;
uint32_t tryz=nbEntry;
uint32_t len,flags;
ADMImage *ptr=NULL;

	// Already there ?
	if((i=searchFrame(frame))>=0)
	{
		aprintf("Cache : Cache hit %d buffer %d\n",frame,i);
		entry[i].frameLock++;
		entry[i].lastUse=counter;
		counter++;
		return entry[i].frameBuffer;	
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

	ptr=entry[target].frameBuffer;
	if(!incoming->getFrameNumberNoAlloc(frame,&len,ptr,&flags)) return NULL;
	// Update LRU info
	entry[target].frameLock++;
	entry[target].frameNum=frame;
	entry[target].lastUse=counter;
	counter++;	
	return ptr;
	
}

