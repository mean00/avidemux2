/*****************************************************************************
 * matroska.h:
 *****************************************************************************
 * Copyright (C) 2005 x264 project
 *
 * Authors: Mike Matsnev
 *          Nathan Caldwell
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
#ifndef _MATROSKA_H
#define _MATROSKA_H 1

#include "md5.h"
#include "ebml.h"

#define CLSIZE    1048576
#define CHECK(x)  do { if ((x) < 0) return -1; } while (0)

/* Matroska version supported */
#define MATROSKA_VERSION 2

/* Copied from ffmpeg */
/*
 * Matroska element IDs. max. 32-bit.
 */

/* toplevel segment */
#define MATROSKA_ID_SEGMENT         0x18538067	/* sub-elements */

/* matroska top-level master IDs */
#define MATROSKA_ID_SEEKHEAD		0x114D9B74	/* sub-elements */
#define MATROSKA_ID_INFO			0x1549A966	/* sub-elements */
#define MATROSKA_ID_CLUSTER			0x1F43B675	/* sub-elements */
#define MATROSKA_ID_TRACKS			0x1654AE6B	/* sub-elements */
#define MATROSKA_ID_CUES			0x1C53BB6B	/* sub-elements */
#define MATROSKA_ID_ATTACHMENTS		0x1941A469	/* sub-elements */
#define MATROSKA_ID_CHAPTERS		0x1043A770	/* sub-elements */
#define MATROSKA_ID_TAGS			0x1254C367	/* sub-elements */

/* IDs in the info master */
#define MATROSKA_ID_TIMECODESCALE	0x2AD7B1	/* u-integer */
#define MATROSKA_ID_DURATION		0x4489		/* float */
#define MATROSKA_ID_TITLE			0x7BA9		/* UTF-8 */
#define MATROSKA_ID_WRITINGAPP		0x5741		/* UTF-8 */
#define MATROSKA_ID_MUXINGAPP		0x4D80		/* UTF-8 */
#define MATROSKA_ID_DATEUTC			0x4461		/* date */
#define MATROSKA_ID_SEGMENTUID		0x73A4		/* binary */

/* ID in the tracks master */
#define MATROSKA_ID_TRACKENTRY		0xAE	/* sub-elements */

/* IDs in the trackentry master */
#define MATROSKA_ID_TRACKNUMBER			 0xD7		/* u-integer */
#define MATROSKA_ID_TRACKUID			 0x73C5		/* u-integer */
#define MATROSKA_ID_TRACKTYPE			 0x83		/* u-integer */
#define MATROSKA_ID_TRACKAUDIO			 0xE1		/* sub-elements */
#define MATROSKA_ID_TRACKVIDEO			 0xE0		/* sub-elements */
#define MATROSKA_ID_CODECID				 0x86		/* string */
#define MATROSKA_ID_CODECPRIVATE		 0x63A2		/* binary */
#define MATROSKA_ID_CODECNAME			 0x258688	/* UTF-8 */
#define MATROSKA_ID_CODECINFOURL		 0x3B4040	/* string */
#define MATROSKA_ID_CODECDOWNLOADURL	 0x26B240	/* string */
#define MATROSKA_ID_TRACKNAME			 0x536E		/* UTF-8 */
#define MATROSKA_ID_TRACKLANGUAGE		 0x22B59C	/* string */
#define MATROSKA_ID_TRACKFLAGENABLED	 0xB9		/* u-integer (1 bit) */
#define MATROSKA_ID_TRACKFLAGDEFAULT	 0x88		/* u-integer (1 bit) */
#define MATROSKA_ID_TRACKFLAGLACING		 0x9C		/* u-integer (1 bit) */
#define MATROSKA_ID_TRACKFLAGFORCED		 0x55AA		/* u-integer (1 bit) */
#define MATROSKA_ID_TRACKMINCACHE		 0x6DE7		/* u-integer */
#define MATROSKA_ID_TRACKMAXCACHE		 0x6DF8		/* u-integer */
#define MATROSKA_ID_TRACKDEFAULTDURATION 0x23E383	/* u-integer */

/* IDs in the trackvideo master */
#define MATROSKA_ID_VIDEODISPLAYWIDTH	 0x54B0		/* u-integer */
#define MATROSKA_ID_VIDEODISPLAYHEIGHT	 0x54BA		/* u-integer */
#define MATROSKA_ID_VIDEODISPLAYUNIT	 0x54B2		/* u-integer */
#define MATROSKA_ID_VIDEOPIXELWIDTH		 0xB0		/* u-integer */
#define MATROSKA_ID_VIDEOPIXELHEIGHT	 0xBA		/* u-integer */
#define MATROSKA_ID_VIDEOFLAGINTERLACED	 0x9A		/* u-integer (1 bit) */
#define MATROSKA_ID_VIDEOSTEREOMODE		 0x53B9		/* u-integer */
#define MATROSKA_ID_VIDEOASPECTRATIOTYPE 0x54B3		/* u-integer */
#define MATROSKA_ID_VIDEOCOLORSPACE		 0x2EB524	/* binary */
#define MATROSKA_ID_VIDEOPIXELCROPBOTTOM 0x54AA		/* u-integer */
#define MATROSKA_ID_VIDEOPIXELCROPTOP	 0x54BB		/* u-integer */
#define MATROSKA_ID_VIDEOPIXELCROPLEFT	 0x54CC		/* u-integer */
#define MATROSKA_ID_VIDEOPIXELCROPRIGHT	 0x54DD		/* u-integer */

