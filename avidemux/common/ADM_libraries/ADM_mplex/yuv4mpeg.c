/*
 *  yuv4mpeg.c:  Functions for reading and writing "new" YUV4MPEG streams
 *
 *  Copyright (C) 2001 Matthew J. Marjanovic <maddog@mir.com>
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
 *
 */
#define ADM_LEGACY_PROGGY
#include "ADM_default.h"
#define INTERNAL_Y4M_LIBCODE_STUFF_QPX
#include "yuv4mpeg.h"
#include "yuv4mpeg_intern.h"
#include "mjpeg_logging.h"


static int _y4mparam_allow_unknown_tags = 1;  /* default is forgiveness */
static int _y4mparam_feature_level = 0;       /* default is ol YUV4MPEG2 */

static void *(*_y4m_alloc)(size_t bytes) = malloc;
static void (*_y4m_free)(void *ptr) = free;



int y4m_allow_unknown_tags(int yn)
{
  int old = _y4mparam_allow_unknown_tags;
  if (yn >= 0)
    _y4mparam_allow_unknown_tags = (yn) ? 1 : 0;
  return old;
}


int y4m_accept_extensions(int level)
{
  int old = _y4mparam_feature_level;
  if (level >= 0)
    _y4mparam_feature_level = level;
  return old;
}



/*************************************************************************
 *
 * Convenience functions for fd read/write
 *
 *   - guaranteed to transfer entire payload (or fail)
 *   - returns:
 *               0 on complete success
 *               +(# of remaining bytes) on eof (for y4m_read)
 *               -(# of rem. bytes) on error (and ERRNO should be set)
 *     
 *************************************************************************/


ssize_t y4m_read(int fd, void *buf, size_t len)
{
   ssize_t n;
   uint8_t *ptr = (uint8_t *)buf;

   while (len > 0) {
     n = read(fd, ptr, len);
     if (n <= 0) {
       /* return amount left to read */
       if (n == 0)
	 return len;  /* n == 0 --> eof */
       else
	 return -len; /* n < 0 --> error */
     }
     ptr += n;
     len -= n;
   }
   return 0;
}


ssize_t y4m_write(int fd, const void *buf, size_t len)
{
   ssize_t n;
   const uint8_t *ptr = (const uint8_t *)buf;

   while (len > 0) {
     n = write(fd, ptr, len);
     if (n <= 0) return -len;  /* return amount left to write */
     ptr += n;
     len -= n;
   }
   return 0;
}




/*************************************************************************
 *
 * "Extra tags" handling
 *
 *************************************************************************/


static char *y4m_new_xtag(void)
{
  return _y4m_alloc(Y4M_MAX_XTAG_SIZE * sizeof(char));
}


void y4m_init_xtag_list(y4m_xtag_list_t *xtags)
{
  int i;
  xtags->count = 0;
  for (i = 0; i < Y4M_MAX_XTAGS; i++) {
    xtags->tags[i] = NULL;
  }
}


void y4m_fini_xtag_list(y4m_xtag_list_t *xtags)
{
  int i;
  for (i = 0; i < Y4M_MAX_XTAGS; i++) {
    if (xtags->tags[i] != NULL) {
      _y4m_free(xtags->tags[i]);
      xtags->tags[i] = NULL;
    }
  }
  xtags->count = 0;
}


void y4m_copy_xtag_list(y4m_xtag_list_t *dest, const y4m_xtag_list_t *src)
{
  int i;
  for (i = 0; i < src->count; i++) {
    if (dest->tags[i] == NULL) 
      dest->tags[i] = y4m_new_xtag();
    strncpy(dest->tags[i], src->tags[i], Y4M_MAX_XTAG_SIZE);
  }
  dest->count = src->count;
}



static int y4m_snprint_xtags(char *s, int maxn, const y4m_xtag_list_t *xtags)
{
  int i, room;
  
  for (i = 0, room = maxn - 1; i < xtags->count; i++) {
    int n = snprintf(s, room + 1, " %s", xtags->tags[i]);
    if ((n < 0) || (n > room)) return Y4M_ERR_HEADER;
    s += n;
    room -= n;
  }
  s[0] = '\n';  /* finish off header with newline */
  s[1] = '\0';  /* ...and end-of-string           */
  return Y4M_OK;
}


int y4m_xtag_count(const y4m_xtag_list_t *xtags)
{
  return xtags->count;
}


const char *y4m_xtag_get(const y4m_xtag_list_t *xtags, int n)
{
  if (n >= xtags->count)
    return NULL;
  else
    return xtags->tags[n];
}


int y4m_xtag_add(y4m_xtag_list_t *xtags, const char *tag)
{
  if (xtags->count >= Y4M_MAX_XTAGS) return Y4M_ERR_XXTAGS;
  if (xtags->tags[xtags->count] == NULL) 
    xtags->tags[xtags->count] = y4m_new_xtag();
  strncpy(xtags->tags[xtags->count], tag, Y4M_MAX_XTAG_SIZE);
  (xtags->count)++;
  return Y4M_OK;
}


int y4m_xtag_remove(y4m_xtag_list_t *xtags, int n)
{
  int i;
  char *q;

  if ((n < 0) || (n >= xtags->count)) return Y4M_ERR_RANGE;
  q = xtags->tags[n];
  for (i = n; i < (xtags->count - 1); i++)
    xtags->tags[i] = xtags->tags[i+1];
  xtags->tags[i] = q;
  (xtags->count)--;
  return Y4M_OK;
}


int y4m_xtag_clearlist(y4m_xtag_list_t *xtags)
{
  xtags->count = 0;
  return Y4M_OK;
}


