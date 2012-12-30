/*****************************************************************************
 * matroska.c:
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
#include "config.h"
#include "libmkv.h"
#include "matroska.h"
#include "md5.h"

#include <sys/time.h>

#define RESERVED_SEEKHEAD 0x100
/* 256 bytes should be enough room for our Seek entries. */
#define RESERVED_CHAPTERS 0x1000
/* 4096 bytes, hopefully enough for Chapters. */

int mk_seekFile(mk_Writer *w, uint64_t pos)
{
	if (fseek(w->fp, pos, SEEK_SET))
		return -1;

	w->f_pos = pos;

	if (pos > w->f_eof)
		w->f_eof = pos;

	return 0;
}

char *mk_laceXiph(uint64_t *sizes, uint8_t num_frames,
				  uint64_t *output_size)
{
	unsigned i, j;
	uint64_t offset = 0;
	uint64_t alloc_size = num_frames * 6;	/* Complete guess. It gets realloc'd
											 * below if we need more space, though.
											 */
	char *laced = calloc(alloc_size, sizeof(*laced));
	if (laced == NULL)
		return NULL;

	laced[offset++] = num_frames;
	for (i = 0; i < num_frames; i++) {
		for (j = sizes[i]; j >= 255; j -= 255) {
			laced[offset++] = 255;
			if (offset + 1 >= alloc_size) {
				int avg_sz = offset / (i - 1);	/* Compute approximate average bytes/frame */
				alloc_size += avg_sz * (num_frames - i);
								/* Sum average so far and number of frames left
								 * with alloc'd size
								 */
				if ((laced = realloc(laced, alloc_size)) == NULL)
					return NULL;
			}
		}
		laced[offset++] = j;
	}

	if (output_size != NULL)
		*output_size = offset;

	return laced;
}

mk_Writer *mk_createWriter(const char *filename, int64_t timescale,
						   uint8_t vlc_compat)
{
	mk_Writer *w = calloc(1, sizeof(*w));
	struct timeval tv;

	if (w == NULL)
		return NULL;

	w->root = mk_createContext(w, NULL, 0);
	if (w->root == NULL) {
		free(w);
		return NULL;
	}

	/* Cues */
	if ((w->cues = mk_createContext(w, w->root, MATROSKA_ID_CUES)) == NULL)
	{
		mk_destroyContexts(w);
		free(w);
		return NULL;
	}

	/* Clusters SeekHead */
	if ((w->cluster.seekhead = mk_createContext(w, w->root, MATROSKA_ID_SEEKHEAD)) == NULL)
	{
		mk_destroyContexts(w);
		free(w);
		return NULL;
	}

	w->fp = fopen(filename, "wb");
	if (w->fp == NULL) {
		mk_destroyContexts(w);
		free(w);
		return NULL;
	}

	gettimeofday( &tv, NULL );
	srandom(tv.tv_sec ^ tv.tv_usec);

	w->timescale = timescale;
	w->vlc_compat = vlc_compat;

	return w;
}

