//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef EDITOR_CACHE__
#define EDITOR_CACHE__
#include "ADM_image.h"
#define ADM_INVALID_CACHE 0xffff
#define ADM_IN_USE_CACHE  0xfffe

#define EDITOR_CACHE_MIN_SIZE 8
#define EDITOR_CACHE_MAX_SIZE 16

typedef struct cacheElem
{
    ADMImage *image;
    uint64_t pts;       // If set to ADM_NO_PTS -> unused entry
    int ref;
}cacheElem;
/**
    \class EditorCache
    \brief internal source-attached image cache
*/
class EditorCache
{
private :
    uint32_t        readIndex,writeIndex;
    uint32_t        commonWidth,commonHeight;
    cacheElem       _elem[EDITOR_CACHE_MAX_SIZE];
    uint32_t        _nbImage;

    void            check(void);

public:
                    EditorCache(uint32_t w, uint32_t h);
                    ~EditorCache(void);

    bool            createBuffers(uint32_t size);
    ADMImage        *getFreeImage(int vid);
    bool            validate(ADMImage *image);
    void            dump(void);
    void            flush(void);
    void            invalidate(ADMImage *image);
    ADMImage        *getByPts(int vid, uint64_t Pts);
    ADMImage        *getAfter(int vid, uint64_t Pts);
    ADMImage        *getBefore(int vid, uint64_t Pts);
    ADMImage        *getLast(int vid);
};
#endif
