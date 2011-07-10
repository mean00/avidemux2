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


/**
    \class Telecide

*/
class  Decimate:public ADM_coreVideoFilter
{
protected:
        deciMate           configuration;
protected:
        int 			num_frames_hi;
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
        int heightY, row_sizeY, pitchY;
        int heightUV, row_sizeUV, pitchUV;
        int pitch, row_size, height;
        int xblocks, yblocks;
        unsigned int *sum, div;
        bool debug, show;
        
        VideoCache	*vidCache;
public:
                            Decimate(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~Decimate();
        bool                goToTime(uint64_t usSeek);
        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;           /// Start graphical user interface

protected:
        void   		DrawShow(ADMImage  *src, int useframe, bool forced, int dropframe,
		                              double metric, int inframe );
        void   		FindDuplicate(int frame, int *chosen, double *metric, bool *forced   );
    	void   		FindDuplicate2(int frame, int *chosen, bool *forced );
    	void   		FindDuplicateYUY2(int frame, int *chosen, double *metric, bool *force);
    	void   		FindDuplicate2YUY2(int frame, int *chosen, bool *forced );
	
};