int mk_writeHeader(mk_Writer *w, const char *writingApp)
{
	mk_Context *c;
	mk_Track *tk;
	int i;
	int64_t offset = 0;

	if (w->wrote_header)
		return -1;

	md5_starts(&w->segment_md5);	/* Initalize MD5 */

	CHECK(mk_writeEbmlHeader(w, "matroska", MATROSKA_VERSION, MATROSKA_VERSION));

	/* Segment */
	if ((c = mk_createContext(w, w->root, MATROSKA_ID_SEGMENT)) == NULL)
		return -1;
	CHECK(mk_flushContextID(c));
	w->segment_ptr = c->d_cur;
	CHECK(mk_closeContext(c, &w->segment_ptr));

	if (w->vlc_compat) {
		CHECK(mk_writeVoid(w->root, RESERVED_SEEKHEAD));	/* Reserved space for SeekHead */
		CHECK(mk_writeVoid(w->root, RESERVED_CHAPTERS));	/* Reserved space for Chapters */
	} else {
		w->seek_data.seekhead = 0x80000000;
		CHECK(mk_writeSeekHead(w, &w->seekhead_ptr));
		w->seek_data.seekhead = 0;
	}

	if ((c = mk_createContext(w, w->root, MATROSKA_ID_INFO)) == NULL)	/* SegmentInfo */
		return -1;
	w->seek_data.segmentinfo = w->root->d_cur - w->segment_ptr;
	/* Reserve space for a SegmentUID (16 bytes + 1 byte longer EBML ID), to be written it later. */
	CHECK(mk_writeVoid(c, 16 + 1));
	CHECK(mk_writeStr(c, MATROSKA_ID_MUXINGAPP, PACKAGE_STRING));	/* MuxingApp */
	CHECK(mk_writeStr(c, MATROSKA_ID_WRITINGAPP, writingApp));	/* WritingApp */
	CHECK(mk_writeUInt(c, MATROSKA_ID_TIMECODESCALE, w->timescale));	/* TimecodeScale */
	CHECK(mk_writeFloat(c, MATROSKA_ID_DURATION, 0));	/* Duration */
	w->duration_ptr = c->d_cur - 4;
	CHECK(mk_closeContext(c, &offset));
	w->duration_ptr += offset;
	w->segmentuid_ptr = offset;

	w->seek_data.tracks = w->root->d_cur - w->segment_ptr;

	if (w->tracks) {
		offset = 0;
		CHECK(mk_closeContext(w->tracks, &offset));
		for (i = 0; i < w->num_tracks; i++) {
			tk = w->tracks_arr[i];
			if (tk->private_data_size)
				tk->private_data_ptr += offset;
		}
	}

	CHECK(mk_flushContextData(w->root));

	w->wrote_header = 1;
	w->def_duration = w->tracks_arr[0]->default_duration;
	return 0;
}

static int mk_closeCluster(mk_Writer *w)
{
	if (w->cluster.context == NULL)
		return 0;
	w->cluster.count++;
	CHECK(mk_closeContext(w->cluster.context, 0));
	w->cluster.context = NULL;
	CHECK(mk_flushContextData(w->root));

	return 0;
}

