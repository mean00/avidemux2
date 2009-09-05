#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "decodebufmodel.hpp"
#include <stdlib.h>

using std::deque;

/******************************************************************
 *	Remove entries from FIFO buffer list, if their DTS is less than
 *	actual SCR. These packet data have been already decoded and have
 *	been removed from the system target decoder's elementary stream
 *	buffer.
 *****************************************************************/

void DecodeBufModel::Cleaned(clockticks SCR)
{
    while ( bufstate.size() != 0 && bufstate.front().DTS < SCR)
    {
		bufstate.pop_front();
    }
}

/******************************************************************
 * Return the SCR when there will next be some change in the
 * buffer.
 * If the buffer is empty return a zero timestamp.
 *****************************************************************/

clockticks DecodeBufModel::NextChange()
{
	if( bufstate.size() == 0 )
		return static_cast<clockticks>(0);
	else
		return bufstate.front().DTS;
}


/******************************************************************
 *
 *	Remove all entries from FIFO buffer list.
 *
 *****************************************************************/

void DecodeBufModel::Flushed ()
{
	bufstate.clear();
}

/******************************************************************
	DecodeBufModel::Space

	returns free space in the buffer
******************************************************************/

unsigned int DecodeBufModel::Space ()
{
	unsigned int used_bytes = 0;
	for( std::deque<DecodeBufEntry>::iterator i = bufstate.begin(); 
		 i < bufstate.end();
		 ++i )
	{
		used_bytes += i->size;
    }

    return (buffer_size - used_bytes);

}

/******************************************************************
	Queue_Buffer

	adds entry into the buffer FIFO queue
******************************************************************/

void DecodeBufModel::Queued (unsigned int bytes, clockticks TS)
{
	DecodeBufEntry entry;
	entry.size = bytes;
	entry.DTS = TS;
	bufstate.push_back( entry );
}
