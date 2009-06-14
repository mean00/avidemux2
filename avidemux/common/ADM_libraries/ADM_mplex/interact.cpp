
/*
 *  interact.cc:  Simple command-line front-end
 *
 *  Copyright (C) 2001 Andrew Stevens <andrew.stevens@philips.com>
 *
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


#include <config.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <mjpeg_logging.h>
#include <format_codes.h>

#include "interact.hpp"
#include "videostrm.hpp"
#include "audiostrm.hpp"
#ifdef ZALPHA
#include "zalphastrm.hpp"
#endif
#include "mplexconsts.hpp"
#include "aunit.hpp"


static const char *KindNames[] =
{
    "MPEG audio",
    "AC3 audio",
    "LPCM audio",
    "DTS audio",
    "MPEG video",
    "Z Alpha channel"
};

const char *JobStream::NameOfKind()
{
    return KindNames[kind];
}

Workarounds::Workarounds() 
{
}

MultiplexJob::MultiplexJob()
{
    verbose = 1;
    data_rate = 0;  /* 3486 = 174300B/sec would be right for VCD */
    video_offset = 0;
    audio_offset = 0;
    sector_size = 2048;
    VBR = false;
    mpeg = 1;
    mux_format = MPEG_FORMAT_MPEG1;
    multifile_segment = false;
    always_system_headers = false;
    packets_per_pack = 1;
    max_timeouts = 10;
    max_PTS = 0;
    max_segment_size = 0; // MB, default is unlimited (suitable for DVD)
    outfile_pattern = 0;
    packets_per_pack = 1;
    audio_tracks = 0;
    video_tracks = 0;
    lpcm_tracks = 0;
#ifdef ZALPHA
    z_alpha_tracks = 0;
#endif

}


MultiplexJob::~MultiplexJob()
{
	std::vector<JobStream *>::iterator i;
    for( i = streams.begin(); i < streams.end(); ++i )
        delete *i;
}


unsigned int MultiplexJob::NumberOfTracks( StreamKind kind )
{
    unsigned int count = 0;
	std::vector<JobStream *>::iterator i;
    for( i = streams.begin(); i < streams.end(); ++i )
        if( (*i)->kind == kind )
            ++count;
    return count;
    
}

void MultiplexJob::GetInputStreams( vector<JobStream *> &res, StreamKind kind )
{
    res.erase( res.begin(), res.end() );
	std::vector<JobStream *>::iterator i;
    for( i = streams.begin(); i < streams.end(); ++i )
        if( (*i)->kind == kind )
            res.push_back( *i );
}


// MEANX
static const char *kindToString(StreamKind kind)
{
#define KID(x)  x##_AUDIO:return #x;
    switch(kind)
    {
        case KID(MPEG)
       case  KID(AC3)
        case KID(LPCM)
        case KID(DTS)
        default: 
            printf("Kind %d\n",kind);
            assert(0);
    }
  return "Oops";
}

