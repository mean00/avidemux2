/*****************************************************************************
 * attachments.c:
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
#include "ADM_coreConfig.h"
#include "libmkv.h"
#include "matroska.h"

int mk_createAttachment(
	mk_Writer * w, 
	char *name, 
	char *description, 
	char *mime, 
	const void *data, 
	unsigned size)
{
	mk_Context *attach;
	unsigned long file_uid;

	if ((data == NULL) || (size == 0))
		return -1;
	
	/*
	 * Generate a random UID for this Attachment.
	 * NOTE: This probably should be a CRC32 of file data.
	 *        In place of being completely random.
	 */
	file_uid = random();

	if (w->attachments == NULL) {
		/* Attachments */
		if ((w->attachments = mk_createContext(w, w->root, MATROSKA_ID_ATTACHMENTS)) == NULL)
			return -1;
	}
	/* AttachedFile */
	if ((attach = mk_createContext(w, w->attachments, MATROSKA_ID_ATTACHEDFILE)) == NULL)
		return -1;

	/* FileName */
	CHECK(mk_writeStr(attach, MATROSKA_ID_FILENAME, name));
	if ((description != NULL) && (strlen(description) > 0)) {
		/* FileDescription */
		CHECK(mk_writeStr(attach, MATROSKA_ID_FILEDESCRIPTION, description));
	}
	/* FileMimeType */
	CHECK(mk_writeStr(attach, MATROSKA_ID_FILEMIMETYPE, mime));
	/* FileUID */
	CHECK(mk_writeUInt(attach, MATROSKA_ID_FILEUID, file_uid));
	/* FileData */
	CHECK(mk_writeBin(attach, MATROSKA_ID_FILEDATA, data, size));
	CHECK(mk_closeContext(attach, 0));

	return 0;
}

int mk_writeAttachments(mk_Writer * w)
{
	if (w->attachments == NULL)
		return -1;
	CHECK(mk_closeContext(w->attachments, 0));

	return 0;
}
