/*****************************************************************************
 * tags.c:
 *****************************************************************************
 * Copyright (C) 2007 libmkv
 *
 * Authors: John A. Stebbins
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

int mk_initTags(mk_Writer *w)
{
	mk_Context *targets;

	if (w->tags == NULL) {
		/* Tags */
		if ((w->tags = mk_createContext(w, w->root, MATROSKA_ID_TAGS)) == NULL)
			return -1;
		/* Tag */
		if ((w->tag = mk_createContext(w, w->tags, MATROSKA_ID_TAG)) == NULL)
			return -1;
		/* Targets */
		if ((targets = mk_createContext(w, w->tag, MATROSKA_ID_TARGETS)) == NULL)
			return -1;
		/* TargetTypeValue */
		CHECK(mk_writeUInt(targets, MATROSKA_ID_TARGETTYPEVALUE, MK_TARGETTYPE_MOVIE));	/* Album/Movie/Episode */
		CHECK(mk_closeContext(targets, 0));
	}

	return 0;
}

int mk_createTagSimple(mk_Writer * w, char *tag_id, char *value)
{
	mk_Context *simple;

	CHECK(mk_initTags(w));

	/* SimpleTag */
	if ((simple = mk_createContext(w, w->tag, MATROSKA_ID_SIMPLETAG)) == NULL)
		return -1;

	CHECK(mk_writeStr(simple, MATROSKA_ID_TAGNAME, tag_id));	/* TagName */
	CHECK(mk_writeStr(simple, MATROSKA_ID_TAGSTRING, value));	/* TagString */
	CHECK(mk_closeContext(simple, 0));

	return 0;
}

int mk_createTagSimpleBin(mk_Writer * w, char *tag_id, const void *data, unsigned size)
{
	mk_Context *simple;
	
	CHECK(mk_initTags(w));
	
	/* SimpleTag */
	if ((simple = mk_createContext(w, w->tag, MATROSKA_ID_SIMPLETAG)) == NULL)
		return -1;

	/* TagName */
	CHECK(mk_writeStr(simple, MATROSKA_ID_TAGNAME, tag_id));
	/* TagBinary */
	CHECK(mk_writeBin(simple, MATROSKA_ID_TAGBINARY, data, size));
	CHECK(mk_closeContext(simple, 0));

	return 0;
}

int mk_writeTags(mk_Writer * w)
{
	if ((w->tags == NULL) || (w->tag == NULL))
		return -1;
	CHECK(mk_closeContext(w->tag, 0));
	CHECK(mk_closeContext(w->tags, 0));

	return 0;
}
