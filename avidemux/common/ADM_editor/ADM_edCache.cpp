/**
    \file edCache
    \brief Handle internal cache for decoded image
    \author mean fixounet@free.fr (c) 2003-2010

   Simple fifo queue for dedcoded image with helper functions to search image in it.
*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_image.h"
#include "ADM_editor/ADM_edCache.h"
#define ADM_NO_PTS 0xffffffffffffffffLL

#if 1
#define aprintf(...) {}
#define aADM_warning(...) {}
#else
#define aprintf printf
#define aADM_warning(...) ADM_warning
#endif
/**
    \fn EditorCache
    \brief Constructor
*/

EditorCache::EditorCache(uint32_t size,uint32_t w, uint32_t h)
{
	_elem=new cacheElem[size];
	for(uint32_t i=0;i<size;i++)
	{
		_elem[i].image=new ADMImage(w,h);
		_elem[i].pts=ADM_NO_PTS;
	}
	_nbImage=size;
}
/**
    \fn EditorCache
    \brief Destructor
*/
EditorCache::~EditorCache(void)
{
	for(uint32_t i=0;i<_nbImage;i++)
	{
		delete _elem[i].image;
	}
	delete[] _elem;
}
/**

*/
void EditorCache::check()
{
    if(readIndex==writeIndex) // empty
    {
        readIndex=writeIndex=0;
    }
    // Wrap
    if(writeIndex &0x80000000)
    {
        int min=readIndex;
        if(writeIndex<min) min=writeIndex;
        int mul=min/_nbImage;
        int start=mul*_nbImage;
        readIndex-=start;
        writeIndex-=start;
    }
}
/**
    \fn getFreeImage
    \brief  Get the LRUed iimage in the cache
                the cache is big enough to be immune
                to reuse in the same go
*/
ADMImage	*EditorCache::getFreeImage(void)
{
	uint32_t min=0;
	uint64_t  delta=0;
    int found=-1;
    check();
    // next!
    int r,w;
    r=readIndex%_nbImage;
    w=(writeIndex)%_nbImage;
    aprintf("Read: %"LU" write :%"LU"\n",readIndex,writeIndex);
    if(r==w && readIndex!=writeIndex) //full
    {
        readIndex++; // free older
        aprintf("Erasing read\n");
    }
    found=writeIndex%_nbImage;
    
    // Mark it as used
    if(found==-1) ADM_assert(0);
	_elem[found].pts=ADM_NO_PTS;;
    writeIndex++;
    aprintf("Using free image at index %d\n",found);
	return _elem[found].image;

}
/**
    \fn flush
    \brief Empty cache, mark all slots as not used

*/
 void        EditorCache::flush(void)
{
    printf("[edCache] Flush\n");
    for(int i=0;i<_nbImage;i++)
    {
        _elem[i].pts=ADM_NO_PTS;
    }
    writeIndex=readIndex=0;
}
/**
    \fn invalidate
    \brief invalidate one line of cache
*/
void        EditorCache::invalidate(ADMImage *image)
{
    for(int i=0;i<_nbImage;i++)
    {
        if(_elem[i].image==image)
            {
                uint32_t prev=(writeIndex+_nbImage-1)%_nbImage;
                 ADM_assert(i==prev);
                 ADM_assert(_elem[i].pts==ADM_NO_PTS);
                 aprintf("Invalidate writeIndex %"LU"\n",writeIndex);
                 writeIndex--;
                 return;
            }
    }
    printf("[edCache]Image not in cache\n");
    ADM_assert(0);
}
/**
        \fn updateFrameNum
        \brief update the frameNo associated to a cache line (obsolete)
               Only used to mark it as valid
*/
bool		EditorCache::validate(ADMImage *image)
{
	for(uint32_t i=0;i<_nbImage;i++)
	{
		if(_elem[i].image==image)
		{
            ADM_assert(_elem[i].pts==ADM_NO_PTS);
			_elem[i].pts=image->Pts;
            aprintf("validate Index %"LU" with pts=%"LLU"ms\n",i,image->Pts);
			return true;
		}

	}
	ADM_assert(0);
    return false;
}
/**
    \fn dump
    \brief dump the content of cache in a human readable way (sort of)
*/
void EditorCache::dump( void)
{
    printf("ReadIndex:%"LU", WriteIndex:%"LU"\n",readIndex,writeIndex);
	for(int i=0;i<_nbImage;i++)
	{
        cacheElem *e=&(_elem[i]);
        switch(e->pts)
        {
            case ADM_NO_PTS:  printf("Not used %d\n",i);break;
            default:
                printf("Edcache content[%d]: PTS : %"LLU" us%"LLU" ms\n",i,
                                                                    e->image->Pts,e->image->Pts/1000);
        }
	}
}
/**
    \fn findJustAfter
    \brief Find the image with the closest PTS just above pts.

*/
ADMImage    *EditorCache::getAfter(uint64_t pts)
{
    for(uint32_t i=readIndex;i<writeIndex-1;i++)
	{
        int index=i%_nbImage;
        ADM_assert(_elem[index].pts!=ADM_NO_PTS);
        if(_elem[index].pts==pts)
        {
            index++;
            index%=_nbImage;
            return _elem[index].image;
        }
    }
    aADM_warning("Cannot find image after %"LLU" ms in cache\n",pts/1000);
    return NULL;
}
/**
    \fn findJustAfter
    \brief Find the image with the closest PTS just above pts.

*/
ADMImage    *EditorCache::getBefore(uint64_t pts)
{
    for(int i=readIndex+1;i<writeIndex;i++)
	{
        int index=i%_nbImage;
        ADM_assert(_elem[index].pts!=ADM_NO_PTS);
        if(_elem[index].pts==pts)
        {
            index+=_nbImage-1;
            index%=_nbImage;
            printf("GetBefore : Looking for %"LLU" ms get %"LLU" ms\n",pts/1000,_elem[index].image->Pts/1000);
            return _elem[index].image;
        }
    }
    aADM_warning("Cannot find image before %"LLU" ms in cache\n",pts/1000);
    return NULL;
}

/**
    \fn getByPts
    \brief returns the image that has exactly that PTS
*/
ADMImage *EditorCache::getByPts(uint64_t Pts)
{
    for(int i=readIndex;i<writeIndex;i++)
	{
        int index=i%_nbImage;
		if(_elem[index].image->Pts==Pts)
        {
            return _elem[index].image;
        }
    }
    return NULL;
}
/**
    \fn getLast
    \brief Return the most recent image from cache
*/
ADMImage *EditorCache::getLast(void)
{
    if(readIndex==writeIndex) return NULL;
    int index=(writeIndex-1)%_nbImage;
    return _elem[index].image;
}
// EOF
