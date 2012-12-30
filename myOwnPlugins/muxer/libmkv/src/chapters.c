/*****************************************************************************
 * chapters.c:
 *****************************************************************************
 * Copyright (C) 2007 libmkv
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
#include "config.h"
#include "libmkv.h"
#include "matroska.h"

int mk_createChapterSimple(mk_Writer *w, uint64_t start, uint64_t end,
						   char *name)
{
	mk_Context *ca, *cd;
	unsigned long chapter_uid;

	/*
	 * Generate a random UID for this Chapter.
	 * NOTE: This probably should be a CRC32 of some unique chapter information.
	 *        In place of being completely random.
	 */
	chapter_uid = random();

	if (w->chapters == NULL) {
		unsigned long edition_uid;
		edition_uid = random();

		/* Chapters */
		if ((w->chapters = mk_createContext(w, w->root, MATROSKA_ID_CHAPTERS)) == NULL)
			return -1;
		/* EditionEntry */
		if ((w->edition_entry = mk_createContext(w, w->chapters, MATROSKA_ID_EDITIONENTRY)) == NULL)
			return -1;
		/* EditionUID - See note above about Chapter UID. */
		CHECK(mk_writeUInt(w->edition_entry, MATROSKA_ID_EDITIONUID, edition_uid));
		/* EditionFlagDefault - This is set to 1 to force this Edition to be the default one. */
		CHECK(mk_writeUInt(w->edition_entry, MATROSKA_ID_EDITIONFLAGDEFAULT, 1));
		/* EditionFlagOrdered - Force simple chapters. */
		CHECK(mk_writeUInt(w->edition_entry, MATROSKA_ID_EDITIONFLAGORDERED, 0));
	}
	/* ChapterAtom */
	if ((ca = mk_createContext(w, w->edition_entry, MATROSKA_ID_CHAPTERATOM)) == NULL)
		return -1;
	CHECK(mk_writeUInt(ca, MATROSKA_ID_CHAPTERUID, chapter_uid));	/* ChapterUID */
	CHECK(mk_writeUInt(ca, MATROSKA_ID_CHAPTERTIMESTART, start));	/* ChapterTimeStart */
	if (end != start)			/* Only create a StartTime if chapter length would be 0. */
		CHECK(mk_writeUInt(ca, MATROSKA_ID_CHAPTERTIMEEND, end));	/* ChapterTimeEnd */
	if (name != NULL) {
		/* ChapterDisplay */
		if ((cd = mk_createContext(w, ca, MATROSKA_ID_CHAPTERDISPLAY)) == NULL)
			return -1;
		CHECK(mk_writeStr(cd, MATROSKA_ID_CHAPTERSTRING, name));	/* ChapterString */
		CHECK(mk_closeContext(cd, 0));
	}
	CHECK(mk_closeContext(ca, 0));

	return 0;
}

int mk_writeChapters(mk_Writer *w)
{
	if ((w->chapters == NULL) || (w->edition_entry == NULL))
		return -1;
	CHECK(mk_closeContext(w->edition_entry, 0));
	CHECK(mk_closeContext(w->chapters, 0));

	return 0;
}
