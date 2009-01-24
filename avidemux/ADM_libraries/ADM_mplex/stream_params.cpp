/*
 * params.hpp:  User specifiable parameters for various types of stream
 *
 *  The Check<stream>Params pseudo-constructors are constructed so that
 *  they will only construct legal combinations of parameters.
 *
 *  Copyright (C) 2002 Andrew Stevens <andrew.stevens@philips.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of version 2 of the GNU General Public License
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "stream_params.hpp"
#include "format_codes.h"

LpcmParams *LpcmParams::Default(unsigned int mux_format)
{
	return new LpcmParams(48000,2,16);
}

LpcmParams::LpcmParams( unsigned int samples,  
						unsigned int chans, 
						unsigned int bits ) :
	samples_per_sec( samples ),
	channels(chans),
	bits_per_sample(bits)
{
}

LpcmParams *LpcmParams::Checked(unsigned int samples,  
								unsigned int chans, 
								unsigned int bits )
{
    if( samples != 48000 && samples != 96000 )
        return 0;
    if( chans < 1 || chans > 7 )
        return 0;
    if( bits != 16 && bits != 20 && bits != 24 )
        return 0;

    return new LpcmParams(samples,chans,bits);
}

bool VideoParams::Force( unsigned int mux_format )
{
	unsigned int bufsiz;
	//
	// Handle formats that force the buffer size parameter to a
	// standard-conforming value
	//
	switch( mux_format )
	{
	case MPEG_FORMAT_SVCD :
		bufsiz = 230;
		break;
	case MPEG_FORMAT_VCD :
		bufsiz = 46;
		break;
	case MPEG_FORMAT_DVD :
	case MPEG_FORMAT_DVD_NAV :
		bufsiz = 232;
		break;
	default :
		return false;
	}
	decode_buffer_size = bufsiz;
	return true;
}

VideoParams *VideoParams::Checked( unsigned int bufsiz)
{
	if( bufsiz < 20 && bufsiz >= 4096 )	// In KB here...
		return 0;
	return new VideoParams(bufsiz);
}

VideoParams::VideoParams( unsigned int bufsiz ) :
	decode_buffer_size(bufsiz)
{
}

VideoParams *VideoParams::Default(unsigned int mux_format)
{
	unsigned int bufsiz;
	switch( mux_format )
	{
	case MPEG_FORMAT_MPEG2 :
	case MPEG_FORMAT_SVCD :
	case MPEG_FORMAT_SVCD_NSR :	
	case MPEG_FORMAT_SVCD_STILL :
		bufsiz = 230;
		break;
	case MPEG_FORMAT_DVD :		
	case MPEG_FORMAT_DVD_NAV :		
		bufsiz = 232;
		break;
	default :
		bufsiz = 46;
	}
	return new VideoParams(bufsiz);
}


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