// /MEANS
void MultiplexJob::SetupInputStreams( std::vector< IBitStream *> &inputs )
{
    IBitStream *bs;
    IBitStreamUndo undo;
    unsigned int i;
    bool bad_file = false;
    
	for( i = 0; i < inputs.size(); ++i )
    {
        bs = inputs[i];
        // Remember the streams initial state...
        bs->PrepareUndo( undo );
// MEANX : Use the info we have to be sure of the stream type
// Else we cannot detect safly LPCM/PCM/DTS
        switch(bs->streamDesc.kind)
        {
            case MPEG_AUDIO:
            case AC3_AUDIO:
            case LPCM_AUDIO:
            case DTS_AUDIO:
                mjpeg_info ("File %s looks like an %s Audio stream (fq %u, channel %u).\n", bs->StreamName() , kindToString(bs->streamDesc.kind),bs->streamDesc.frequency,bs->streamDesc.channel);
                bs->UndoChanges( undo );
                streams.push_back( new JobStream( bs, bs->streamDesc.kind) );
                ++audio_tracks;
                if(bs->streamDesc.kind==LPCM_AUDIO) 
                    lpcm_param.push_back(LpcmParams::Checked(  bs->streamDesc.frequency,                                         
                                                   bs->streamDesc.channel,16));//++lpcm_tracks;
                continue;
            case MPEG_VIDEO:
                mjpeg_info ("File %s looks like an Video stream.", bs->StreamName() );
                bs->UndoChanges( undo );
                streams.push_back( new JobStream( bs,MPEG_VIDEO) );
                ++video_tracks;
                continue;
            default: assert(0);
            
        }
    }

#if 0 //MEANX
        if( LPCMStream::Probe( *bs ) )
        {
            mjpeg_info ("File %s looks like an LPCM Audio stream.",
                        bs->StreamName());
            bs->UndoChanges( undo );
            streams.push_back( new JobStream( bs,  LPCM_AUDIO) );
            ++audio_tracks;
            ++lpcm_tracks;
            continue;
        }

        bs->UndoChanges( undo );
        if( MPAStream::Probe( *bs ) )
        {
            mjpeg_info ("File %s looks like an MPEG Audio stream.", 
                        bs->StreamName() );
            bs->UndoChanges( undo );
            streams.push_back( new JobStream( bs, MPEG_AUDIO) );
            ++audio_tracks;
            continue;
        }

        bs->UndoChanges( undo );
        if( AC3Stream::Probe( *bs ) )
        {
            mjpeg_info ("File %s looks like an AC3 Audio stream.",
                        bs->StreamName());
            bs->UndoChanges( undo );
            streams.push_back( new JobStream( bs, AC3_AUDIO) );
            ++audio_tracks;
            continue;
        }

        bs->UndoChanges( undo );
        if( DTSStream::Probe( *bs ) )
        {
            mjpeg_info ("File %s looks like a dts Audio stream.",
                        bs->StreamName());
            bs->UndoChanges( undo);
            streams.push_back( new JobStream( bs, DTS_AUDIO) );
            ++audio_tracks;
            continue;
        }

        bs->UndoChanges( undo );
        if( VideoStream::Probe( *bs ) )
        {
            mjpeg_info ("File %s looks like an MPEG Video stream.",
                        bs->StreamName());
            bs->UndoChanges( undo );
            streams.push_back( new JobStream( bs, MPEG_VIDEO) );
            ++video_tracks;
            continue;
        }

        bs->UndoChanges( undo );
#ifdef ZALPHA
        if( ZAlphaStream::Probe( *bs ) )
        {
            mjpeg_info ("File %s looks like an Z/Alpha Video stream.",
                        bs->StreamName());
            bs->UndoChanges( undo );
            streams.push_back( new JobStream( bs, Z_ALPHA) );
            ++video_tracks;
            ++z_alpha_tracks;
            continue;
        }
#endif
        bad_file = true;
        mjpeg_error ("File %s unrecogniseable!", bs->StreamName());
        delete bs;
    }
#endif // MEANX IF 0 
    if( bad_file )
    {
        mjpeg_error_exit1( "Unrecogniseable file(s)... exiting.");
    }

	//
	// Where no parameters for streams have been specified
	// simply set the default values (these will depend on the format
	// we're muxing of course...)
	//

	for( i = video_param.size(); i < video_tracks; ++i )
	{
		video_param.push_back(VideoParams::Default( mux_format ));
	}
	for( i = lpcm_param.size(); i < lpcm_tracks; ++i )
	{
		lpcm_param.push_back(LpcmParams::Default(mux_format));
	}

	//
	// Set standard values if the selected profile implies this...
	//
	for( i = 0; i <video_tracks; ++i )
	{
		if( video_param[i]->Force(mux_format) )
		{
			mjpeg_info( "Video stream %d: profile %d selected - ignoring non-standard options!", i, mux_format );
		}
	}

	mjpeg_info( "Found %d audio streams and %d video streams",
                audio_tracks,
				video_tracks
        );
        
}


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