int y4m_xtag_addlist(y4m_xtag_list_t *dest, const y4m_xtag_list_t *src)
{
  int i, j;

  if ((dest->count + src->count) > Y4M_MAX_XTAGS) return Y4M_ERR_XXTAGS;
  for (i = dest->count, j = 0;
       j < src->count;
       i++, j++) {
    if (dest->tags[i] == NULL) 
      dest->tags[i] = y4m_new_xtag();
    strncpy(dest->tags[i], src->tags[i], Y4M_MAX_XTAG_SIZE);
  }
  dest->count += src->count;
  return Y4M_OK;
}  


/*************************************************************************
 *
 * Creators/destructors for y4m_*_info_t structures
 *
 *************************************************************************/


void y4m_init_stream_info(y4m_stream_info_t *info)
{
  if (info == NULL) return;
  /* init substructures */
  y4m_init_xtag_list(&(info->x_tags));
  /* set defaults */
  y4m_clear_stream_info(info);
}


void y4m_clear_stream_info(y4m_stream_info_t *info)
{
  if (info == NULL) return;
  /* clear/initialize info */
  info->width = Y4M_UNKNOWN;
  info->height = Y4M_UNKNOWN;
  info->interlace = Y4M_UNKNOWN;
  info->framerate = y4m_fps_UNKNOWN;
  info->sampleaspect = y4m_sar_UNKNOWN;
  if (_y4mparam_feature_level < 1) {
    info->chroma = Y4M_CHROMA_420JPEG;
  } else {
    info->chroma = Y4M_UNKNOWN;
  }
  y4m_xtag_clearlist(&(info->x_tags));
}


void y4m_copy_stream_info(y4m_stream_info_t *dest,
			  const y4m_stream_info_t *src)
{
  if ((dest == NULL) || (src == NULL)) return;
  /* copy info */
  dest->width = src->width;
  dest->height = src->height;
  dest->interlace = src->interlace;
  dest->framerate = src->framerate;
  dest->sampleaspect = src->sampleaspect;
  dest->chroma = src->chroma;
  y4m_copy_xtag_list(&(dest->x_tags), &(src->x_tags));
}


void y4m_fini_stream_info(y4m_stream_info_t *info)
{
  if (info == NULL) return;
  y4m_fini_xtag_list(&(info->x_tags));
}


void y4m_si_set_width(y4m_stream_info_t *si, int width)
{
  si->width = width;
}

int y4m_si_get_width(const y4m_stream_info_t *si)
{ return si->width; }

void y4m_si_set_height(y4m_stream_info_t *si, int height)
{
  si->height = height; 
}

int y4m_si_get_height(const y4m_stream_info_t *si)
{ return si->height; }

void y4m_si_set_interlace(y4m_stream_info_t *si, int interlace)
{ si->interlace = interlace; }

int y4m_si_get_interlace(const y4m_stream_info_t *si)
{ return si->interlace; }

void y4m_si_set_framerate(y4m_stream_info_t *si, y4m_ratio_t framerate)
{ si->framerate = framerate; }

y4m_ratio_t y4m_si_get_framerate(const y4m_stream_info_t *si)
{ return si->framerate; }

void y4m_si_set_sampleaspect(y4m_stream_info_t *si, y4m_ratio_t sar)
{ si->sampleaspect = sar; }

y4m_ratio_t y4m_si_get_sampleaspect(const y4m_stream_info_t *si)
{ return si->sampleaspect; }

void y4m_si_set_chroma(y4m_stream_info_t *si, int chroma_mode)
{ si->chroma = chroma_mode; }

int y4m_si_get_chroma(const y4m_stream_info_t *si)
{ return si->chroma; }


int y4m_si_get_plane_count(const y4m_stream_info_t *si)
{
  switch (si->chroma) {
  case Y4M_CHROMA_420JPEG:
  case Y4M_CHROMA_420MPEG2:
  case Y4M_CHROMA_420PALDV:
  case Y4M_CHROMA_444:
  case Y4M_CHROMA_422:
  case Y4M_CHROMA_411:
    return 3;
  case Y4M_CHROMA_MONO:
    return 1;
  case Y4M_CHROMA_444ALPHA:
    return 4;
  default:
    return Y4M_UNKNOWN;
  }
}

int y4m_si_get_plane_width(const y4m_stream_info_t *si, int plane)
{
  switch (plane) {
  case 0:
    return (si->width);
  case 1:
  case 2:
    switch (si->chroma) {
    case Y4M_CHROMA_420JPEG: 
    case Y4M_CHROMA_420MPEG2:
    case Y4M_CHROMA_420PALDV:
      return (si->width) / 2;
    case Y4M_CHROMA_444:
    case Y4M_CHROMA_444ALPHA:
      return (si->width);
    case Y4M_CHROMA_422:
      return (si->width) / 2;
    case Y4M_CHROMA_411:
      return (si->width) / 4;
    default:
      return Y4M_UNKNOWN;
    }
  case 3:
    switch (si->chroma) {
    case Y4M_CHROMA_444ALPHA:
      return (si->width);
    default:
      return Y4M_UNKNOWN;
    }
  default:
    return Y4M_UNKNOWN;
  }
}

int y4m_si_get_plane_height(const y4m_stream_info_t *si, int plane)
{
  switch (plane) {
  case 0:
    return (si->height);
  case 1:
  case 2:
    switch (si->chroma) {
    case Y4M_CHROMA_420JPEG: 
    case Y4M_CHROMA_420MPEG2:
    case Y4M_CHROMA_420PALDV:
      return (si->height) / 2;
    case Y4M_CHROMA_444:
    case Y4M_CHROMA_444ALPHA:
    case Y4M_CHROMA_422:
    case Y4M_CHROMA_411:
      return (si->height);
    default:
      return Y4M_UNKNOWN;
    }
  case 3:
    switch (si->chroma) {
    case Y4M_CHROMA_444ALPHA:
      return (si->height);
    default:
      return Y4M_UNKNOWN;
    }
  default:
    return Y4M_UNKNOWN;
  }
}

