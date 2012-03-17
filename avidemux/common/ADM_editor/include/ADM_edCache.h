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
typedef struct cacheElem
{
	ADMImage *image;
    uint64_t pts;       // If set to ADM_NO_PTS -> unused entry
}cacheElem;
/**
    \class EditorCache
    \brief internal source-attached image cache
*/
class EditorCache
{
	private :
			uint32_t     readIndex,writeIndex;
			cacheElem	 *_elem;
			uint32_t	_nbImage;
            void        check(void);
	public:
                        EditorCache(uint32_t size,uint32_t w, uint32_t h);
                        ~EditorCache(void);
			ADMImage	*getFreeImage(void);	
			bool		validate(ADMImage *image);
			void		dump(void);
            void        flush(void);
            void        invalidate(ADMImage *image);
            ADMImage    *getByPts(uint64_t Pts);            
            ADMImage    *getAfter(uint64_t Pts);
            ADMImage    *getBefore(uint64_t Pts);
            ADMImage    *getLast(void);
};
#endif
