#ifndef __AUNITBUFFER_H__
#define __AUNITBUFFER_H__

#include <deque>
#include "mjpeg_logging.h"
#include "aunit.hpp"

class AUStream
{
public:
	AUStream()  {}
	~AUStream() 
	{
		for( std::deque<AUnit *>::iterator i = buf.begin(); i < buf.end(); ++i )
			delete *i;
	}
	
	void Append( AUnit &rec )
	{
		if( buf.size() >= BUF_SIZE_SANITY )
			mjpeg_error_exit1( "INTERNAL ERROR: AU buffer overflow" );
		buf.push_back( new AUnit(rec) );
	}

	inline AUnit *Next( ) 
	{ 
		if( buf.size()==0 )
		{
			return 0;
		}
	    else
		{
			AUnit *res = buf.front();
			buf.pop_front();
			return res;
		}
	}

	inline void DropLast()
		{
			if( buf.empty() )
				mjpeg_error_exit1( "INTERNAL ERROR: droplast empty AU buffer" );
			buf.pop_back();
			
		}

	inline AUnit *Lookahead( unsigned int n)
	{
		return buf.size() <= n ? 0 : buf[n];
    }

	inline unsigned int MaxAULookahead() const { return buf.size(); }

private:
	static const unsigned int BUF_SIZE_SANITY = 1000;


	
	std::deque<AUnit *> buf;
};




#endif // __AUSTREAM_H__
