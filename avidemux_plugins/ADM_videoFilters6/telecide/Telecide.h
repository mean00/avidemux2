/*
	Telecide plugin for Avisynth -- recovers original progressive
	frames from  telecined streams. The filter operates by matching
	fields and automatically adapts to phase/pattern changes.

	Copyright (C) 2003 Donald A. Graft

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_videoFilterCache.h"
#include "DIA_factory.h"
#include "telec.h"

#define DrawString drawString
#define BitBlt BitBlit

#undef DEBUG_PATTERN_GUIDANCE

#undef WINDOWED_MATCH

#define MAX_CYCLE 6
#define BLKSIZE 24
#define BLKSIZE_TIMES2 (2 * BLKSIZE)
#define GUIDE_NONE 0
#define GUIDE_32 1
#define GUIDE_22 2
#define GUIDE_32322 3
#define AHEAD 0
#define BEHIND 1
#define POST_NONE 0
#define POST_METRICS 1
#define POST_FULL 2
#define POST_FULL_MAP 3
#define POST_FULL_NOMATCH 4
#define POST_FULL_NOMATCH_MAP 5
#define CACHE_SIZE 100000
#define P 0
#define C 1
#define N 2
#define PBLOCK 3
#define CBLOCK 4

#define NO_BACK 0
#define BACK_ON_COMBED 1
#define ALWAYS_BACK 2

#define OutputDebugString(x) aprintf("%s\n",x)
typedef uint8_t* PVideoFrame ;


struct CACHE_ENTRY
{
	unsigned int frame;
	unsigned int metrics[5];
	unsigned int chosen;
};

struct PREDICTION
{
	unsigned int metric;
	unsigned int phase;
	unsigned int predicted;
	unsigned int predicted_metric;
};

#define GETFRAME(g, fp) { int GETFRAMEf; uint32_t len,flags;GETFRAMEf = (g); fp=NULL;\
    if (GETFRAMEf < 0) GETFRAMEf = 0; 	 fp=vidCache->getImage(GETFRAMEf); }



/**
    \class Telecide

*/
class  Telecide:public ADM_coreVideoFilter
{
protected:
        teleCide           configuration;
protected:
        bool tff;	
        uint32_t _lastFrame;	
#if 1
        int pitch, dpitch, pitchover2, pitchtimes4;
        int w, h, wover2, hover2, hplus1over2, hminus2;
#endif
        int xblocks, yblocks;
    #ifdef WINDOWED_MATCH
        unsigned int *matchc, *matchp, highest_matchc, highest_matchp;
    #endif
        unsigned int *sumc, *sump, highest_sumc, highest_sump;
        int vmetric;
        
        bool film, override, inpattern, found;
        int force;

        
        int chosen;
        unsigned int p, c, pblock, cblock, lowest, predicted, predicted_metric;
        unsigned int np, nc, npblock, ncblock;
        float mismatch;
        int  x, y;
        
        bool hard;
        char status[80];

        // Metrics cache.
        struct CACHE_ENTRY *cache;

        // Pattern guidance data.
        int cycle;
        struct PREDICTION pred[MAX_CYCLE+1];

        // For output message formatting.
        char buf[255];
        
        VideoCache	*vidCache;
public:
                            Telecide(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~Telecide();
        bool                goToTime(uint64_t usSeek);
        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;           /// Start graphical user interface

protected:
	
	void CalculateMetrics(int n, unsigned char *crp, unsigned char *crpU, unsigned char *crpV, 
				unsigned char *prp, unsigned char *prpU, unsigned char *prpV);
	void Show(ADMImage *dst, int frame);
	void Debug(int frame);


	void PutChosen(int frame, unsigned int chosen);
	

	void CacheInsert(int frame, unsigned int p, unsigned int pblock,
				unsigned int c, unsigned int cblock);
	
	bool CacheQuery(int frame, unsigned int *p, unsigned int *pblock,
				unsigned int *c, unsigned int *cblock);	

	bool PredictHardYUY2(int frame, unsigned int *predicted, unsigned int *predicted_metric) ;
	
	struct PREDICTION *PredictSoftYUY2(int frame);

	void WriteHints(unsigned char *dst, bool film, bool inpattern);
};
