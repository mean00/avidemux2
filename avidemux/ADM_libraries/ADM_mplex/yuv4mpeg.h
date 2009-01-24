/*
 *  yuv4mpeg.h:  Functions for reading and writing "new" YUV4MPEG2 streams.
 *
 *               Stream format is described at the end of this file.
 *
 *
 *  Copyright (C) 2004 Matthew J. Marjanovic <maddog@mir.com>
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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

#ifndef __YUV4MPEG_H__
#define __YUV4MPEG_H__

#include <stdlib.h>
#include <mjpeg_types.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <mjpeg_logging.h>


/************************************************************************
 *  error codes returned by y4m_* functions
 ************************************************************************/
#define Y4M_OK          0
#define Y4M_ERR_RANGE   1  /* argument or tag value out of range */
#define Y4M_ERR_SYSTEM  2  /* failed system call, check errno */
#define Y4M_ERR_HEADER  3  /* illegal/malformed header */
#define Y4M_ERR_BADTAG  4  /* illegal tag character */
#define Y4M_ERR_MAGIC   5  /* bad header magic */
#define Y4M_ERR_EOF     6  /* end-of-file (clean) */
#define Y4M_ERR_XXTAGS  7  /* too many xtags */
#define Y4M_ERR_BADEOF  8  /* unexpected end-of-file */
#define Y4M_ERR_FEATURE 9  /* stream requires features beyond allowed level */


/* generic 'unknown' value for integer parameters (e.g. interlace, height) */
#define Y4M_UNKNOWN -1

/************************************************************************
 * values for the "interlace" parameter [y4m_*_interlace()]
 ************************************************************************/
#define Y4M_ILACE_NONE          0   /* non-interlaced, progressive frame */
#define Y4M_ILACE_TOP_FIRST     1   /* interlaced, top-field first       */
#define Y4M_ILACE_BOTTOM_FIRST  2   /* interlaced, bottom-field first    */
#define Y4M_ILACE_MIXED         3   /* mixed, "refer to frame header"    */

/************************************************************************
 * values for the "chroma" parameter [y4m_*_chroma()]
 ************************************************************************/
#define Y4M_CHROMA_420JPEG     0  /* 4:2:0, H/V centered, for JPEG/MPEG-1 */
#define Y4M_CHROMA_420MPEG2    1  /* 4:2:0, H cosited, for MPEG-2         */
#define Y4M_CHROMA_420PALDV    2  /* 4:2:0, alternating Cb/Cr, for PAL-DV */
#define Y4M_CHROMA_444         3  /* 4:4:4, no subsampling, phew.         */
#define Y4M_CHROMA_422         4  /* 4:2:2, H cosited                     */
#define Y4M_CHROMA_411         5  /* 4:1:1, H cosited                     */
#define Y4M_CHROMA_MONO        6  /* luma plane only                      */
#define Y4M_CHROMA_444ALPHA    7  /* 4:4:4 with an alpha channel          */

/************************************************************************
 * values for sampling parameters [y4m_*_spatial(), y4m_*_temporal()]
 ************************************************************************/
#define Y4M_SAMPLING_PROGRESSIVE 0
#define Y4M_SAMPLING_INTERLACED  1

/************************************************************************
 * values for "presentation" parameter [y4m_*_presentation()]
 ************************************************************************/
#define Y4M_PRESENT_TOP_FIRST         0  /* top-field-first                 */
#define Y4M_PRESENT_TOP_FIRST_RPT     1  /* top-first, repeat top           */
#define Y4M_PRESENT_BOTTOM_FIRST      2  /* bottom-field-first              */
#define Y4M_PRESENT_BOTTOM_FIRST_RPT  3  /* bottom-first, repeat bottom     */
#define Y4M_PRESENT_PROG_SINGLE       4  /* single progressive frame        */
#define Y4M_PRESENT_PROG_DOUBLE       5  /* progressive frame, repeat once  */
#define Y4M_PRESENT_PROG_TRIPLE       6  /* progressive frame, repeat twice */

#define Y4M_MAX_NUM_PLANES 4

/************************************************************************
 *  'ratio' datatype, for rational numbers
 *                                     (see 'ratio' functions down below)
 ************************************************************************/
typedef struct _y4m_ratio {
  int n;  /* numerator   */
  int d;  /* denominator */
} y4m_ratio_t;