int y4m_si_get_plane_length(const y4m_stream_info_t *si, int plane)
{
  int w = y4m_si_get_plane_width(si, plane);
  int h = y4m_si_get_plane_height(si, plane);
  if ((w != Y4M_UNKNOWN) && (h != Y4M_UNKNOWN))
    return (w * h);
  else
    return Y4M_UNKNOWN;
}

int y4m_si_get_framelength(const y4m_stream_info_t *si)
{
  int total = 0;
  int planes = y4m_si_get_plane_count(si);
  int p;
  for (p = 0; p < planes; p++) {
    int plen = y4m_si_get_plane_length(si, p);
    if (plen == Y4M_UNKNOWN) return Y4M_UNKNOWN;
    total += plen;
  }
  return total;
}


y4m_xtag_list_t *y4m_si_xtags(y4m_stream_info_t *si)
{ return &(si->x_tags); }



void y4m_init_frame_info(y4m_frame_info_t *info)
{
  if (info == NULL) return;
  /* init substructures */
  y4m_init_xtag_list(&(info->x_tags));
  /* set defaults */
  y4m_clear_frame_info(info);
}


void y4m_clear_frame_info(y4m_frame_info_t *info)
{
  if (info == NULL) return;
  /* clear/initialize info */
  info->spatial = Y4M_UNKNOWN;
  info->temporal = Y4M_UNKNOWN;
  info->presentation = Y4M_UNKNOWN;
  y4m_xtag_clearlist(&(info->x_tags));
}


void y4m_copy_frame_info(y4m_frame_info_t *dest, const y4m_frame_info_t *src)
{
  if ((dest == NULL) || (src == NULL)) return;
  /* copy info */
  dest->spatial = src->spatial;
  dest->temporal = src->temporal;
  dest->presentation = src->presentation;
  y4m_copy_xtag_list(&(dest->x_tags), &(src->x_tags));
}

void y4m_fini_frame_info(y4m_frame_info_t *info)
{
  if (info == NULL) return;
  y4m_fini_xtag_list(&(info->x_tags));
}


void y4m_fi_set_presentation(y4m_frame_info_t *fi, int pres)
{ fi->presentation = pres; }

int y4m_fi_get_presentation(const y4m_frame_info_t *fi)
{ return fi->presentation; }

void y4m_fi_set_temporal(y4m_frame_info_t *fi, int sampling)
{ fi->temporal = sampling; }

int y4m_fi_get_temporal(const y4m_frame_info_t *fi)
{ return fi->temporal; }

void y4m_fi_set_spatial(y4m_frame_info_t *fi, int sampling)
{ fi->spatial = sampling; }

int y4m_fi_get_spatial(const y4m_frame_info_t *fi)
{ return fi->spatial; }



y4m_xtag_list_t *y4m_fi_xtags(y4m_frame_info_t *fi)
{ return &(fi->x_tags); }


/*************************************************************************
 *
 * Tag parsing 
 *
 *************************************************************************/


/* Parse (the first) old, unofficial X-tag chroma specification,
   and then remove that tag from the X-tag list. */
static int
handle_old_chroma_xtag(y4m_stream_info_t *si)
{
  y4m_xtag_list_t *xtags = y4m_si_xtags(si);
  const char *tag = NULL;
  int n, chroma;

  for (n = y4m_xtag_count(xtags) - 1; n >= 0; n--) {
    tag = y4m_xtag_get(xtags, n);
    if (!strncmp("XYSCSS=", tag, 7)) break;
  }
  if ((tag == NULL) || (n < 0)) return Y4M_UNKNOWN;
  mjpeg_warn("Deprecated X-tag for chroma found in a stream header...");
  mjpeg_warn("...pester someone to upgrade the source's program!");
  /* parse the tag */
  tag += 7;
  if (!strcmp("411", tag))           chroma = Y4M_CHROMA_411;
  else if (!strcmp(tag, "420"))      chroma = Y4M_CHROMA_420JPEG;
  else if (!strcmp(tag, "420MPEG2")) chroma = Y4M_CHROMA_420MPEG2;
  else if (!strcmp(tag, "420PALDV")) chroma = Y4M_CHROMA_420PALDV;
  else if (!strcmp(tag, "420JPEG"))  chroma = Y4M_CHROMA_420JPEG;
  else if (!strcmp(tag, "444"))      chroma = Y4M_CHROMA_444;
  else chroma = Y4M_UNKNOWN;
  /* Remove the 'X' tag so that no one has to worry about it any more. */
  y4m_xtag_remove(xtags, n);
  /* Hmm... what if there are more XYSCSS tags?  Broken is as broken does;
     thank goodness this is temporary code. */
  return chroma;
}




