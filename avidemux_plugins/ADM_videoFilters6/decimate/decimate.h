#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_videoFilterCache.h"
#include "DIA_factory.h"

#include "dec.h"

#define PROGRESSIVE  0x00000001
#define MAGIC_NUMBER (0xdeadbeef)
#define IN_PATTERN   0x00000002

#define MAX_CYCLE_SIZE 25
#define MAX_BLOCKS 50

#define BLKSIZE 32

#define DrawString drawString

#define GETFRAME(g, fp) \
{ \
	int GETFRAMEf=g; \
	if (GETFRAMEf < 0) GETFRAMEf = 0; \
	(fp) = vidCache->getImage(GETFRAMEf); \
}
#if 1
    #define aprintf(...) {}
#else
    #define aprintf ADM_info
#endif
#define OutputDebugString aprintf

/**
    \class Telecide

*/
class  Decimate:public ADM_coreVideoFilterCached
{
protected:
        deciMate           configuration;
protected:
        int last_request, last_result;
        bool last_forced;
        double last_metric;
        double metrics[MAX_CYCLE_SIZE];
        double showmetrics[MAX_CYCLE_SIZE];
        int Dprev[MAX_CYCLE_SIZE];
        int Dcurr[MAX_CYCLE_SIZE];
        int Dnext[MAX_CYCLE_SIZE];
        int Dshow[MAX_CYCLE_SIZE];
        unsigned int hints[MAX_CYCLE_SIZE];
        bool hints_invalid;
        bool all_video_cycle;
        bool firsttime;
        int xblocks, yblocks;
        unsigned int *sum, div;
        
        
public:
                             Decimate(ADM_coreVideoFilter *previous,CONFcouple *conf);
                             ~Decimate();
        bool                 goToTime(uint64_t usSeek);
        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;           /// Start graphical user interface

protected:
        void   		DrawShow(ADMImage  *src, int useframe, bool forced, int dropframe,
		                              double metric, int inframe );
        void   		FindDuplicate(int frame, int *chosen, double *metric, bool *forced   );
    	void   		FindDuplicate2(int frame, int *chosen, bool *forced );
        void        updateInfo(void);
        uint32_t    computeDiff(ADMImage *current,ADMImage *previous);
        void        reset(void);
        bool        get0(uint32_t *fn,ADMImage *data);
        bool        get1(uint32_t *fn,ADMImage *data);
        bool        get2(uint32_t *fn,ADMImage *data);
        bool        get3(uint32_t *fn,ADMImage *data);
};