/************************************************************************
 *  useful standard framerates (as ratios)
 ************************************************************************/
extern const y4m_ratio_t y4m_fps_UNKNOWN;
extern const y4m_ratio_t y4m_fps_NTSC_FILM;  /* 24000/1001 film (in NTSC)  */
extern const y4m_ratio_t y4m_fps_FILM;       /* 24fps film                 */
extern const y4m_ratio_t y4m_fps_PAL;        /* 25fps PAL                  */
extern const y4m_ratio_t y4m_fps_NTSC;       /* 30000/1001 NTSC            */
extern const y4m_ratio_t y4m_fps_30;         /* 30fps                      */
extern const y4m_ratio_t y4m_fps_PAL_FIELD;  /* 50fps PAL field rate       */
extern const y4m_ratio_t y4m_fps_NTSC_FIELD; /* 60000/1001 NTSC field rate */
extern const y4m_ratio_t y4m_fps_60;         /* 60fps                      */

/************************************************************************
 *  useful standard sample (pixel) aspect ratios (W:H)
 ************************************************************************/
extern const y4m_ratio_t y4m_sar_UNKNOWN; 
extern const y4m_ratio_t y4m_sar_SQUARE;        /* square pixels */
extern const y4m_ratio_t y4m_sar_NTSC_CCIR601;  /* 525-line (NTSC) Rec.601 */
extern const y4m_ratio_t y4m_sar_NTSC_16_9;     /* 16:9 NTSC/Rec.601       */
extern const y4m_ratio_t y4m_sar_NTSC_SVCD_4_3; /* NTSC SVCD 4:3           */
extern const y4m_ratio_t y4m_sar_NTSC_SVCD_16_9;/* NTSC SVCD 16:9          */
extern const y4m_ratio_t y4m_sar_PAL_CCIR601;   /* 625-line (PAL) Rec.601  */
extern const y4m_ratio_t y4m_sar_PAL_16_9;      /* 16:9 PAL/Rec.601        */
extern const y4m_ratio_t y4m_sar_PAL_SVCD_4_3;  /* PAL SVCD 4:3            */
extern const y4m_ratio_t y4m_sar_PAL_SVCD_16_9; /* PAL SVCD 16:9           */
extern const y4m_ratio_t y4m_sar_SQR_ANA16_9;   /* anamorphic 16:9 sampled */
                                            /* from 4:3 with square pixels */

/************************************************************************
 *  useful standard display aspect ratios (W:H)
 ************************************************************************/
extern const y4m_ratio_t y4m_dar_UNKNOWN; 
extern const y4m_ratio_t y4m_dar_4_3;     /* standard TV   */
extern const y4m_ratio_t y4m_dar_16_9;    /* widescreen TV */
extern const y4m_ratio_t y4m_dar_221_100; /* word-to-your-mother TV */


#define Y4M_MAX_XTAGS 32        /* maximum number of xtags in list       */
#define Y4M_MAX_XTAG_SIZE 32    /* max length of an xtag (including 'X') */

typedef struct _y4m_xtag_list y4m_xtag_list_t;
typedef struct _y4m_stream_info y4m_stream_info_t;
typedef struct _y4m_frame_info y4m_frame_info_t;


#ifdef __cplusplus
#define BEGIN_CDECLS extern "C" {
#define END_CDECLS   }
#else
#define BEGIN_CDECLS 
#define END_CDECLS   
#endif

BEGIN_CDECLS

/************************************************************************
 *  'ratio' functions
 ************************************************************************/

/* 'normalize' a ratio (remove common factors) */
void y4m_ratio_reduce(y4m_ratio_t *r);

/* parse "nnn:ddd" into a ratio (returns Y4M_OK or Y4M_ERR_RANGE) */
int y4m_parse_ratio(y4m_ratio_t *r, const char *s);

/* quick test of two ratios for equality (i.e. identical components) */
#define Y4M_RATIO_EQL(a,b) ( ((a).n == (b).n) && ((a).d == (b).d) )

/* quick conversion of a ratio to a double (no divide-by-zero check!) */
#define Y4M_RATIO_DBL(r) ((double)(r).n / (double)(r).d)

/*************************************************************************
 *
 * Guess the true SAR (sample aspect ratio) from a list of commonly 
 * encountered values, given the "suggested" display aspect ratio (DAR),
 * and the true frame width and height.
 *
 * Returns y4m_sar_UNKNOWN if no match is found.
 *
 *************************************************************************/
