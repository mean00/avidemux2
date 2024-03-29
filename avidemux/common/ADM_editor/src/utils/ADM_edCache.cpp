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
#include "ADM_edCache.h"
#include "ADM_vidMisc.h"

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
EditorCache::EditorCache(uint32_t w, uint32_t h)
{
    readIndex = writeIndex = 0;
    commonWidth = w;
    commonHeight = h;
    for(uint32_t i=0; i < EDITOR_CACHE_MAX_SIZE; i++)
    {
        _elem[i].image = NULL;
        _elem[i].pts = ADM_NO_PTS;
        _elem[i].ref = -1;
    }
    _nbImage = 0;
    ADM_info("Video cache instance created at %p\n",this);
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
}

/**
    \fn createBuffers
*/
bool EditorCache::createBuffers(uint32_t size)
{
    int added = 0;
    while(_nbImage < size)
    {
        _elem[_nbImage++].image = new ADMImageDefault(commonWidth,commonHeight);
        added++;
    }
    ADM_info("Added %d video cache buffers, %u total.\n",added,_nbImage);
    return true;
}

/**
    \fn check
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
ADMImage *EditorCache::getFreeImage(int vid)
{
    int found=-1;
    check();
    // next!
    int r,w;
    r=readIndex%_nbImage;
    w=(writeIndex)%_nbImage;
    aprintf("Read: %" PRIu32" write :%" PRIu32"\n",readIndex,writeIndex);
    if(r==w && readIndex!=writeIndex) //full
    {
        readIndex++; // free older
        aprintf("Erasing read\n");
    }
    found=writeIndex%_nbImage;
    
    // Mark it as used
    if(found==-1) ADM_assert(0);
    _elem[found].pts=ADM_NO_PTS;
    writeIndex++;
    aprintf("Using free image at index %d\n",found);
    _elem[found].ref = vid;
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
        _elem[i].ref = -1;
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
            aprintf("Invalidate writeIndex %" PRIu32"\n",writeIndex);
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
bool EditorCache::validate(ADMImage *image)
{
    for(uint32_t i=0;i<_nbImage;i++)
    {
        if(_elem[i].image==image)
        {
            ADM_assert(_elem[i].pts==ADM_NO_PTS);
            _elem[i].pts=image->Pts;
            aprintf("validate Index %" PRIu32" with pts=%" PRIu64"ms\n",i,image->Pts);
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
    printf("ReadIndex:%" PRIu32", WriteIndex:%" PRIu32"\n",readIndex,writeIndex);
    for(int i=0;i<_nbImage;i++)
    {
      cacheElem *e=&(_elem[i]);
      switch(e->pts)
        {
            case ADM_NO_PTS:  printf("Not used %02d\n",i);break;
            default:
                printf("Edcache content[%02d]: ref: %d, PTS : %s %" PRIu64" ms\n",i,e->ref,
                                                                    ADM_us2plain(e->image->Pts),e->image->Pts/1000);
                break;
        }
    }
}
/**
    \fn findJustAfter
    \brief Find the image with the closest PTS just above pts.

*/
ADMImage    *EditorCache::getAfter(int vid, uint64_t pts)
{
    if(!writeIndex) return NULL;
    for(uint32_t i=readIndex;i<writeIndex-1;i++)
    {
        int index=i%_nbImage;
        ADM_assert(_elem[index].pts!=ADM_NO_PTS);
        if(_elem[index].pts == pts && _elem[index].ref == vid)
        {
            index++;
            index%=_nbImage;
            ADMImage *candidate=_elem[index].image;
            if(_elem[index].ref == vid)
            {
                if(candidate->Pts>pts)
                    return candidate;

                ADM_error("The next frame has a PTS <= the one we started from. We got timing %s\n", ADM_us2plain(candidate->Pts));
                ADM_error("We are looking for the one after %s\n",ADM_us2plain(pts));
                // in that case try to find the next valid one
                for(int k=i+2;k<writeIndex-1;k++)
                {
                    int otherIndex=k%_nbImage;
                    if(_elem[otherIndex].pts > pts && _elem[otherIndex].ref == vid)
                        return _elem[otherIndex].image;

                    ADM_error("Cannot find a second valid candidate after bad timing\n");
                    dump();
                }
            }
        }
    }
    aADM_warning("Cannot find image after %" PRIu64" ms in cache\n",pts/1000);
    return NULL;
}
/**
    \fn findJustAfter
    \brief Find the image with the closest PTS just above pts.

*/
ADMImage    *EditorCache::getBefore(int vid, uint64_t pts)
{
    for(int i=readIndex+1;i<writeIndex;i++)
    {
        int index=i%_nbImage;
        ADM_assert(_elem[index].pts!=ADM_NO_PTS);
        if(_elem[index].pts==pts && _elem[index].ref == vid)
        {
            index+=_nbImage-1;
            index%=_nbImage;
            printf("GetBefore : Looking for %" PRIu64" ms get %" PRIu64" ms\n",pts/1000,_elem[index].image->Pts/1000);
            if(_elem[index].ref == vid)
                return _elem[index].image;
        }
    }
    aADM_warning("Cannot find image before %" PRIu64" ms in cache\n",pts/1000);
    return NULL;
}

/**
    \fn getByPts
    \brief returns the image that has exactly that PTS
*/
ADMImage *EditorCache::getByPts(int vid, uint64_t Pts)
{
    for(int i=readIndex;i<writeIndex;i++)
    {
        int index=i%_nbImage;
        if(_elem[index].image->Pts==Pts && _elem[index].ref == vid)
            return _elem[index].image;
    }
    return NULL;
}
/**
    \fn getLast
    \brief Return the most recent image from cache
*/
ADMImage *EditorCache::getLast(int vid)
{
    if(readIndex==writeIndex) return NULL;
    int index=(writeIndex-1)%_nbImage;
    if(_elem[index].ref == vid)
        return _elem[index].image;
    return NULL;
}
// EOF
