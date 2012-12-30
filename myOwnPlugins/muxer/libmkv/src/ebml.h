/*****************************************************************************
 * ebml.h:
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
#ifndef _EBML_H
#define _EBML_H 1

/* Copied from ffmpeg */
/* EBML version supported */
#define EBML_VERSION 1

/* top-level master-IDs */
#define EBML_ID_HEADER              0x1A45DFA3	/* sub-elements */

/* IDs in the HEADER master */
#define EBML_ID_EBMLVERSION         0x4286	/* u-integer */
#define EBML_ID_EBMLREADVERSION     0x42F7	/* u-integer */
#define EBML_ID_EBMLMAXIDLENGTH     0x42F2	/* u-integer */
#define EBML_ID_EBMLMAXSIZELENGTH   0x42F3	/* u-integer */
#define EBML_ID_DOCTYPE             0x4282	/* string */
#define EBML_ID_DOCTYPEVERSION      0x4287	/* u-integer */
#define EBML_ID_DOCTYPEREADVERSION  0x4285	/* u-integer */

/* general EBML types */
#define EBML_ID_VOID                0xEC	/* binary */
/* Above copied from ffmpeg */

typedef struct mk_Context_s mk_Context;

struct mk_Context_s {
	mk_Context *next, **prev, *parent;
	mk_Writer *owner;
	unsigned id;

	void *data;
	unsigned d_cur, d_max;
};

/* Contexts */
mk_Context *mk_createContext(mk_Writer *w, mk_Context *parent,
							 unsigned id);
int mk_appendContextData(mk_Context *c, const void *data, uint64_t size);
int mk_flushContextID(mk_Context *c);
int mk_flushContextData(mk_Context *c);
int mk_closeContext(mk_Context *c, int64_t *off);
void mk_destroyContexts(mk_Writer *w);
/* Contexts */

/* EBML */
int mk_writeID(mk_Context *c, unsigned id);
int mk_writeSize(mk_Context *c, uint64_t size);
int mk_writeSSize(mk_Context *c, int64_t size);
int mk_writeStr(mk_Context *c, unsigned id, const char *str);
int mk_writeBin(mk_Context *c, unsigned id, const void *data,
				unsigned size);
int mk_writeUInt(mk_Context *c, unsigned id, uint64_t ui);
int mk_writeSInt(mk_Context *c, unsigned id, int64_t si);
int mk_writeFloatRaw(mk_Context *c, float f);
int mk_writeFloat(mk_Context *c, unsigned id, float f);
int mk_writeVoid(mk_Context *c, uint64_t length);
unsigned mk_ebmlSizeSize(uint64_t s);
unsigned mk_ebmlSIntSize(int64_t si);
unsigned mk_ebmlUIntSize(uint64_t ui);
int mk_writeEbmlHeader(mk_Writer *w, const char *doctype,
					   uint64_t doctype_version,
					   uint64_t doctype_readversion);
/* EBML */

#endif							/* _EBML_H */