y4m_ratio_t y4m_guess_sar(int width, int height, y4m_ratio_t dar);


/*************************************************************************
 *
 * Chroma Subsampling Mode information
 *
 *  x_ratio, y_ratio  -  subsampling of chroma planes
 *  x_offset, y_offset - offset of chroma sample grid,
 *                        relative to luma (0,0) sample
 *
 *************************************************************************/

y4m_ratio_t y4m_chroma_ss_x_ratio(int chroma_mode);
y4m_ratio_t y4m_chroma_ss_y_ratio(int chroma_mode);
#if 0
y4m_ratio_t y4m_chroma_ss_x_offset(int chroma_mode, int field, int plane);
y4m_ratio_t y4m_chroma_ss_y_offset(int chroma_mode, int field, int plane);
#endif

/* Given a string containing a (case-insensitive) chroma-tag keyword,
   return appropriate chroma mode (or Y4M_UNKNOWN) */
int y4m_chroma_parse_keyword(const char *s);

/* Given a Y4M_CHROMA_* mode, return appropriate chroma-tag keyword,
   or NULL if there is none. */
const char *y4m_chroma_keyword(int chroma_mode);

/* Given a Y4M_CHROMA_* mode, return appropriate chroma mode description,
   or NULL if there is none. */
const char *y4m_chroma_description(int chroma_mode);



/************************************************************************
 *  'xtag' functions
 *
 * o Before using an xtag_list (but after the structure/memory has been
 *    allocated), you must initialize it via y4m_init_xtag_list().
 * o After using an xtag_list (but before the structure is released),
 *    call y4m_fini_xtag_list() to free internal memory.
 *
 ************************************************************************/

/* initialize an xtag_list structure */
void y4m_init_xtag_list(y4m_xtag_list_t *xtags);

/* finalize an xtag_list structure */
void y4m_fini_xtag_list(y4m_xtag_list_t *xtags);

/* make one xtag_list into a copy of another */
void y4m_copy_xtag_list(y4m_xtag_list_t *dest, const y4m_xtag_list_t *src);

/* return number of tags in an xtag_list */
int y4m_xtag_count(const y4m_xtag_list_t *xtags);

/* access n'th tag in an xtag_list */
const char *y4m_xtag_get(const y4m_xtag_list_t *xtags, int n);

/* append a new tag to an xtag_list
    returns:          Y4M_OK - success
              Y4M_ERR_XXTAGS - list is already full */
int y4m_xtag_add(y4m_xtag_list_t *xtags, const char *tag);

/* remove a tag from an xtag_list 
    returns:         Y4M_OK - success
              Y4M_ERR_RANGE - n is out of range */
int y4m_xtag_remove(y4m_xtag_list_t *xtags, int n);

/* remove all tags from an xtag_list 
    returns:   Y4M_OK - success       */
int y4m_xtag_clearlist(y4m_xtag_list_t *xtags);

/* append copies of tags from src list to dest list
    returns:          Y4M_OK - success
              Y4M_ERR_XXTAGS - operation would overfill dest list */
int y4m_xtag_addlist(y4m_xtag_list_t *dest, const y4m_xtag_list_t *src);



/************************************************************************
 *  '*_info' functions
 *
 * o Before using a *_info structure (but after the structure/memory has
 *    been allocated), you must initialize it via y4m_init_*_info().
 * o After using a *_info structure (but before the structure is released),
 *    call y4m_fini_*_info() to free internal memory.
 * o Use the 'set' and 'get' accessors to modify or access the fields in
 *    the structures; don't touch the structure directly.  (Ok, so there
 *    is no really convenient C syntax to prevent you from doing this,
 *    but we are all responsible programmers here, so just don't do it!)
 *
 ************************************************************************/

/* initialize a stream_info structure */
void y4m_init_stream_info(y4m_stream_info_t *i);

/* finalize a stream_info structure */
void y4m_fini_stream_info(y4m_stream_info_t *i);

/* reset stream_info back to default/unknown values */
void y4m_clear_stream_info(y4m_stream_info_t *info);

/* make one stream_info into a copy of another */
void y4m_copy_stream_info(y4m_stream_info_t *dest,
			  const y4m_stream_info_t *src);

