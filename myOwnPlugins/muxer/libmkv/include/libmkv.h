/*****************************************************************************
 * libmkv.h:
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
#ifndef _LIBMKV_H
#define _LIBMKV_H 1

#ifdef __cplusplus
extern "C" {
#endif							/* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif

/* Video codecs */
#define MK_VCODEC_RAW      "V_UNCOMPRESSED"
#define MK_VCODEC_MPEG1    "V_MPEG1"
#define MK_VCODEC_MPEG2    "V_MPEG2"
#define MK_VCODEC_THEORA   "V_THEORA"
#define MK_VCODEC_SNOW     "V_SNOW"
#define MK_VCODEC_MP4SP    "V_MPEG4/ISO/SP"
#define MK_VCODEC_MP4ASP   "V_MPEG4/ISO/ASP"
#define MK_VCODEC_MP4AP    "V_MPEG4/ISO/AP"
#define MK_VCODEC_MP4AVC   "V_MPEG4/ISO/AVC"
#define MK_VCODEC_MSVCM    "V_MS/VFW/FOURCC"
#define MK_VCODEC_MSMP4V3  "V_MPEG4/MS/V3"
#define MK_VCODEC_REAL     "V_REAL"
#define MK_VCODEC_REAL10   MK_VCODEC_REAL "/RV10"
#define MK_VCODEC_REAL20   MK_VCODEC_REAL "/RV20"
#define MK_VCODEC_REAL30   MK_VCODEC_REAL "/RV30"
#define MK_VCODEC_REAL40   MK_VCODEC_REAL "/RV40"
#define MK_VCODEC_QUICKTIME "V_QUICKTIME"

/* Audio codecs */
#define MK_ACODEC_AC3      "A_AC3"
#define MK_ACODEC_AC3BSID9 "A_AC3/BSID9"
#define MK_ACODEC_AC3BSID10 "A_AC3/BSID10"
#define MK_ACODEC_MP3      "A_MPEG/L3"
#define MK_ACODEC_MP2      "A_MPEG/L2"
#define MK_ACODEC_MP1      "A_MPEG/L1"
#define MK_ACODEC_DTS      "A_DTS"
#define MK_ACODEC_PCMINTLE "A_PCM/INT/LIT"
#define MK_ACODEC_PCMINTBE "A_PCM/INT/BIG"
#define MK_ACODEC_PCMFLTLE "A_PCM/FLOAT/IEEE"
#define MK_ACODEC_TTA1     "A_TTA1"
#define MK_ACODEC_WAVPACK  "A_WAVPACK4"
#define MK_ACODEC_VORBIS   "A_VORBIS"
#define MK_ACODEC_FLAC     "A_FLAC"
#define MK_ACODEC_AAC      "A_AAC"
#define MK_ACODEC_MPC      "A_MPC"
#define MK_ACODEC_REAL1    "A_REAL/14_4"
#define MK_ACODEC_REAL2    "A_REAL/28_8"
#define MK_ACODEC_REALCOOK "A_REAL/COOK"
#define MK_ACODEC_REALSIPR "A_REAL/SIPR"
#define MK_ACODEC_REALRALF "A_REAL/RALF"
#define MK_ACODEC_REALATRC "A_REAL/ATRC"
#define MK_ACODEC_MSACM    "A_MS/ACM"
#define MK_ACODEC_QUICKTIME "A_QUICKTIME"
#define MK_ACODEC_QDMC     MK_ACODEC_QUICKTIME "/QDMC"
#define MK_ACODEC_QDM2     MK_ACODEC_QUICKTIME "/QDM2"
#define MK_ACODEC_TTA1     "A_TTA1"
#define MK_ACODEC_WAVPACK4 "A_WAVEPACK4"