int mk_flushFrame(mk_Writer *w, mk_Track *track)
{
	mk_Context *c, *tp;
	int64_t delta, ref = 0;
	unsigned fsize, bgsize;
	uint8_t flags, c_delta[2];
	int i;
	char *laced = NULL;
	uint64_t length = 0;
	uint64_t block_duration = 0;

	if (!track->in_frame)
		return 0;

	delta = track->frame.timecode / w->timescale - w->cluster.tc_scaled;
	block_duration = track->frame.duration / w->timescale;

	/* NOTE: If we switch rapidly back-and-forth between tracks with
	 * drastically different timecodes this causes a new cluster to
	 * be written each time a switch is made. This causes unnecessary
	 * overhead. The calling application is assumed to have interleaved
	 * track samples based on timestamp.
	 */

	/* Soft limit: If the frame is a video keyframe and we are not closer than
	 * 2 seconds to the last cluster, start a new cluster.
	 */
	if (track->track_type == MK_TRACK_VIDEO && track->frame.keyframe && delta > 2000ll)
		CHECK(mk_closeCluster(w));

	/* Hard limit: if the current cluster is greater than 20 seconds,
	 * start a new cluster
	 */
	if (delta > 20000ll || delta < -20000ll)
		CHECK(mk_closeCluster(w));

	if (w->cluster.context == NULL) {
		w->cluster.tc_scaled = track->frame.timecode / w->timescale;
		/* Cluster */
		w->cluster.context = mk_createContext(w, w->root, MATROSKA_ID_CLUSTER);
		if (w->cluster.context == NULL)
			return -1;

		w->cluster.pointer = w->f_pos - w->segment_ptr;

		/* Cluster SeekEntry */
		CHECK(mk_writeSeek(w, w->cluster.seekhead, MATROSKA_ID_CLUSTER,
						   w->cluster.pointer));

		/* Cluster Timecode */
		CHECK(mk_writeUInt(w->cluster.context, MATROSKA_ID_CLUSTERTIMECODE, w->cluster.tc_scaled));

		delta = 0;
		w->cluster.block_count = 0;
	}

	/* Calculate the encoded lacing sizes. */
	switch (track->frame.lacing) {
		case MK_LACING_XIPH:
			laced =	mk_laceXiph(track->frame.lacing_sizes,
								track->frame.lacing_num_frames, &length);
			break;
		case MK_LACING_EBML:
		{
			uint64_t u_size = 0;
			/* Add one below for the frame count. */
			length += mk_ebmlSizeSize(track->frame.lacing_sizes[0]) + 1;
			for (i = 1; i < track->frame.lacing_num_frames; i++) {
				u_size = llabs(track->frame.lacing_sizes[i] -
							   track->frame.lacing_sizes[i - 1]);
				/* Shift by one so we get the right size for a signed number. */
				length += mk_ebmlSizeSize((u_size) << 1);
			}
			break;
		}
		case MK_LACING_FIXED:
		{
			laced = calloc(1, sizeof(*laced));
			laced[0] = track->frame.lacing_num_frames;
			++length;
			break;
		}
		default:
			break;
	}

	fsize = track->frame.data ? track->frame.data->d_cur : 0;
	bgsize = fsize + 4 + mk_ebmlSizeSize(fsize + 4 + length) + 1 + length;
	if (!track->frame.keyframe) {
		ref = track->prev_frame_tc_scaled - w->cluster.tc_scaled - delta;
		bgsize += 1 + 1 + mk_ebmlSIntSize(ref);
	}
	if (block_duration > 0)	/* BlockDuration */
	{
		bgsize += 1 + 1 + mk_ebmlUIntSize(block_duration);
	}

	CHECK(mk_writeID(w->cluster.context, MATROSKA_ID_BLOCKGROUP));	/* BlockGroup */
	CHECK(mk_writeSize(w->cluster.context, bgsize));
	CHECK(mk_writeID(w->cluster.context, MATROSKA_ID_BLOCK));	/* Block */
	CHECK(mk_writeSize(w->cluster.context, fsize + 4 + length));	/* BlockSize */
	CHECK(mk_writeSize(w->cluster.context, track->track_id));	/* track number */

	w->cluster.block_count++;

	c_delta[0] = delta >> 8;
	c_delta[1] = delta;
	/* Timecode relative to Cluster. */
	CHECK(mk_appendContextData(w->cluster.context, c_delta, 2));

/*	flags = ( track->frame.keyframe << 8 ) | track->frame.lacing; */
	flags = track->frame.lacing << 1;	/* Flags: Bit 5-6 describe what type of lacing to use. */
	CHECK(mk_appendContextData(w->cluster.context, &flags, 1));
	if (track->frame.lacing) {
		if (track->frame.lacing == MK_LACING_EBML) {
			/* Number of frames in lace - 1 */
			CHECK(mk_appendContextData(w->cluster.context, &track->frame.lacing_num_frames, 1));
			/* Size of 1st frame. */
			CHECK(mk_writeSize(w->cluster.context, track->frame.lacing_sizes[0]));
			for (i = 1; i < track->frame.lacing_num_frames; i++) {
				/* Size difference between previous size and this size. */
				CHECK(mk_writeSSize(w->cluster.context, track->frame.lacing_sizes[i] - track->frame.lacing_sizes[i - 1]));
			}
		} else if (length > 0 && laced != NULL) {
			CHECK(mk_appendContextData(w->cluster.context, laced, length));
			free(laced);
			laced = NULL;
		}
	}

	if (track->frame.data) {
		CHECK(mk_appendContextData(w->cluster.context, track->frame.data->data,
								   track->frame.data->d_cur));
		track->frame.data->d_cur = 0;
	}
	if (!track->frame.keyframe)		/* ReferenceBlock */
		CHECK(mk_writeSInt(w->cluster.context, MATROSKA_ID_REFERENCEBLOCK, ref));

	if (block_duration > 0)	/* BlockDuration */
		CHECK(mk_writeUInt(w->cluster.context, 0x9b, block_duration));

	/* This may get a little out of hand, but it seems sane enough for now. */
	if (track->frame.keyframe && (track->track_type == MK_TRACK_VIDEO)) {
/*	if (track->frame.keyframe && (track->track_type & MK_TRACK_VIDEO) && ((track->prev_cue_pos + 3*CLSIZE) <= w->f_pos || track->frame.timecode == 0)) { */
		/* CuePoint */
		if ((c = mk_createContext(w, w->cues, MATROSKA_ID_CUEPOINT)) == NULL)
			return -1;
		/* CueTime */
		CHECK(mk_writeUInt(c, MATROSKA_ID_CUETIME, (track->frame.timecode / w->timescale)));

		/* CueTrackPositions */
		if ((tp = mk_createContext(w, c, MATROSKA_ID_CUETRACKPOSITIONS)) == NULL)
			return -1;
		/* CueTrack */
		CHECK(mk_writeUInt(tp, MATROSKA_ID_CUETRACK, track->track_id));
		/* CueClusterPosition */
		CHECK(mk_writeUInt(tp, MATROSKA_ID_CUECLUSTERPOSITION, w->cluster.pointer));
		/* CueBlockNumber */
/*		CHECK(mk_writeUInt(c, MATROSKA_ID_CUEBLOCKNUMBER, w->cluster.block_count)); */
		CHECK(mk_closeContext(tp, 0));
		CHECK(mk_closeContext(c, 0));
		track->prev_cue_pos = w->f_pos;
	}

	track->in_frame = 0;
	track->prev_frame_tc_scaled = w->cluster.tc_scaled + delta;

	return 0;
}