/* IDs in the trackaudio master */
#define MATROSKA_ID_AUDIOSAMPLINGFREQ	 0xB5	/* float */
#define MATROSKA_ID_AUDIOOUTSAMPLINGFREQ 0x78B5	/* float */
#define MATROSKA_ID_AUDIOBITDEPTH		 0x6264	/* u-integer */
#define MATROSKA_ID_AUDIOCHANNELS		 0x9F	/* u-integer */

/* ID in the cues master */
#define MATROSKA_ID_CUEPOINT		0xBB	/* sub-elements */

/* IDs in the pointentry master */
#define MATROSKA_ID_CUETIME			  0xB3	/* u-integer */
#define MATROSKA_ID_CUETRACKPOSITIONS 0xB7	/* sub-elements */

/* IDs in the cuetrackposition master */
#define MATROSKA_ID_CUETRACK		   0xF7		/* u-integer */
#define MATROSKA_ID_CUECLUSTERPOSITION 0xF1		/* u-integer */
#define MATROSKA_ID_CUEBLOCKNUMBER	   0x5378	/* u-integer */

/* IDs in the seekhead master */
#define MATROSKA_ID_SEEKENTRY		0x4DBB	/* sub-elements */

/* IDs in the seekpoint master */
#define MATROSKA_ID_SEEKID			0x53AB	/* binary */
#define MATROSKA_ID_SEEKPOSITION	0x53AC	/* u-integer */

/* IDs in the cluster master */
#define MATROSKA_ID_CLUSTERTIMECODE	0xE7	/* u-integer */
#define MATROSKA_ID_BLOCKGROUP		0xA0	/* sub-elements */
#define MATROSKA_ID_SIMPLEBLOCK		0xA3	/* binary */

/* IDs in the blockgroup master */
#define MATROSKA_ID_BLOCK			0xA1	/* binary */
#define MATROSKA_ID_BLOCKDURATION	0x9B	/* u-integer */
#define MATROSKA_ID_REFERENCEBLOCK	0xFB	/* s-integer */

/* IDs in the attachments master */
#define MATROSKA_ID_ATTACHEDFILE	0x61A7	/* sub-elements */
#define MATROSKA_ID_FILEDESCRIPTION	0x467E	/* UTF-8 */
#define MATROSKA_ID_FILENAME		0x466E	/* UTF-8 */
#define MATROSKA_ID_FILEMIMETYPE	0x4660	/* string */
#define MATROSKA_ID_FILEDATA		0x465C	/* binary */
#define MATROSKA_ID_FILEUID			0x46AE	/* u-integer */
/* Above copied from ffmpeg */

/* IDs in the tags master */
#define MATROSKA_ID_TAG				0x7373	/* sub-elements */

/* IDs in the tag master */
#define MATROSKA_ID_TARGETS			0x63C0	/* sub-elements */
#define MATROSKA_ID_SIMPLETAG		0x67C8	/* sub-elements */

/* IDs in the targets master */
#define MATROSKA_ID_TARGETTYPEVALUE		0x68CA	/* u-integer */
#define MATROSKA_ID_TARGETTYPE			0x63CA	/* string */
#define MATROSKA_ID_TARGETTRACKUID		0x63C5	/* u-integer */
#define MATROSKA_ID_TARGETEDITIONUID	0x63C9	/* u-integer */
#define MATROSKA_ID_TARGETCHAPTERUID	0x63C4	/* u-integer */
#define MATROSKA_ID_TARGETATTACHMENTUID	0x63C6	/* u-integer */

/* IDs in the simple tag master */
#define MATROSKA_ID_TAGNAME			0x45A3	/* UTF-8 */
#define MATROSKA_ID_TAGLANGUAGE		0x447A	/* string */
#define MATROSKA_ID_TAGDEFAULT		0x4484	/* u-integer (1-bit)) */
#define MATROSKA_ID_TAGSTRING		0x4487	/* UTF-8 */
#define MATROSKA_ID_TAGBINARY		0x4485	/* binary */

/* IDs in the chapters master */
#define MATROSKA_ID_EDITIONENTRY	0x45B9	/* sub-elements */