int y4m_parse_stream_tags(char *s, y4m_stream_info_t *i)
{
  char *token, *value;
  char tag;
  int err;

  /* parse fields */
  for (token = strtok(s, Y4M_DELIM); 
       token != NULL; 
       token = strtok(NULL, Y4M_DELIM)) {
    if (token[0] == '\0') continue;   /* skip empty strings */
    tag = token[0];
    value = token + 1;
    switch (tag) {
    case 'W':  /* width */
      i->width = atoi(value);
      if (i->width <= 0) return Y4M_ERR_RANGE;
      break;
    case 'H':  /* height */
      i->height = atoi(value); 
      if (i->height <= 0) return Y4M_ERR_RANGE;
      break;
    case 'F':  /* frame rate (fps) */
      if ((err = y4m_parse_ratio(&(i->framerate), value)) != Y4M_OK)
	return err;
      if (i->framerate.n < 0) return Y4M_ERR_RANGE;
      break;
    case 'I':  /* interlacing */
      switch (value[0]) {
      case 'p':  i->interlace = Y4M_ILACE_NONE; break;
      case 't':  i->interlace = Y4M_ILACE_TOP_FIRST; break;
      case 'b':  i->interlace = Y4M_ILACE_BOTTOM_FIRST; break;
      case 'm':  i->interlace = Y4M_ILACE_MIXED; break;
      case '?':
      default:
	i->interlace = Y4M_UNKNOWN; break;
      }
      break;
    case 'A':  /* sample (pixel) aspect ratio */
      if ((err = y4m_parse_ratio(&(i->sampleaspect), value)) != Y4M_OK)
	return err;
      if (i->sampleaspect.n < 0) return Y4M_ERR_RANGE;
      break;
    case 'C':
      i->chroma = y4m_chroma_parse_keyword(value);
      if (i->chroma == Y4M_UNKNOWN)
	return Y4M_ERR_HEADER;
      break;
    case 'X':  /* 'X' meta-tag */
      if ((err = y4m_xtag_add(&(i->x_tags), token)) != Y4M_OK) return err;
      break;
    default:
      /* possible error on unknown options */
      if (_y4mparam_allow_unknown_tags) {
	/* unknown tags ok:  store in xtag list and warn... */
	if ((err = y4m_xtag_add(&(i->x_tags), token)) != Y4M_OK) return err;
	mjpeg_warn("Unknown stream tag encountered:  '%s'", token);
      } else {
	/* unknown tags are *not* ok */
	return Y4M_ERR_BADTAG;
      }
      break;
    }
  }

  /* If feature_level > 0, then handle and/or remove any old-style XYSCSS
     chroma tags.  The new-style 'C' tag takes precedence, however. */
  if (_y4mparam_feature_level > 0) {
    int xt_chroma = handle_old_chroma_xtag(i);

    if (i->chroma == Y4M_UNKNOWN)
      i->chroma = xt_chroma;
    else if ((xt_chroma != Y4M_UNKNOWN) &&
             (xt_chroma != i->chroma))
      mjpeg_warn("Old chroma X-tag (ignored) does not match new chroma tag.");
  }

  /* Without 'C' tag or any other chroma spec, default to 420jpeg */
  if (i->chroma == Y4M_UNKNOWN) 
    i->chroma = Y4M_CHROMA_420JPEG;

  /* Error checking... */
  /*      - Width and Height are required. */
  if ((i->width == Y4M_UNKNOWN) || (i->height == Y4M_UNKNOWN))
    return Y4M_ERR_HEADER;
  /*      - Non-420 chroma and mixed interlace require level >= 1 */
  if (_y4mparam_feature_level < 1) {
    if ((i->chroma != Y4M_CHROMA_420JPEG) &&
	(i->chroma != Y4M_CHROMA_420MPEG2) &&
	(i->chroma != Y4M_CHROMA_420PALDV))
      return Y4M_ERR_FEATURE;
    if (i->interlace == Y4M_ILACE_MIXED)
      return Y4M_ERR_FEATURE;
  }

  /* ta da!  done. */
  return Y4M_OK;
}



static int y4m_parse_frame_tags(char *s, const y4m_stream_info_t *si,
				y4m_frame_info_t *fi)
{
  char *token, *value;
  char tag;
  int err;

  /* parse fields */
  for (token = strtok(s, Y4M_DELIM); 
       token != NULL; 
       token = strtok(NULL, Y4M_DELIM)) {
    if (token[0] == '\0') continue;   /* skip empty strings */
    tag = token[0];
    value = token + 1;
    switch (tag) {
    case 'I':
      /* frame 'I' tag requires feature level >= 1 */
      if (_y4mparam_feature_level < 1) return Y4M_ERR_FEATURE;
      if (si->interlace != Y4M_ILACE_MIXED) return Y4M_ERR_BADTAG;
      switch (value[0]) {
      case 't':  fi->presentation = Y4M_PRESENT_TOP_FIRST;        break;
      case 'T':  fi->presentation = Y4M_PRESENT_TOP_FIRST_RPT;    break;
      case 'b':  fi->presentation = Y4M_PRESENT_BOTTOM_FIRST;     break;
      case 'B':  fi->presentation = Y4M_PRESENT_BOTTOM_FIRST_RPT; break;
      case '1':  fi->presentation = Y4M_PRESENT_PROG_SINGLE;      break;
      case '2':  fi->presentation = Y4M_PRESENT_PROG_DOUBLE;      break;
      case '3':  fi->presentation = Y4M_PRESENT_PROG_TRIPLE;      break;
      default: 
	return Y4M_ERR_BADTAG;
      }
      switch (value[1]) {
      case 'p':  fi->temporal = Y4M_SAMPLING_PROGRESSIVE; break;
      case 'i':  fi->temporal = Y4M_SAMPLING_INTERLACED;  break;
      default: 
	return Y4M_ERR_BADTAG;
      }
      switch (value[2]) {
      case 'p':  fi->spatial = Y4M_SAMPLING_PROGRESSIVE; break;
      case 'i':  fi->spatial = Y4M_SAMPLING_INTERLACED;  break;
      case '?':  fi->spatial = Y4M_UNKNOWN;              break;
      default: 
	return Y4M_ERR_BADTAG;
      }
      break;
    case 'X':  /* 'X' meta-tag */
      if ((err = y4m_xtag_add(&(fi->x_tags), token)) != Y4M_OK) return err;
      break;
    default:
      /* possible error on unknown options */
      if (_y4mparam_allow_unknown_tags) {
	/* unknown tags ok:  store in xtag list and warn... */
	if ((err = y4m_xtag_add(&(fi->x_tags), token)) != Y4M_OK) return err;
	mjpeg_warn("Unknown frame tag encountered:  '%s'", token);
      } else {
	/* unknown tags are *not* ok */
	return Y4M_ERR_BADTAG;
      }
      break;
    }
  }
  /* error-checking and/or non-mixed defaults */
  switch (si->interlace) {
  case Y4M_ILACE_MIXED:
    /* T and P are required if stream "Im" */
    if ((fi->presentation == Y4M_UNKNOWN) || (fi->temporal == Y4M_UNKNOWN))
      return Y4M_ERR_HEADER;
    /* and S is required if stream is also 4:2:0 */
    if ( ((si->chroma == Y4M_CHROMA_420JPEG) ||
          (si->chroma == Y4M_CHROMA_420MPEG2) ||
          (si->chroma == Y4M_CHROMA_420PALDV)) &&
         (fi->spatial == Y4M_UNKNOWN) )
      return Y4M_ERR_HEADER;
    break;
  case Y4M_ILACE_NONE:
    /* stream "Ip" --> equivalent to frame "I1pp" */
    fi->spatial = Y4M_SAMPLING_PROGRESSIVE;
    fi->temporal = Y4M_SAMPLING_PROGRESSIVE;
    fi->presentation = Y4M_PRESENT_PROG_SINGLE;
    break;
  case Y4M_ILACE_TOP_FIRST:
    /* stream "It" --> equivalent to frame "Itii" */
    fi->spatial = Y4M_SAMPLING_INTERLACED;
    fi->temporal = Y4M_SAMPLING_INTERLACED;
    fi->presentation = Y4M_PRESENT_TOP_FIRST;
    break;
  case Y4M_ILACE_BOTTOM_FIRST:
    /* stream "Ib" --> equivalent to frame "Ibii" */
    fi->spatial = Y4M_SAMPLING_INTERLACED;
    fi->temporal = Y4M_SAMPLING_INTERLACED;
    fi->presentation = Y4M_PRESENT_BOTTOM_FIRST;
    break;
  default:
    /* stream unknown:  then, whatever */
    break;
  }
  /* ta da!  done. */
  return Y4M_OK;
}