/* access or set stream_info fields */
/*      level 0                   */
int y4m_si_get_width(const y4m_stream_info_t *si);
int y4m_si_get_height(const y4m_stream_info_t *si);
int y4m_si_get_interlace(const y4m_stream_info_t *si);
y4m_ratio_t y4m_si_get_framerate(const y4m_stream_info_t *si);
y4m_ratio_t y4m_si_get_sampleaspect(const y4m_stream_info_t *si);
void y4m_si_set_width(y4m_stream_info_t *si, int width);
void y4m_si_set_height(y4m_stream_info_t *si, int height);
void y4m_si_set_interlace(y4m_stream_info_t *si, int interlace);
void y4m_si_set_framerate(y4m_stream_info_t *si, y4m_ratio_t framerate);
void y4m_si_set_sampleaspect(y4m_stream_info_t *si, y4m_ratio_t sar);
/*      level 1                   */
void y4m_si_set_chroma(y4m_stream_info_t *si, int chroma_mode);
int y4m_si_get_chroma(const y4m_stream_info_t *si);

/* derived quantities (no setter) */
/*      level 0                   */
int y4m_si_get_framelength(const y4m_stream_info_t *si);
/*      level 1                   */
int y4m_si_get_plane_count(const y4m_stream_info_t *si);
int y4m_si_get_plane_width(const y4m_stream_info_t *si, int plane);
int y4m_si_get_plane_height(const y4m_stream_info_t *si, int plane);
int y4m_si_get_plane_length(const y4m_stream_info_t *si, int plane);


/* access stream_info xtag_list */
y4m_xtag_list_t *y4m_si_xtags(y4m_stream_info_t *si);


/* initialize a frame_info structure */
void y4m_init_frame_info(y4m_frame_info_t *i);

/* finalize a frame_info structure */
void y4m_fini_frame_info(y4m_frame_info_t *i);

/* reset frame_info back to default/unknown values */
void y4m_clear_frame_info(y4m_frame_info_t *info);

/* make one frame_info into a copy of another */
void y4m_copy_frame_info(y4m_frame_info_t *dest,
			 const y4m_frame_info_t *src);


/* access or set frame_info fields (level 1) */
int y4m_fi_get_presentation(const y4m_frame_info_t *fi);
int y4m_fi_get_temporal(const y4m_frame_info_t *fi);
int y4m_fi_get_spatial(const y4m_frame_info_t *fi);

void y4m_fi_set_presentation(y4m_frame_info_t *fi, int pres);
void y4m_fi_set_temporal(y4m_frame_info_t *fi, int sampling);
void y4m_fi_set_spatial(y4m_frame_info_t *fi, int sampling);


/* access frame_info xtag_list */
y4m_xtag_list_t *y4m_fi_xtags(y4m_frame_info_t *fi);



/************************************************************************
 *  blocking read and write functions
 *
 *  o guaranteed to transfer entire payload (or fail)
 *  o return values:
 *                         0 (zero)   complete success
 *          -(# of remaining bytes)   error (and errno left set)
 *          +(# of remaining bytes)   EOF (for y4m_read only)
 *
 ************************************************************************/

/* read len bytes from fd into buf */
ssize_t y4m_read(int fd, void *buf, size_t len);

/* write len bytes from fd into buf */
ssize_t y4m_write(int fd, const void *buf, size_t len);



/************************************************************************
 *  stream header processing functions
 *  
 *  o return values:
 *                   Y4M_OK - success
 *                Y4M_ERR_* - error (see y4m_strerr() for descriptions)
 *
 ************************************************************************/

/* parse a string of stream header tags */
int y4m_parse_stream_tags(char *s, y4m_stream_info_t *i);

/* read a stream header from file descriptor fd
   (the current contents of stream_info are erased first) */
int y4m_read_stream_header(int fd, y4m_stream_info_t *i);

/* write a stream header to file descriptor fd */
int y4m_write_stream_header(int fd, const y4m_stream_info_t *i);



/************************************************************************
 *  frame processing functions
 *  
 *  o return values:
 *                   Y4M_OK - success
 *                Y4M_ERR_* - error (see y4m_strerr() for descriptions)
 *
 ************************************************************************/

/* write a frame header to file descriptor fd */
int y4m_write_frame_header(int fd,
			   const y4m_stream_info_t *si,
			   const y4m_frame_info_t *fi);

/* write a complete frame (header + data)
   o planes[] points to 1-4 buffers, one each for image plane */