int mk_startFrame(mk_Writer *w, mk_Track *track)
{
	if (mk_flushFrame(w, track) < 0)
		return -1;

	track->in_frame = 1;
	track->frame.keyframe = 0;
	track->frame.lacing = MK_LACING_NONE;
	track->frame.lacing_num_frames = 0;
	track->frame.lacing_sizes = NULL;

	return 0;
}

int mk_setFrameFlags(mk_Writer *w, mk_Track *track, int64_t timestamp,
					 unsigned keyframe, uint64_t duration)
{
	if (!track->in_frame)
		return -1;

	track->frame.timecode = timestamp;
	track->frame.keyframe = keyframe != 0;

	if (track->max_frame_tc < timestamp)
		track->max_frame_tc = timestamp;

	if (duration > 0)
		track->frame.duration = duration;

	return 0;
}

int mk_setFrameLacing(mk_Writer *w, mk_Track *track,
					  mk_LacingType lacing, uint8_t num_frames,
					  uint64_t sizes[])
{
	if (!track->in_frame)
		return -1;
	track->frame.lacing_sizes = calloc(num_frames, sizeof(*sizes));

	track->frame.lacing = lacing;
	track->frame.lacing_num_frames = num_frames;
	memcpy(track->frame.lacing_sizes, sizes,
		   num_frames * sizeof(*sizes));

	return 0;
}

int mk_addFrameData(mk_Writer *w, mk_Track *track, const void *data,
					unsigned size)
{
	if (!track->in_frame)
		return -1;

	if (track->frame.data == NULL) {
		if ((track->frame.data = mk_createContext(w, NULL, 0)) == NULL)
			return -1;
	}

	md5_update(&w->segment_md5, (unsigned char *) data, size);

	return mk_appendContextData(track->frame.data, data, size);
}

int mk_writeSeek(mk_Writer *w, mk_Context *c, unsigned seek_id,
				 uint64_t seek_pos)
{
	mk_Context *s;

	if ((s = mk_createContext(w, c, MATROSKA_ID_SEEKENTRY)) == NULL)	/* Seek */
		return -1;
	CHECK(mk_writeUInt(s, MATROSKA_ID_SEEKID, seek_id));	/* SeekID */
	CHECK(mk_writeUInt(s, MATROSKA_ID_SEEKPOSITION, seek_pos));	/* SeekPosition */
	CHECK(mk_closeContext(s, 0));

	return 0;
}