/*************************************************************************
 *
 * Read/Write stream header
 *
 *************************************************************************/


int y4m_read_stream_header(int fd, y4m_stream_info_t *i)
{
   char line[Y4M_LINE_MAX];
   char *p;
   int n;
   int err;

  /* start with a clean slate */
  y4m_clear_stream_info(i);
   /* read the header line */
   for (n = 0, p = line; n < Y4M_LINE_MAX; n++, p++) {
     if (read(fd, p, 1) < 1) 
       return Y4M_ERR_SYSTEM;
     if (*p == '\n') {
       *p = '\0';           /* Replace linefeed by end of string */
       break;
     }
   }
   if (n >= Y4M_LINE_MAX)
      return Y4M_ERR_HEADER;
   /* look for keyword in header */
   if (strncmp(line, Y4M_MAGIC, strlen(Y4M_MAGIC)))
    return Y4M_ERR_MAGIC;
   if ((err = y4m_parse_stream_tags(line + strlen(Y4M_MAGIC), i)) != Y4M_OK)
     return err;

   return Y4M_OK;
}



int y4m_write_stream_header(int fd, const y4m_stream_info_t *i)
{
  char s[Y4M_LINE_MAX+1];
  int n;
  int err;
  y4m_ratio_t rate = i->framerate;
  y4m_ratio_t aspect = i->sampleaspect;
  const char *chroma_keyword = y4m_chroma_keyword(i->chroma);

  if ((i->chroma == Y4M_UNKNOWN) || (chroma_keyword == NULL))
    return Y4M_ERR_HEADER;
  if (_y4mparam_feature_level < 1) {
    if ((i->chroma != Y4M_CHROMA_420JPEG) &&
	(i->chroma != Y4M_CHROMA_420MPEG2) &&
	(i->chroma != Y4M_CHROMA_420PALDV))
      return Y4M_ERR_FEATURE;
    if (i->interlace == Y4M_ILACE_MIXED)
      return Y4M_ERR_FEATURE;
  }
  y4m_ratio_reduce(&rate);
  y4m_ratio_reduce(&aspect);
  n = snprintf(s, sizeof(s), "%s W%d H%d F%d:%d I%s A%d:%d C%s",
	       Y4M_MAGIC,
	       i->width,
	       i->height,
	       rate.n, rate.d,
	       (i->interlace == Y4M_ILACE_NONE) ? "p" :
	       (i->interlace == Y4M_ILACE_TOP_FIRST) ? "t" :
	       (i->interlace == Y4M_ILACE_BOTTOM_FIRST) ? "b" :
	       (i->interlace == Y4M_ILACE_MIXED) ? "m" : "?",
	       aspect.n, aspect.d,
	       chroma_keyword
	       );
  if ((n < 0) || (n > Y4M_LINE_MAX)) return Y4M_ERR_HEADER;
  if ((err = y4m_snprint_xtags(s + n, sizeof(s) - n - 1, &(i->x_tags))) 
      != Y4M_OK) 
    return err;
  /* non-zero on error */
  return (y4m_write(fd, s, strlen(s)) ? Y4M_ERR_SYSTEM : Y4M_OK);
}





/*************************************************************************
 *
 * Read/Write frame header
 *
 *************************************************************************/

int y4m_read_frame_header(int fd,
			  const y4m_stream_info_t *si,
			  y4m_frame_info_t *fi)
{
  char line[Y4M_LINE_MAX];
  char *p;
  int n;
  ssize_t remain;
  
  /* start with a clean slate */
  y4m_clear_frame_info(fi);
  /* This is more clever than read_stream_header...
     Try to read "FRAME\n" all at once, and don't try to parse
     if nothing else is there...
  */
  remain = y4m_read(fd, line, sizeof(Y4M_FRAME_MAGIC)-1+1); /* -'\0', +'\n' */
  if (remain < 0) return Y4M_ERR_SYSTEM;
  if (remain > 0) {
    /* A clean EOF should end exactly at a frame-boundary */
    if (remain == sizeof(Y4M_FRAME_MAGIC))
      return Y4M_ERR_EOF;
    else
      return Y4M_ERR_BADEOF;
  }
  if (strncmp(line, Y4M_FRAME_MAGIC, sizeof(Y4M_FRAME_MAGIC)-1))
    return Y4M_ERR_MAGIC;
  if (line[sizeof(Y4M_FRAME_MAGIC)-1] == '\n')
    return Y4M_OK; /* done -- no tags:  that was the end-of-line. */

  if (line[sizeof(Y4M_FRAME_MAGIC)-1] != Y4M_DELIM[0]) {
    return Y4M_ERR_MAGIC; /* wasn't a space -- what was it? */
  }

  /* proceed to get the tags... (overwrite the magic) */
  for (n = 0, p = line; n < Y4M_LINE_MAX; n++, p++) {
    if (y4m_read(fd, p, 1))
      return Y4M_ERR_SYSTEM;
    if (*p == '\n') {
      *p = '\0';           /* Replace linefeed by end of string */
      break;
    }
  }
  if (n >= Y4M_LINE_MAX) return Y4M_ERR_HEADER;
  /* non-zero on error */
  return y4m_parse_frame_tags(line, si, fi);
}