int y4m_write_frame(int fd, const y4m_stream_info_t *si, 
		    const y4m_frame_info_t *fi, uint8_t * const *planes);


/* write a complete frame (header + data), but interleave fields
    from two separate buffers
   o upper_field[] same as planes[] above, but for upper field only
   o lower_field[] same as planes[] above, but for lower field only
*/
int y4m_write_fields(int fd, const y4m_stream_info_t *si, 
		     const y4m_frame_info_t *fi,
		     uint8_t * const *upper_field, 
		     uint8_t * const *lower_field);


/* read a frame header from file descriptor fd 
   (the current contents of frame_info are erased first) */
int y4m_read_frame_header(int fd,
			  const y4m_stream_info_t *si,
			  y4m_frame_info_t *fi);

/* read frame data [to be called after y4m_read_frame_header()]
   o planes[] points to 1-4 buffers, one each for image plane */
int y4m_read_frame_data(int fd, const y4m_stream_info_t *si, 
                        y4m_frame_info_t *fi, uint8_t * const *planes);

/* read frame data, but de-interleave fields into two separate buffers
    [to be called after y4m_read_frame_header()]
   o upper_field[] same as planes[] above, but for upper field only
   o lower_field[] same as planes[] above, but for lower field only
*/
int y4m_read_fields_data(int fd, const y4m_stream_info_t *si, 
                         y4m_frame_info_t *fi,
                         uint8_t * const *upper_field, 
                         uint8_t * const *lower_field);

/* read a complete frame (header + data)
   o planes[] points to 1-4 buffers, one each for image plane */
int y4m_read_frame(int fd, const y4m_stream_info_t *si, 
		   y4m_frame_info_t *fi, uint8_t * const *planes);

/* read a complete frame (header + data), but de-interleave fields
    into two separate buffers
   o upper_field[] same as planes[] above, but for upper field only
   o lower_field[] same as planes[] above, but for lower field only
*/
int y4m_read_fields(int fd, const y4m_stream_info_t *si, 
		    y4m_frame_info_t *fi,
		    uint8_t * const *upper_field, 
		    uint8_t * const *lower_field);


/************************************************************************
 *  miscellaneous functions
 ************************************************************************/

/* convenient dump of stream header info via mjpeg_log facility
 *  - each logged/printed line is prefixed by 'prefix'
 */
void y4m_log_stream_info(log_level_t level, const char *prefix,
			 const y4m_stream_info_t *i);

/* convert a Y4M_ERR_* error code into mildly explanatory string */
const char *y4m_strerr(int err);

/* set 'allow_unknown_tag' flag for library...
    o yn = 0 :  unknown header tags will produce a parsing error
    o yn = 1 :  unknown header tags/values will produce a warning, but
                 are otherwise passed along via the xtags list
    o yn = -1:  don't change, just return current setting

   return value:  previous setting of flag
*/
int y4m_allow_unknown_tags(int yn);


/* set level of "accepted extensions" for the library...
    o level = 0:  default - conform to original YUV4MPEG2 spec; yield errors
                   when reading or writing a stream which exceeds it.
    o level = 1:  allow reading/writing streams which contain non-420jpeg
                   chroma and/or mixed-mode interlacing
    o level = -1: don't change, just return current setting

   return value:  previous setting of level
 */
int y4m_accept_extensions(int level);


END_CDECLS