/* IDs in the edition entry master */
#define MATROSKA_ID_EDITIONUID		   0x45BC	/* u-integer */
#define MATROSKA_ID_EDITIONFLAGHIDDEN  0x45BD	/* u-integer (1 bit) */
#define MATROSKA_ID_EDITIONFLAGDEFAULT 0x45DB	/* u-integer (1 bit) */
#define MATROSKA_ID_EDITIONFLAGORDERED 0x45DD	/* u-integer (1 bit) */
#define MATROSKA_ID_CHAPTERATOM		   0xB6		/* sub-elements */

/* IDs in the chapter atom master */
#define MATROSKA_ID_CHAPTERUID				 0x73C4	/* u-integer */
#define MATROSKA_ID_CHAPTERTIMESTART		 0x91	/* u-integer */
#define MATROSKA_ID_CHAPTERTIMEEND			 0x92	/* u-integer */
#define MATROSKA_ID_CHAPTERFLAGHIDDEN		 0x98	/* u-integer (1 bit) */
#define MATROSKA_ID_CHAPTERFLAGENABLED		 0x4598	/* u-integer (1 bit) */
#define MATROSKA_ID_CHAPTERSEGMENTUID		 0x6E67	/* binary */
#define MATROSKA_ID_CHAPTERSEGMENTEDITIONUID 0x6EBC	/* binary */
#define MATROSKA_ID_CHAPTERPHYSICALEQUIV	 0x63C3	/* u-integer */
#define MATROSKA_ID_CHAPTERTRACK			 0x8F	/* sub-elements */
#define MATROSKA_ID_CHAPTERDISPLAY			 0x80	/* sub-elements */
#define MATROSKA_ID_CHAPPROCESS				 0x6944	/* sub-elements */

/* IDs in the chapter track master */
#define MATROSKA_ID_CHAPTERTRACKNUMBER 0x89	/* u-integer */

/* IDs in the chapter display master */
#define MATROSKA_ID_CHAPTERSTRING	0x85	/* UTF-8 */
#define MATROSKA_ID_CHAPTERLANGUAGE	0x437C	/* string */
#define MATROSKA_ID_CHAPTERCOUNTRY	0x437E	/* string */

/* IDs in the chap process master */
#define MATROSKA_ID_CHAPPROCESSCODECID 0x6955	/* u-integer */
#define MATROSKA_ID_CHAPPROCESSPRIVATE 0x450D	/* binary */
#define MATROSKA_ID_CHAPPROCESSCOMMAND 0x6911	/* sub-elements */

/* IDs in the chap proccess command master */
#define MATROSKA_ID_CHAPPROCESSTIME	0x6922	/* u-integer */
#define MATROSKA_ID_CHAPPROCESSDATA	0x6933	/* binary */

typedef struct mk_Seek_s mk_Seek;
typedef struct mk_Chapter_s mk_Chapter;

struct mk_Writer_s {
	FILE *fp;
	uint64_t f_pos;
	uint64_t f_eof;

	int64_t duration_ptr;
	int64_t seekhead_ptr;
	int64_t segment_ptr;
	int64_t segmentuid_ptr;

	mk_Context *root;
	mk_Context *freelist;
	mk_Context *actlist;
	mk_Context *chapters;
	mk_Context *edition_entry;
	mk_Context *tags;
	mk_Context *attachments;
	mk_Context *tag;
	mk_Context *tracks;
	mk_Context *cues;

	uint64_t def_duration;
	uint64_t timescale;

	uint8_t wrote_header;

	uint8_t num_tracks;
	uint8_t alloc_tracks;
	mk_Track **tracks_arr;

	struct {
		int64_t segmentinfo;
		int64_t seekhead;
		int64_t tracks;
		int64_t cues;
		int64_t chapters;
		int64_t attachments;
		int64_t tags;
	} seek_data;

	struct {
		mk_Context *context;
		mk_Context *seekhead;
		uint64_t block_count;
		uint64_t count;
		uint64_t pointer;
		int64_t tc_scaled;
	} cluster;

	md5_context segment_md5;
	uint8_t vlc_compat;
};

struct mk_Track_s {
	uint8_t track_id;

	int64_t prev_frame_tc_scaled;
	int64_t max_frame_tc;
	uint8_t in_frame;
	uint64_t default_duration;
	mk_TrackType track_type;
	int64_t prev_cue_pos;
	uint8_t *private_data;
	unsigned private_data_size;
	int64_t private_data_ptr;

	struct {
		mk_Context *data;
		int64_t timecode;
		uint8_t keyframe;
		mk_LacingType lacing;
		uint8_t lacing_num_frames;
		uint64_t *lacing_sizes;
		uint64_t      duration;
	} frame;
};

int mk_writeSeek(mk_Writer *w, mk_Context *c, unsigned seek_id,
				 uint64_t seek_pos);
int mk_writeSeekHead(mk_Writer *w, int64_t *pointer);
int mk_writeTracks(mk_Writer *w, mk_Context *tracks);
int mk_writeChapters(mk_Writer *w);
int mk_writeTags(mk_Writer *w);
int mk_writeAttachments(mk_Writer *w);
int mk_seekFile(mk_Writer *w, uint64_t pos);

#endif