/* The offset of the SeekHead is returned in pointer. */
int mk_writeSeekHead(mk_Writer *w, int64_t *pointer)
{
	mk_Context *c;
	int64_t seekhead_ptr;

	/* SeekHead */
	if ((c = mk_createContext(w, w->root, MATROSKA_ID_SEEKHEAD)) == NULL)
		return -1;
	if (pointer != NULL)
		seekhead_ptr = w->f_pos;
	if (w->seek_data.seekhead)
		CHECK(mk_writeSeek(w, c, MATROSKA_ID_SEEKHEAD, w->seek_data.seekhead));
	if (w->seek_data.segmentinfo)
		CHECK(mk_writeSeek(w, c, MATROSKA_ID_INFO, w->seek_data.segmentinfo));
	if (w->seek_data.tracks)
		CHECK(mk_writeSeek(w, c, MATROSKA_ID_TRACKS, w->seek_data.tracks));
	if (w->seek_data.cues)
		CHECK(mk_writeSeek(w, c, MATROSKA_ID_CUES, w->seek_data.cues));
	if (w->seek_data.attachments)
		CHECK(mk_writeSeek(w, c, MATROSKA_ID_ATTACHMENTS, w->seek_data.attachments));
	if (w->seek_data.chapters)
		CHECK(mk_writeSeek(w, c, MATROSKA_ID_CHAPTERS, w->seek_data.chapters));
	if (w->seek_data.tags)
		CHECK(mk_writeSeek(w, c, MATROSKA_ID_TAGS, w->seek_data.tags));
	CHECK(mk_closeContext(c, 0));

	if (pointer != NULL)
		*pointer = seekhead_ptr;

	return 0;
}

