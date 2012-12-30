/*****************************************************************************
 * test.c:
 *****************************************************************************
 * Copyright (C) 2007  libmkv
 * $Id: $
 *
 * Authors: Nathan Caldwell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/******************
 * A _very_ simple matroska muxer. This is simply a test to make sure it even
 * works.
 ******************/

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#include "libmkv.h"


int main( int argc, char ** argv )
{
    mk_Writer * mkvfile;
    int c, option_index = 0;
    static char *output = NULL;
    static char *in_video = NULL;
    static char *in_audio = NULL;
    
    for( ;; )
    {
        static struct option long_options[] =
        {
            { "help",        no_argument,       NULL,    'h' },
            { "output",      required_argument, NULL,    'o' },
            { "audio",       required_argument, NULL,    'a' },
            { "video",       required_argument, NULL,    'v' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long( argc, argv,
                         "ho:a:v:",
                         long_options, &option_index );
        if( c < 0 )
        {
            break;
        }

        switch( c )
        {
            case 'o':
                printf("Got output filename: %s\n", optarg);
                output = strdup( optarg );
                break;
            case 'v':
                printf("Got input video track: %s\n", optarg);
                in_video = strdup( optarg );
                break;
            case 'a':
                printf("Got input audio track: %s\n", optarg);
                in_audio = strdup( optarg );
                break;
            default:
                fprintf( stderr, "Unknown option (%s)\n", argv[optind] );
            case 'h':
                show_help(argv);
                return 1;
        }
    }
/*
    mkvfile = mk_createWriter( output );

    mk_writeHeader(mkvfile, "test.c",
                   MKV_CODEC_X264,
                   codecPrivate, sizeof( codecPrivate ),
                   default_frame_duration,
                   timescale,
                   width, height,
                   d_width, d_height );

    mk_close( mkvfile );

    free( output );
    free( in_video );
    free( in_audio );
*/
    return 0;
}

show_help(char ** argv)
{
    fprintf(stderr, "%s [options] -o output_file.mkv video_track audio_track\n", argv[0]);
    fprintf(stderr, "\t-h, --help\t\tPrint this message, then quit.\n");
    fprintf(stderr, "\t-o, --output\t\tSpecify the output file.*\n");
    fprintf(stderr, "\t\t* - Required argument.\n");
}