/* Subtitles */
#define MK_SUBTITLE_ASCII  "S_TEXT/ASCII"
#define MK_SUBTITLE_UTF8   "S_TEXT/UTF8"
#define MK_SUBTITLE_SSA    "S_TEXT/SSA"
#define MK_SUBTITLE_ASS    "S_TEXT/ASS"
#define MK_SUBTITLE_USF    "S_TEXT/USF"
#define MK_SUBTITLE_VOBSUB "S_VOBSUB"
#define MK_SUBTITLE_BMP    "S_IMAGE/BMP"

/* Official Tags */
#define MK_TAG_TITLE		"TITLE"
#define MK_TAG_SUBTITLE		"SUBTITLE"
#define MK_TAG_DIRECTOR		"DIRECTOR"
#define MK_TAG_ACTOR		"ACTOR"
#define MK_TAG_WRITER		"WRITTEN_BY"
#define MK_TAG_SCREENPLAY	"SCREENPLAY_BY"
#define MK_TAG_PRODUCER		"PRODUCER"
#define MK_TAG_EXECUTIVEPRODUCER	"EXECUTIVE_PRODUCER"
#define MK_TAG_GENRE		"GENRE"
#define MK_TAG_SUMMARY		"SUMMARY"
#define MK_TAG_SYNOPSIS		"SYNOPSIS"
#define MK_TAG_LAWRATING	"LAW_RATING"
#define MK_TAG_COMMENT		"COMMENT"
#define MK_TAG_PLAYCOUNT	"PLAY_COUNTER"
#define MK_TAG_RATING		"RATING"

typedef enum {
	MK_TRACK_VIDEO = 0x01,
	MK_TRACK_AUDIO = 0x02,
	MK_TRACK_COMPLEX = 0x03,
	MK_TRACK_LOGO = 0x10,
	MK_TRACK_SUBTITLE = 0x11,
	MK_TRACK_BUTTONS = 0x12,
	MK_TRACK_CONTROL = 0x20
} mk_TrackType;

typedef enum {
	MK_LACING_NONE = 0x00,
	MK_LACING_XIPH,
	MK_LACING_FIXED,
	MK_LACING_EBML
} mk_LacingType;

typedef enum {
	MK_ASPECTRATIO_FREE = 0x00,
	MK_ASPECTRATIO_KEEP,
	MK_ASPECTRATIO_FIXED
} mk_AspectType;

/* The enum uses the video naming where there isn't a common name for the targettype.
 * Later we define the other names to their equivelent enum value.
 */
enum {
	MK_TARGETTYPE_SHOT = 10,
	MK_TARGETTYPE_SCENE = 20,
	MK_TARGETTYPE_CHAPTER = 30,
	MK_TARGETTYPE_PART = 40,
	MK_TARGETTYPE_MOVIE = 50,
	MK_TARGETTYPE_SEQUEL = 60,
	MK_TARGETTYPE_COLLECTION = 70
};
#define MK_TARGETTYPE_SUBTRACK  MK_TARGETTYPE_SCENE
#define MK_TARGETTYPE_MOVEMENT  MK_TARGETTYPE_SCENE
#define MK_TARGETTYPE_TRACK  MK_TARGETTYPE_CHAPTER
#define MK_TARGETTYPE_SONG   MK_TARGETTYPE_CHAPTER
#define MK_TARGETTYPE_SESSION  MK_TARGETTYPE_PART
#define MK_TARGETTYPE_EPISODE  MK_TARGETTYPE_MOVIE
#define MK_TARGETTYPE_CONCERT  MK_TARGETTYPE_MOVIE
#define MK_TARGETTYPE_ALBUM    MK_TARGETTYPE_MOVIE
#define MK_TARGETTYPE_OPERA    MK_TARGETTYPE_MOVIE
#define MK_TARGETTYPE_SEASON   MK_TARGETTYPE_SEQUEL
#define MK_TARGETTYPE_VOLUME   MK_TARGETTYPE_SEQUEL
#define MK_TARGETTYPE_EDITION  MK_TARGETTYPE_SEQUEL
#define MK_TARGETTYPE_ISSUE    MK_TARGETTYPE_SEQUEL
#define MK_TARGETTYPE_OPUS     MK_TARGETTYPE_SEQUEL