int mk_close(mk_Writer *w)
{
	int i, ret = 0;
	mk_Track *tk;
	int64_t max_frame_tc = w->tracks_arr[0]->max_frame_tc;
	uint64_t segment_size = 0;
	unsigned char c_size[8];
	unsigned char segment_uid[16];

	md5_finish(&w->segment_md5, segment_uid);

	for (i = w->num_tracks - 1; i >= 0; i--) {
		tk = w->tracks_arr[i];
		if (mk_flushFrame(w, tk) < 0)
			ret = -1;
	}

	if (mk_closeCluster(w) < 0)
		ret = -1;

	w->seek_data.cues = w->f_pos - w->segment_ptr;
	if (mk_closeContext(w->cues, 0) < 0)
		ret = -1;
	if (mk_flushContextData(w->root) < 0)
		ret = -1;

	if (w->cluster.seekhead) {
		w->seek_data.seekhead = w->f_pos - w->segment_ptr;
		if (mk_closeContext(w->cluster.seekhead, 0) < 0)
			ret = -1;
		if (mk_flushContextData(w->root) < 0)
			ret = -1;
	}

	if (w->attachments != NULL) {
		w->seek_data.attachments = w->f_pos - w->segment_ptr;
		mk_writeAttachments(w);
		if (mk_flushContextData(w->root) < 0)
			ret = -1;
	}

	if (w->tags != NULL) {
		w->seek_data.tags = w->f_pos - w->segment_ptr;
		mk_writeTags(w);
		if (mk_flushContextData(w->root) < 0)
			ret = -1;
	}

	if (w->chapters != NULL) {
		if (w->vlc_compat) {
			if (mk_flushContextData(w->root) < 0)
				ret = -1;
			if (mk_seekFile(w, w->segment_ptr + RESERVED_SEEKHEAD + 3) < 0)
				ret = -1;
		}
		w->seek_data.chapters = w->f_pos - w->segment_ptr;
		mk_writeChapters(w);
		if (mk_flushContextData(w->root) < 0)
			ret = -1;
		if (w->vlc_compat) {
			if (mk_writeVoid(w->root, (RESERVED_CHAPTERS - (w->f_pos - w->segment_ptr - RESERVED_SEEKHEAD - 3))) < 0)
				ret = -1;
			if (mk_flushContextData(w->root) < 0)
				ret = -1;
		}
	}

	if (w->wrote_header) {
		if (w->vlc_compat) {
			if (mk_seekFile(w, w->segment_ptr) < 0)
				ret = -1;
		}

		if (mk_writeSeekHead(w, &w->seek_data.seekhead) < 0)
			ret = -1;
		w->seek_data.seekhead -= w->segment_ptr;

		if (w->vlc_compat) {
			if (mk_flushContextData(w->root) < 0)
				ret = -1;
			if (mk_writeVoid(w->root, (RESERVED_SEEKHEAD - (w->f_pos - w->segment_ptr))) < 0)
				ret = -1;
		}

		if (mk_flushContextData(w->root) < 0)
			ret = -1;

		if (!w->vlc_compat) {
			int i = w->seek_data.segmentinfo;
			w->seek_data.segmentinfo = 0;
			w->seek_data.tracks = 0;
			w->seek_data.cues = 0;
			w->seek_data.chapters = 0;
			w->seek_data.attachments = 0;
			w->seek_data.tags = 0;
			if (mk_seekFile(w, w->segment_ptr) < 0)
				ret = -1;
			if (mk_writeSeekHead(w, NULL) < 0 || mk_flushContextData(w->root) < 0)
				ret = -1;
			// The conditional below is easier to understand, but incorrect
			// because f_pos is unsigned and causes the lhs to be evaluated
			// as an unsigned quantity.
			// if (((i + w->segment_ptr) - w->f_pos - 2) > 1)
			if ((i + w->segment_ptr) > (w->f_pos + 3))
				if (mk_writeVoid(w->root, (i + w->segment_ptr) - w->f_pos - 2) < 0
					|| mk_flushContextData(w->root) < 0)
					ret = -1;
		}

		if (mk_seekFile(w, w->duration_ptr) < 0)
			ret = -1;
		if (mk_writeFloatRaw(w->root,
							 (float) ((double) (max_frame_tc + w->def_duration) /
							 w->timescale)) < 0
			|| mk_flushContextData(w->root) < 0)
			ret = -1;
		if (mk_seekFile(w, w->segment_ptr - 8) < 0)
			ret = -1;
		segment_size = w->f_eof - w->segment_ptr;
		for (i = 7; i > 0; --i)
			c_size[i] = segment_size >> (8 * (7 - i));
		c_size[i] = 0x01;
		if (mk_appendContextData(w->root, &c_size, 8) < 0 ||
			mk_flushContextData(w->root) < 0)
			ret = -1;
		if (mk_seekFile(w, w->segmentuid_ptr) < 0)
			ret = -1;
		if (mk_writeBin(w->root, MATROSKA_ID_SEGMENTUID, segment_uid,
						sizeof(segment_uid)) < 0 ||
			mk_flushContextData(w->root) < 0)
			ret = -1;
	}

    /* update any track private data that may have changed */
	for (i = w->num_tracks - 1; i >= 0; i--) {
		tk = w->tracks_arr[i];
		if (tk->private_data_size && tk->private_data)
		{
			if (mk_seekFile(w, tk->private_data_ptr) < 0)
				ret = -1;
			if (mk_writeBin(w->root, MATROSKA_ID_CODECPRIVATE,
							tk->private_data, tk->private_data_size) < 0 ||
				mk_flushContextData(w->root) < 0)
				ret = -1;
			free(tk->private_data);
        }
		w->tracks_arr[i] = NULL;
		--w->num_tracks;
		free(tk);
	}

	if (mk_closeContext(w->root, 0) < 0)
		ret = -1;
	mk_destroyContexts(w);
	fclose(w->fp);
	free(w->tracks_arr);
	free(w);

	return ret;
}