int y4m_write_frame_header(int fd,
			   const y4m_stream_info_t *si,
			   const y4m_frame_info_t *fi)
{
  char s[Y4M_LINE_MAX+1];
  int n, err;

  if (si->interlace == Y4M_ILACE_MIXED) {
    if (_y4mparam_feature_level < 1) return Y4M_ERR_FEATURE;
    n = snprintf(s, sizeof(s), "%s I%c%c%c", Y4M_FRAME_MAGIC,
		 (fi->presentation == Y4M_PRESENT_TOP_FIRST)        ? 't' :
		 (fi->presentation == Y4M_PRESENT_TOP_FIRST_RPT)    ? 'T' :
		 (fi->presentation == Y4M_PRESENT_BOTTOM_FIRST)     ? 'b' :
		 (fi->presentation == Y4M_PRESENT_BOTTOM_FIRST_RPT) ? 'B' :
		 (fi->presentation == Y4M_PRESENT_PROG_SINGLE) ? '1' :
		 (fi->presentation == Y4M_PRESENT_PROG_DOUBLE) ? '2' :
		 (fi->presentation == Y4M_PRESENT_PROG_TRIPLE) ? '3' :
		 '?',
		 (fi->temporal == Y4M_SAMPLING_PROGRESSIVE) ? 'p' :
		 (fi->temporal == Y4M_SAMPLING_INTERLACED)  ? 'i' :
		 '?',
		 (fi->spatial == Y4M_SAMPLING_PROGRESSIVE) ? 'p' :
		 (fi->spatial == Y4M_SAMPLING_INTERLACED)  ? 'i' :
		 '?'
		 );
  } else {
    n = snprintf(s, sizeof(s), "%s", Y4M_FRAME_MAGIC);
  }
  
  if ((n < 0) || (n > Y4M_LINE_MAX)) return Y4M_ERR_HEADER;
  if ((err = y4m_snprint_xtags(s + n, sizeof(s) - n - 1, &(fi->x_tags))) 
      != Y4M_OK) 
    return err;
  /* non-zero on error */
  return (y4m_write(fd, s, strlen(s)) ? Y4M_ERR_SYSTEM : Y4M_OK);
}



/*************************************************************************
 *
 * Read/Write entire frame
 *
 *************************************************************************/

int y4m_read_frame_data(int fd, const y4m_stream_info_t *si, 
                        y4m_frame_info_t *fi, uint8_t * const *frame)
{
  int planes = y4m_si_get_plane_count(si);
  int p;
  
  /* Read each plane */
  for (p = 0; p < planes; p++) {
    int w = y4m_si_get_plane_width(si, p);
    int h = y4m_si_get_plane_height(si, p);
    if (y4m_read(fd, frame[p], w*h)) return Y4M_ERR_SYSTEM;
  }
  return Y4M_OK;
}



int y4m_read_frame(int fd, const y4m_stream_info_t *si, 
		   y4m_frame_info_t *fi, uint8_t * const *frame)
{
  int err;
  
  /* Read frame header */
  if ((err = y4m_read_frame_header(fd, si, fi)) != Y4M_OK) return err;
  /* Read date */
  return y4m_read_frame_data(fd, si, fi, frame);
}




int y4m_write_frame(int fd, const y4m_stream_info_t *si, 
		    const y4m_frame_info_t *fi, uint8_t * const *frame)
{
  int planes = y4m_si_get_plane_count(si);
  int err, p;

  /* Write frame header */
  if ((err = y4m_write_frame_header(fd, si, fi)) != Y4M_OK) return err;
  /* Write each plane */
  for (p = 0; p < planes; p++) {
    int w = y4m_si_get_plane_width(si, p);
    int h = y4m_si_get_plane_height(si, p);
    if (y4m_write(fd, frame[p], w*h)) return Y4M_ERR_SYSTEM;
  }
  return Y4M_OK;
}



/*************************************************************************
 *
 * Read/Write entire frame, (de)interleaved (to)from two separate fields
 *
 *************************************************************************/


int y4m_read_fields_data(int fd, const y4m_stream_info_t *si,
                         y4m_frame_info_t *fi,
                         uint8_t * const *upper_field, 
                         uint8_t * const *lower_field)
{
  int p;
  int planes = y4m_si_get_plane_count(si);
  const int maxrbuf=32*1024;
  uint8_t *rbuf=_y4m_alloc(maxrbuf);
  int rbufpos=0,rbuflen=0;
  
  /* Read each plane */
  for (p = 0; p < planes; p++) {
    uint8_t *dsttop = upper_field[p];
    uint8_t *dstbot = lower_field[p];
    int height = y4m_si_get_plane_height(si, p);
    int width = y4m_si_get_plane_width(si, p);
    int y;
    /* alternately read one line into each field */
    for (y = 0; y < height; y += 2) {
      if( width*2 >= maxrbuf ) {
        if (y4m_read(fd, dsttop, width)) goto y4merr;
        if (y4m_read(fd, dstbot, width)) goto y4merr;
      } else {
        if( rbufpos==rbuflen ) {
          rbuflen=(height-y)*width;
          if( rbuflen>maxrbuf )
            rbuflen=maxrbuf-maxrbuf%(2*width);
          if( y4m_read(fd,rbuf,rbuflen) )
            goto y4merr;
          rbufpos=0;
        }
            
        memcpy(dsttop,rbuf+rbufpos,width); rbufpos+=width;
        memcpy(dstbot,rbuf+rbufpos,width); rbufpos+=width;
      }
      dsttop+=width;
      dstbot+=width;
    }
  }
  _y4m_free(rbuf);
  return Y4M_OK;

 y4merr:
  _y4m_free(rbuf);
  return Y4M_ERR_SYSTEM;
}


