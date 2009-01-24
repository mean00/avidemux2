#ifndef __AUNIT_H__
#define __AUNIT_H__

#include "mjpeg_types.h"
#include "bits.hpp"

typedef int64_t clockticks;		// This value *must* be signed
                                // because we frequently compute *offsets*

class AUnit
{
public:
	AUnit() : length(0), PTS(0), DTS(0) {}
	//
	// How many payload bytes muxing AU will require.  Eventually will be more
	// complex for input streams where AU are no contiguous
	//
	inline unsigned int PayloadSize() const { return length; }
	void markempty() { length = 0; }
	bitcount_t start;
	unsigned int length;
    clockticks PTS;
    int        dorder;
	//
	// Remainder Used only for video AU's... 
	//
    clockticks DTS;
    int		   porder;
    unsigned int type;
	bool	   seq_header;
	bool	   end_seq;

};


#endif // __AUNIT_H__