/************************************************************************
 ************************************************************************

  Description of the (new!, forever?) YUV4MPEG2 stream format:

  STREAM consists of
    o one '\n' terminated STREAM-HEADER
    o unlimited number of FRAMEs

  FRAME consists of
    o one '\n' terminated FRAME-HEADER
    o "length" octets of planar YCrCb 4:2:0 image data
        (if frame is interlaced, then the two fields are interleaved)


  STREAM-HEADER consists of
     o string "YUV4MPEG2"
     o unlimited number TAGGED-FIELDs, each preceded by ' ' separator
     o '\n' line terminator

  FRAME-HEADER consists of
     o string "FRAME"
     o unlimited number of TAGGED-FIELDs, each preceded by ' ' separator
     o '\n' line terminator


  TAGGED-FIELD consists of
     o single ascii character tag
     o VALUE (which does not contain whitespace)

  VALUE consists of
     o integer (base 10 ascii representation)
  or o RATIO
  or o single ascii character
  or o non-whitespace ascii string

  RATIO consists of
     o numerator (integer)
     o ':' (a colon)
     o denominator (integer)


  The currently supported tags for the STREAM-HEADER:
     W - [integer] frame width, pixels, should be > 0
     H - [integer] frame height, pixels, should be > 0
     C - [string]  chroma-subsampling/data format
           420jpeg   (default)
           420mpeg2
           420paldv
           411
           422
           444       - non-subsampled Y'CbCr
	   444alpha  - Y'CbCr with alpha channel (with Y' black/white point)
           mono      - Y' plane only
     I - [char] interlacing:  p - progressive (none)
                              t - top-field-first
                              b - bottom-field-first
                              m - mixed -- see 'I' tag in frame header
                              ? - unknown
     F - [ratio] frame-rate, 0:0 == unknown
     A - [ratio] sample (pixel) aspect ratio, 0:0 == unknown
     X - [character string] 'metadata' (unparsed, but passed around)

  The currently supported tags for the FRAME-HEADER:
     Ixyz - framing/sampling (required if-and-only-if stream is "Im")
          x:  t - top-field-first
              T - top-field-first and repeat
	      b - bottom-field-first
              B - bottom-field-first and repeat
              1 - single progressive frame
              2 - double progressive frame (repeat)
              3 - triple progressive frame (repeat twice)

          y:  p - progressive:  fields sampled at same time
              i - interlaced:   fields sampled at different times

          z:  p - progressive:  subsampling over whole frame
              i - interlaced:   each field subsampled independently
              ? - unknown (allowed only for non-4:2:0 subsampling)           
       
     X - character string 'metadata' (unparsed, but passed around)

 ************************************************************************
 ************************************************************************/


/*

   THAT'S ALL FOLKS!

   Thank you for reading the source code.  We hope you have thoroughly
   enjoyed the experience.

*/





#ifdef INTERNAL_Y4M_LIBCODE_STUFF_QPX
#define Y4MPRIVATIZE(identifier) identifier
#else
#define Y4MPRIVATIZE(identifier) PRIVATE##identifier
#endif

/* 
 * Actual structure definitions of structures which you shouldn't touch.
 *
 */

/************************************************************************
 *  'xtag_list' --- list of unparsed and/or meta/X header tags
 *
 *     Do not touch this structure directly!
 *
 *     Use the y4m_xtag_*() functions (see below).
 *     You must initialize/finalize this structure before/after use.
 ************************************************************************/
struct _y4m_xtag_list {
  int Y4MPRIVATIZE(count);
  char *Y4MPRIVATIZE(tags)[Y4M_MAX_XTAGS];
};


/************************************************************************
 *  'stream_info' --- stream header information
 *
 *     Do not touch this structure directly!
 *
 *     Use the y4m_si_*() functions (see below).
 *     You must initialize/finalize this structure before/after use.
 ************************************************************************/
struct _y4m_stream_info {
  /* values from header/setters */
  int Y4MPRIVATIZE(width);
  int Y4MPRIVATIZE(height);
  int Y4MPRIVATIZE(interlace);            /* see Y4M_ILACE_* definitions  */
  y4m_ratio_t Y4MPRIVATIZE(framerate);    /* see Y4M_FPS_* definitions    */
  y4m_ratio_t Y4MPRIVATIZE(sampleaspect); /* see Y4M_SAR_* definitions    */
  int Y4MPRIVATIZE(chroma);               /* see Y4M_CHROMA_* definitions */

  /* mystical X tags */
  y4m_xtag_list_t Y4MPRIVATIZE(x_tags);
};


/************************************************************************
 *  'frame_info' --- frame header information
 *
 *     Do not touch this structure directly!
 *
 *     Use the y4m_fi_*() functions (see below).
 *     You must initialize/finalize this structure before/after use.
 ************************************************************************/

struct _y4m_frame_info {
  int Y4MPRIVATIZE(spatial);      /* see Y4M_SAMPLING_* definitions */
  int Y4MPRIVATIZE(temporal);     /* see Y4M_SAMPLING_* definitions */
  int Y4MPRIVATIZE(presentation); /* see Y4M_PRESENT_* definitions  */
  /* mystical X tags */
  y4m_xtag_list_t Y4MPRIVATIZE(x_tags);
};


#undef Y4MPRIVATIZE


#endif /* __YUV4MPEG_H__ */


