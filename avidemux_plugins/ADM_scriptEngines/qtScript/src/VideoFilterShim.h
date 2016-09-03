#ifndef VideoFilterShim_h
#define VideoFilterShim_h

#include "ADM_coreVideoFilter.h"

namespace ADM_qtScript
{
    class VideoFilterShim : public ADM_coreVideoFilter
    {
    private:
        FilterInfo _dummyInfo;

    public:
        VideoFilterShim();
        bool getNextFrame(uint32_t *frameNumber, ADMImage *image);
        FilterInfo *getInfo(void);
        bool getCoupledConf(CONFcouple **couples);
        void setCoupledConf(CONFcouple *couples);
    };
}

#endif