typedef struct mk_Writer_s mk_Writer;
typedef struct mk_Track_s mk_Track;
typedef struct mk_TrackConfig_s mk_TrackConfig;

struct mk_TrackConfig_s {
	uint64_t trackUID;			/* Optional: Unique identifier for the track. */
	mk_TrackType trackType;		/* Required: 1 = Video, 2 = Audio. */
	int8_t flagEnabled;			/* Required: Set 1 if the track is used,
								 *			 0 if unused. (Default: enabled)
								 */
	int8_t flagDefault;			/* Required: Set 1 if this track is default,
								 *			 0 if not default, -1 is undefined.
								 */
	int8_t flagForced;			/* Optional: Set 1 if the track MUST be shown during playback
								 *			 (Default: disabled)
								 */
	int8_t flagLacing;			/* Required: Set 1 if the track may contain blocks using lacing. */
	uint8_t minCache;			/* Optional: See Matroska spec. (Default: cache disabled) */
	uint8_t maxCache;
	int64_t defaultDuration;	/* Optional: Number of nanoseconds per frame. */
	char *name;
	char *language;
	char *codecID;				/* Required: See codecs above. */
	void *codecPrivate;
	unsigned codecPrivateSize;
	char *codecName;
	union {
		struct {
			char flagInterlaced;
			unsigned pixelWidth;	/* Pixel width */
			unsigned pixelHeight;	/* Pixel height */
			unsigned pixelCrop[4];	/* Pixel crop - 0 = bottom, 1 = top, 2 = left, 3 = right */
			unsigned displayWidth;	/* Display width */
			unsigned displayHeight;	/* Display height */
			char displayUnit;		/* Display Units - 0 = pixels, 1 = cm, 2 = in */
			mk_AspectType aspectRatioType;	/* Specifies the allowed
											 * modifications to the aspect ratio
											 */
		} video;
		struct {
			float samplingFreq;	/* Sampling Frequency in Hz */
			unsigned channels;	/* Number of channels for this track */
			unsigned bitDepth;	/* Bits per sample (PCM) */
		} audio;
	} extra;
};

/* vlc_compat writes the Seek entries at the top of the file because VLC
 * stops parsing once it finds the first cluster. However, this creates
 * extra overhead in the file.
 */
mk_Writer *mk_createWriter(const char *filename, int64_t timescale,
						   uint8_t vlc_compat);
mk_Track *mk_createTrack(mk_Writer *w, mk_TrackConfig *tc);
int mk_updateTrackPrivateData(mk_Writer *w, mk_Track *track, uint8_t * data, int size );
int mk_writeHeader(mk_Writer *w, const char *writingApp);
int mk_startFrame(mk_Writer *w, mk_Track *track);
int mk_flushFrame(mk_Writer *w, mk_Track *track);
int mk_addFrameData(mk_Writer * w, mk_Track *track, const void *data,
					unsigned size);
int mk_setFrameFlags(mk_Writer * w, mk_Track *track, int64_t timestamp,
					 unsigned keyframe, uint64_t duration);
int mk_setFrameLacing(mk_Writer *w, mk_Track * track,
					  mk_LacingType lacing, uint8_t num_frames,
					  uint64_t sizes[]);
int mk_createChapterSimple(mk_Writer *w, uint64_t start, uint64_t end,
						   char *name);
int mk_close(mk_Writer *w);

char *mk_laceXiph(uint64_t *sizes, uint8_t num_frames,
				  uint64_t *output_size);

int mk_createTagSimple(mk_Writer *w, char *tag_id, char *value);
int mk_createTagSimpleBin(mk_Writer *w, char *tag_id, const void *data, 
						  unsigned size);
int mk_createAttachment( mk_Writer * w, char *name, char *description, 
						 char *mime, const void *data, unsigned size);

#ifdef __cplusplus
}
#endif							/* __cplusplus */

#endif							/* _LIBMKV_H */