int y4m_read_fields(int fd, const y4m_stream_info_t *si, y4m_frame_info_t *fi,
                    uint8_t * const *upper_field, 
                    uint8_t * const *lower_field)
{
  int err;
  /* Read frame header */
  if ((err = y4m_read_frame_header(fd, si, fi)) != Y4M_OK) return err;
  /* Read data */
  return y4m_read_fields_data(fd, si, fi, upper_field, lower_field);
}



int y4m_write_fields(int fd, const y4m_stream_info_t *si,
		     const y4m_frame_info_t *fi,
		     uint8_t * const *upper_field, 
		     uint8_t * const *lower_field)
{
  int p, err;
  int planes = y4m_si_get_plane_count(si);
  int numwbuf=0;
  const int maxwbuf=32*1024;
  uint8_t *wbuf;
  
  /* Write frame header */
  if ((err = y4m_write_frame_header(fd, si, fi)) != Y4M_OK) return err;
  /* Write each plane */
  wbuf=_y4m_alloc(maxwbuf);
  for (p = 0; p < planes; p++) {
    uint8_t *srctop = upper_field[p];
    uint8_t *srcbot = lower_field[p];
    int height = y4m_si_get_plane_height(si, p);
    int width = y4m_si_get_plane_width(si, p);
    int y;
    /* alternately write one line from each field */
    for (y = 0; y < height; y += 2) {
      if( width*2 >= maxwbuf ) {
        if (y4m_write(fd, srctop, width)) goto y4merr;
        if (y4m_write(fd, srcbot, width)) goto y4merr;
      } else {
        if (numwbuf + 2 * width > maxwbuf) {
          if(y4m_write(fd, wbuf, numwbuf)) goto y4merr;
          numwbuf=0;
        }

        memcpy(wbuf+numwbuf,srctop,width); numwbuf += width;
        memcpy(wbuf+numwbuf,srcbot,width); numwbuf += width;
      }
      srctop  += width;
      srcbot  += width;
    }
  }
  if( numwbuf )
    if( y4m_write(fd, wbuf, numwbuf) )
      goto y4merr;
  _y4m_free(wbuf);
  return Y4M_OK;

 y4merr:
  _y4m_free(wbuf);
  return Y4M_ERR_SYSTEM;
}


/*************************************************************************
 *
 * Handy logging of stream info
 *
 *************************************************************************/

void y4m_log_stream_info(log_level_t level, const char *prefix,
			 const y4m_stream_info_t *i)
{
  char s[256];

  snprintf(s, sizeof(s), "  frame size:  ");
  if (i->width == Y4M_UNKNOWN)
    snprintf(s+strlen(s), sizeof(s)-strlen(s), "(?)x");
  else
    snprintf(s+strlen(s), sizeof(s)-strlen(s), "%dx", i->width);
  if (i->height == Y4M_UNKNOWN)
    snprintf(s+strlen(s), sizeof(s)-strlen(s), "(?) pixels ");
  else
    snprintf(s+strlen(s), sizeof(s)-strlen(s), "%d pixels ", i->height);
  {
    int framelength = y4m_si_get_framelength(i);
    if (framelength == Y4M_UNKNOWN)
      snprintf(s+strlen(s), sizeof(s)-strlen(s), "(? bytes)");
    else
      snprintf(s+strlen(s), sizeof(s)-strlen(s), "(%d bytes)", framelength);
    mjpeg_log(level, "%s%s", prefix, s);
  }
  {
    const char *desc = y4m_chroma_description(i->chroma);
    if (desc == NULL) desc = "unknown!";
    mjpeg_log(level, "%s      chroma:  %s", prefix, desc);
  }
  if ((i->framerate.n == 0) && (i->framerate.d == 0))
    mjpeg_log(level, "%s  frame rate:  ??? fps", prefix);
  else
    mjpeg_log(level, "%s  frame rate:  %d/%d fps (~%f)", prefix,
	      i->framerate.n, i->framerate.d, 
	      (double) i->framerate.n / (double) i->framerate.d);
  mjpeg_log(level, "%s   interlace:  %s", prefix,
	  (i->interlace == Y4M_ILACE_NONE) ? "none/progressive" :
	  (i->interlace == Y4M_ILACE_TOP_FIRST) ? "top-field-first" :
	  (i->interlace == Y4M_ILACE_BOTTOM_FIRST) ? "bottom-field-first" :
	  (i->interlace == Y4M_ILACE_MIXED) ? "mixed-mode" :
	  "anyone's guess");
  if ((i->sampleaspect.n == 0) && (i->sampleaspect.d == 0))
    mjpeg_log(level, "%ssample aspect ratio:  ?:?", prefix);
  else
    mjpeg_log(level, "%ssample aspect ratio:  %d:%d", prefix,
	      i->sampleaspect.n, i->sampleaspect.d);
}


/*************************************************************************
 *
 * Convert error code to string
 *
 *************************************************************************/

