/*****************************************************************************
 * tracks.c:
 *****************************************************************************
 * Copyright (C) 2007 libmkv
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

/* TODO: Figure out what can actually fail without damaging the track. */

#define TRACK_STEP 4

mk_Track *mk_createTrack(mk_Writer *w, mk_TrackConfig *tc)
{
	mk_Context *ti, *v;
	int i;
	int64_t offset = 0;
	mk_Track *track = calloc(1, sizeof(*track));
	if (track == NULL)
		return NULL;

	if (w->num_tracks + 1 > w->alloc_tracks) {
		if ((w->tracks_arr = realloc(w->tracks_arr, (w->alloc_tracks + TRACK_STEP) * sizeof(mk_Track *))) == NULL)
			return NULL;		// FIXME
		w->alloc_tracks += TRACK_STEP;
	}
	w->tracks_arr[w->num_tracks] = track;
	track->track_id = ++w->num_tracks;

	if (w->tracks == NULL) {
		/* Tracks */
		if ((w->tracks = mk_createContext(w, w->root, MATROSKA_ID_TRACKS)) == NULL)
			return NULL;
	}

	/* TrackEntry */
	if ((ti = mk_createContext(w, w->tracks, MATROSKA_ID_TRACKENTRY)) == NULL)
		return NULL;
	/* TrackNumber */
	if (mk_writeUInt(ti, MATROSKA_ID_TRACKNUMBER, track->track_id) < 0)
		return NULL;
	if (tc->trackUID) {
		if (mk_writeUInt(ti, MATROSKA_ID_TRACKUID, tc->trackUID) < 0)	/* TrackUID  */
			return NULL;
	} else {
		/*
		 * If we aren't given a UID, randomly generate one.
		 * NOTE: It would probably be better to CRC32 some unique track information
		 *      in place of something completely random.
		 */
		unsigned long track_uid;
		track_uid = random();

		if (mk_writeUInt(ti, MATROSKA_ID_TRACKUID, track_uid) < 0)	/* TrackUID  */
			return NULL;
	}
	if (mk_writeUInt(ti, MATROSKA_ID_TRACKTYPE, tc->trackType) < 0)	/* TrackType */
		return NULL;
	track->track_type = tc->trackType;
	/* FlagLacing */
	if (mk_writeUInt(ti, MATROSKA_ID_TRACKFLAGLACING, tc->flagLacing) < 0)
		return NULL;
	if (mk_writeStr(ti, MATROSKA_ID_CODECID, tc->codecID) < 0)	/* CodecID */
		return NULL;
	if (tc->codecPrivateSize && (tc->codecPrivate != NULL)) {
		/* CodecPrivate */
		track->private_data_size = tc->codecPrivateSize;
		track->private_data_ptr = ti->d_cur;
		if (mk_writeBin(ti, MATROSKA_ID_CODECPRIVATE, tc->codecPrivate, tc->codecPrivateSize) < 0)
			return NULL;
	}
	if (tc->defaultDuration) {
		/* DefaultDuration */
		if (mk_writeUInt(ti, MATROSKA_ID_TRACKDEFAULTDURATION, tc->defaultDuration) < 0)
			return NULL;
		track->default_duration = tc->defaultDuration;
	}
	if (tc->language) {
		/* Language */
		if (mk_writeStr(ti, MATROSKA_ID_TRACKLANGUAGE, tc->language) < 0)
			return NULL;
	}
	if (tc->flagEnabled != 1) {
		/* FlagEnabled */
		if (mk_writeUInt(ti, MATROSKA_ID_TRACKFLAGENABLED, tc->flagEnabled) < 0)
			return NULL;
	}
	/* FlagDefault */
	if (mk_writeUInt(ti, MATROSKA_ID_TRACKFLAGDEFAULT, tc->flagDefault) < 0)
		return NULL;
	if (tc->flagForced) {
		/* FlagForced */
		if (mk_writeUInt(ti, MATROSKA_ID_TRACKFLAGFORCED, tc->flagForced) < 0)
			return NULL;
	}
	if (tc->minCache){
		/* MinCache */
		if (mk_writeUInt(ti, MATROSKA_ID_TRACKMINCACHE, tc->minCache) < 0)
			return NULL;
	}

	/* FIXME: this won't handle NULL values, which signals that the cache is disabled. */
	if (tc->maxCache) {
		/* MaxCache */
		if (mk_writeUInt(ti, MATROSKA_ID_TRACKMAXCACHE, tc->maxCache) < 0)
			return NULL;
	}

	switch (tc->trackType) {
		case MK_TRACK_VIDEO:
			/* Video */
			if ((v = mk_createContext(w, ti, MATROSKA_ID_TRACKVIDEO)) == NULL)
				return NULL;
			if (tc->extra.video.pixelCrop[0] != 0
				|| tc->extra.video.pixelCrop[1] != 0
				|| tc->extra.video.pixelCrop[2] != 0
				|| tc->extra.video.pixelCrop[3] != 0) {
				for (i = 0; i < 4; i++) {
					/* Each pixel crop ID is 0x11 away from the next.
					 * In order from 0x54AA to 0x54DD they are bottom, top, left, right.
					 */
					/* PixelCrop{Bottom,Top,Left,Right} */
					if (mk_writeUInt(v, MATROSKA_ID_VIDEOPIXELCROPBOTTOM + (i * 0x11), tc->extra.video.pixelCrop[i]) < 0)
						return NULL;
				}
			}
			/* PixelWidth */
			if (mk_writeUInt(v, MATROSKA_ID_VIDEOPIXELWIDTH, tc->extra.video.pixelWidth) < 0)
				return NULL;
			/* PixelHeight */
			if (mk_writeUInt(v, MATROSKA_ID_VIDEOPIXELHEIGHT, tc->extra.video.pixelHeight) < 0)
				return NULL;
			/* DisplayWidth */
			if (mk_writeUInt(v, MATROSKA_ID_VIDEODISPLAYWIDTH, tc->extra.video.displayWidth) < 0)
				return NULL;
			/* DisplayHeight */
			if (mk_writeUInt(v, MATROSKA_ID_VIDEODISPLAYHEIGHT, tc->extra.video.displayHeight) < 0)
				return NULL;
			if (tc->extra.video.displayUnit) {
				/* DisplayUnit */
				if (mk_writeUInt(v, MATROSKA_ID_VIDEODISPLAYUNIT, tc->extra.video.displayUnit) < 0)
					return NULL;
			}
			if (tc->extra.video.aspectRatioType != MK_ASPECTRATIO_FREE) {
				/* AspectRatioType */
				if (mk_writeUInt(v, MATROSKA_ID_VIDEOASPECTRATIOTYPE, tc->extra.video.aspectRatioType) < 0)
					return NULL;
			}
			if (mk_closeContext(v, 0) < 0)
				return NULL;
			break;
		case MK_TRACK_AUDIO:
			/* Audio */
			if ((v = mk_createContext(w, ti, MATROSKA_ID_TRACKAUDIO)) == NULL)
				return NULL;
			/* SamplingFrequency */
			if (mk_writeFloat(v, MATROSKA_ID_AUDIOSAMPLINGFREQ, tc->extra.audio.samplingFreq) < 0)
				return NULL;
			/* Channels */
			if (mk_writeUInt(v, MATROSKA_ID_AUDIOCHANNELS, tc->extra.audio.channels) < 0)
				return NULL;
			if (tc->extra.audio.bitDepth) {
				/* BitDepth */
				if (mk_writeUInt(v, MATROSKA_ID_AUDIOBITDEPTH, tc->extra.audio.bitDepth) < 0)
					return NULL;
			}
			if (mk_closeContext(v, 0) < 0)
				return NULL;
			break;
		case MK_TRACK_SUBTITLE:
			/* Subtitles */
			break;
		default:				/* Other track types
								 * TODO: Implement other valid track types.
								 */
			return NULL;
	}

	if (mk_closeContext(ti, &offset) < 0)
		return NULL;
	track->private_data_ptr += offset;

	return track;
}

int mk_updateTrackPrivateData(mk_Writer *w, mk_Track *track, uint8_t * data, int size )
{
	/* can not write data larger than was previously reserved */
	if (size > track->private_data_size)
		return -1;

	if (track->private_data == NULL)
		track->private_data = calloc(1, track->private_data_size);
	memcpy(track->private_data, data, size);
}

int mk_writeTracks(mk_Writer *w, mk_Context *tracks)
{
	int i;
	mk_Track * tk;
	int64_t offset = 0;

	w->seek_data.tracks = w->root->d_cur;

	CHECK(mk_closeContext(w->tracks, &offset));

	for (i = 0; i < w->num_tracks; i++) {
		tk = w->tracks_arr[i];
		if (tk->private_data_size)
			tk->private_data_ptr += offset;
	}

	return 0;
}