const char *y4m_strerr(int err)
{
  switch (err) {
  case Y4M_OK:          return "no error";
  case Y4M_ERR_RANGE:   return "parameter out of range";
  case Y4M_ERR_SYSTEM:  return "system error (failed read/write)";
  case Y4M_ERR_HEADER:  return "bad stream or frame header";
  case Y4M_ERR_BADTAG:  return "unknown header tag";
  case Y4M_ERR_MAGIC:   return "bad header magic";
  case Y4M_ERR_XXTAGS:  return "too many xtags";
  case Y4M_ERR_EOF:     return "end-of-file";
  case Y4M_ERR_BADEOF:  return "stream ended unexpectedly (EOF)";
  case Y4M_ERR_FEATURE: return "stream requires unsupported features";
  default: 
    return "unknown error code";
  }
}


/*************************************************************************
 *
 * Chroma subsampling stuff
 *
 *************************************************************************/

y4m_ratio_t y4m_chroma_ss_x_ratio(int chroma_mode)
{
  y4m_ratio_t r;
  switch (chroma_mode) {
  case Y4M_CHROMA_444ALPHA:
  case Y4M_CHROMA_444:
  case Y4M_CHROMA_MONO:
    r.n = 1; r.d = 1; break;
  case Y4M_CHROMA_420JPEG:
  case Y4M_CHROMA_420MPEG2:
  case Y4M_CHROMA_420PALDV:
  case Y4M_CHROMA_422:
    r.n = 1; r.d = 2; break;
  case Y4M_CHROMA_411:
    r.n = 1; r.d = 4; break;
  default:
    r.n = 0; r.d = 0;
  }
  return r;
}

y4m_ratio_t y4m_chroma_ss_y_ratio(int chroma_mode)
{
  y4m_ratio_t r;
  switch (chroma_mode) {
  case Y4M_CHROMA_444ALPHA:
  case Y4M_CHROMA_444:
  case Y4M_CHROMA_MONO:
  case Y4M_CHROMA_422:
  case Y4M_CHROMA_411:
    r.n = 1; r.d = 1; break;
  case Y4M_CHROMA_420JPEG:
  case Y4M_CHROMA_420MPEG2:
  case Y4M_CHROMA_420PALDV:
    r.n = 1; r.d = 2; break;
  default:
    r.n = 0; r.d = 0;
  }
  return r;
}


#if 0  /* unfinished work here */
y4m_ratio_t y4m_chroma_ss_x_offset(int chroma_mode, int field, int plane)
{
  y4m_ratio_t r;
  switch (chroma_mode) {
  case Y4M_CHROMA_444ALPHA:
  case Y4M_CHROMA_444:
  case Y4M_CHROMA_MONO:
  case Y4M_CHROMA_422:
  case Y4M_CHROMA_411:
    r.n = 0; r.d = 1; break;
  case Y4M_CHROMA_420JPEG:
  case Y4M_CHROMA_420MPEG2:
  case Y4M_CHROMA_420PALDV:
    r.n = 1; r.d = 2; break;
  default:
    r.n = 0; r.d = 0;
  }
  return r;
}

y4m_ratio_t y4m_chroma_ss_y_offset(int chroma_mode, int field, int plane);
{
  y4m_ratio_t r;
  switch (chroma_mode) {
  case Y4M_CHROMA_444ALPHA:
  case Y4M_CHROMA_444:
  case Y4M_CHROMA_MONO:
  case Y4M_CHROMA_422:
  case Y4M_CHROMA_411:
    r.n = 0; r.d = 1; break;
  case Y4M_CHROMA_420JPEG:
  case Y4M_CHROMA_420MPEG2:
  case Y4M_CHROMA_420PALDV:
    r.n = 1; r.d = 2; break;
  default:
    r.n = 0; r.d = 0;
  }
  return r;
}
#endif



int y4m_chroma_parse_keyword(const char *s)
{
  if (!strcasecmp("420jpeg", s))
    return Y4M_CHROMA_420JPEG;
  else if (!strcasecmp("420mpeg2", s))
    return Y4M_CHROMA_420MPEG2;
  else if (!strcasecmp("420paldv", s))
    return Y4M_CHROMA_420PALDV;
  else if (!strcasecmp("444", s))
    return Y4M_CHROMA_444;
  else if (!strcasecmp("422", s))
    return Y4M_CHROMA_422;
  else if (!strcasecmp("411", s))
    return Y4M_CHROMA_411;
  else if (!strcasecmp("mono", s))
    return Y4M_CHROMA_MONO;
  else if (!strcasecmp("444alpha", s))
    return Y4M_CHROMA_444ALPHA;
  else
    return Y4M_UNKNOWN;
}


const char *y4m_chroma_keyword(int chroma_mode)
{
  switch (chroma_mode) {
  case Y4M_CHROMA_420JPEG:  return "420jpeg";
  case Y4M_CHROMA_420MPEG2: return "420mpeg2";
  case Y4M_CHROMA_420PALDV: return "420paldv";
  case Y4M_CHROMA_444:      return "444";
  case Y4M_CHROMA_422:      return "422";
  case Y4M_CHROMA_411:      return "411";
  case Y4M_CHROMA_MONO:     return "mono";
  case Y4M_CHROMA_444ALPHA: return "444alpha";
  default:
    return NULL;
  }
}  


const char *y4m_chroma_description(int chroma_mode)
{           
  switch (chroma_mode) {
  case Y4M_CHROMA_420JPEG:  return "4:2:0 JPEG/MPEG-1 (interstitial)";
  case Y4M_CHROMA_420MPEG2: return "4:2:0 MPEG-2 (horiz. cositing)";
  case Y4M_CHROMA_420PALDV: return "4:2:0 PAL-DV (altern. siting)";
  case Y4M_CHROMA_444:      return "4:4:4 (no subsampling)";
  case Y4M_CHROMA_422:      return "4:2:2 (horiz. cositing)";
  case Y4M_CHROMA_411:      return "4:1:1 (horiz. cositing)";
  case Y4M_CHROMA_MONO:     return "luma plane only";
  case Y4M_CHROMA_444ALPHA: return "4:4:4 with alpha channel";
  default:
    return NULL;
  }
}
