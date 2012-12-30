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
#include "ADM_coreConfig.h"
#include "libmkv.h"
#include "matroska.h"

mk_Context *mk_createContext(mk_Writer *w, mk_Context *parent,
							 unsigned id)
{
	mk_Context *c;

	if (w->freelist) {
		c = w->freelist;
		w->freelist = w->freelist->next;
	} else {
		c = malloc(sizeof(*c));
		memset(c, 0, sizeof(*c));
	}

	if (c == NULL)
		return NULL;

	c->parent = parent;
	c->owner = w;
	c->id = id;

	if (c->owner->actlist)
		c->owner->actlist->prev = &c->next;
	c->next = c->owner->actlist;
	c->prev = &c->owner->actlist;
	c->owner->actlist = c;

	return c;
}

int mk_appendContextData(mk_Context *c, const void *data, uint64_t size)
{
	uint64_t ns = c->d_cur + size;

	if (ns > c->d_max) {
		void *dp;
		uint64_t dn = c->d_max ? c->d_max << 1 : 16;
		while (ns > dn)
			dn <<= 1;

		dp = realloc(c->data, dn);
		if (dp == NULL)
			return -1;

		c->data = dp;
		c->d_max = dn;
	}

	memcpy((char *) c->data + c->d_cur, data, size);

	c->d_cur = ns;

	return 0;
}

int mk_writeEbmlHeader(mk_Writer *w, const char *doctype,
					   uint64_t doctype_version,
					   uint64_t doctype_readversion)
{
	mk_Context *c = NULL;

	if ((c = mk_createContext(w, w->root, EBML_ID_HEADER)) == NULL)	/* EBML */
		return -1;
	CHECK(mk_writeUInt(c, EBML_ID_EBMLVERSION, EBML_VERSION));	/* EBMLVersion */
	CHECK(mk_writeUInt(c, EBML_ID_EBMLREADVERSION, EBML_VERSION));	/* EBMLReadVersion */
	CHECK(mk_writeUInt(c, EBML_ID_EBMLMAXIDLENGTH, 4));	/* EBMLMaxIDLength */
	CHECK(mk_writeUInt(c, EBML_ID_EBMLMAXSIZELENGTH, 8));	/* EBMLMaxSizeLength */
	CHECK(mk_writeStr(c, EBML_ID_DOCTYPE, doctype));	/* DocType */
	CHECK(mk_writeUInt(c, EBML_ID_DOCTYPEVERSION, doctype_version));	/* DocTypeVersion */
	/* DocTypeReadversion */
	CHECK(mk_writeUInt(c, EBML_ID_DOCTYPEREADVERSION, doctype_readversion));
	CHECK(mk_closeContext(c, 0));

	return 0;
}

int mk_writeID(mk_Context *c, unsigned id)
{
	unsigned char c_id[4] = { id >> 24, id >> 16, id >> 8, id };

	if (c_id[0])
		return mk_appendContextData(c, c_id, 4);
	if (c_id[1])
		return mk_appendContextData(c, c_id + 1, 3);
	if (c_id[2])
		return mk_appendContextData(c, c_id + 2, 2);
	return mk_appendContextData(c, c_id + 3, 1);
}

int mk_writeSize(mk_Context *c, uint64_t size)
{
	unsigned char c_size[8] = { 0x01, size >> 48, size >> 40, size >> 32,
								size >> 24, size >> 16,	size >> 8, size };

	if (size < 0x7fll) {
		c_size[7] |= 0x80;
		return mk_appendContextData(c, c_size + 7, 1);
	}
	if (size < 0x3fffll) {
		c_size[6] |= 0x40;
		return mk_appendContextData(c, c_size + 6, 2);
	}
	if (size < 0x1fffffll) {
		c_size[5] |= 0x20;
		return mk_appendContextData(c, c_size + 5, 3);
	}
	if (size < 0x0fffffffll) {
		c_size[4] |= 0x10;
		return mk_appendContextData(c, c_size + 4, 4);
	}
	if (size < 0x07ffffffffll) {
		c_size[3] |= 0x08;
		return mk_appendContextData(c, c_size + 3, 5);
	}
	if (size < 0x03ffffffffffll) {
		c_size[2] |= 0x04;
		return mk_appendContextData(c, c_size + 2, 6);
	}
	if (size < 0x01ffffffffffffll) {
		c_size[1] |= 0x02;
		return mk_appendContextData(c, c_size + 1, 7);
	}
	return mk_appendContextData(c, c_size, 8);
}

int mk_writeSSize(mk_Context *c, int64_t size)
{
	uint64_t u_size = (uint64_t) llabs(size);
	/* Need to shift by one below so ebmlSizeSize returns the correct size. */
	unsigned size_size = mk_ebmlSizeSize(u_size << 1);

	switch (size_size) {
		case 1:
			size += 0x3fll;
			break;
		case 2:
			size += 0x1fffll;
			break;
		case 3:
			size += 0x0fffffll;
			break;
		case 4:
			size += 0x07ffffffll;
			break;
		case 5:
			size += 0x03ffffffffll;
			break;
		case 6:
			size += 0x01ffffffffffll;
			break;
		case 7:
			size += 0x00ffffffffffffll;
			break;
		default:				/* Matroska currently doesn't support any int > 56-bit. */
			return -1;
	}

	return mk_writeSize(c, size);
}

int mk_flushContextID(mk_Context *c)
{
	unsigned char size_undf[8] = { 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	if (c->id == 0)
		return 0;

	CHECK(mk_writeID(c->parent, c->id));
	CHECK(mk_appendContextData(c->parent, &size_undf, 8));

	c->id = 0;

	return 0;
}

int mk_flushContextData(mk_Context *c)
{
	mk_Writer *w = c->owner;

	if (c->d_cur == 0)
		return 0;

	if (c->parent) {
		CHECK(mk_appendContextData(c->parent, c->data, c->d_cur));
	} else {
		if (fwrite(c->data, c->d_cur, 1, w->fp) != 1)
			return -1;
		w->f_pos += c->d_cur;
		if (w->f_pos > w->f_eof)
			w->f_eof = w->f_pos;
	}

	c->d_cur = 0;

	return 0;
}

int mk_closeContext(mk_Context *c, int64_t *off)
{
	if (c->id) {
		CHECK(mk_writeID(c->parent, c->id));
		CHECK(mk_writeSize(c->parent, c->d_cur));
	}

	if (c->parent && off != NULL)
		*off += c->parent->d_cur;

	CHECK(mk_flushContextData(c));

	if (c->next)
		c->next->prev = c->prev;
	*(c->prev) = c->next;
	c->next = c->owner->freelist;
	c->owner->freelist = c;

	return 0;
}

void mk_destroyContexts(mk_Writer *w)
{
	mk_Context *cur, *next;

	for (cur = w->freelist; cur; cur = next) {
		next = cur->next;
		free(cur->data);
		free(cur);
	}

	for (cur = w->actlist; cur; cur = next) {
		next = cur->next;
		free(cur->data);
		free(cur);
	}

	w->freelist = w->actlist = w->root = NULL;
}

int mk_writeStr(mk_Context *c, unsigned id, const char *str)
{
	size_t len = strlen(str);

	CHECK(mk_writeID(c, id));
	CHECK(mk_writeSize(c, len));
	CHECK(mk_appendContextData(c, str, len));
	return 0;
}

int mk_writeBin(mk_Context *c, unsigned id, const void *data,
				unsigned size)
{
	CHECK(mk_writeID(c, id));
	CHECK(mk_writeSize(c, size));
	CHECK(mk_appendContextData(c, data, size));
	return 0;
}

int mk_writeUInt(mk_Context *c, unsigned id, uint64_t ui)
{
	unsigned char c_ui[8] = { ui >> 56, ui >> 48, ui >> 40, ui >> 32,
							  ui >> 24, ui >> 16, ui >> 8, ui };
	unsigned i = 0;

	CHECK(mk_writeID(c, id));
	while (i < 7 && c_ui[i] == 0)
		++i;
	CHECK(mk_writeSize(c, 8 - i));
	CHECK(mk_appendContextData(c, c_ui + i, 8 - i));
	return 0;
}

int mk_writeSInt(mk_Context *c, unsigned id, int64_t si)
{
	unsigned char c_si[8] = { si >> 56, si >> 48, si >> 40, si >> 32,
							  si >> 24, si >> 16, si >> 8, si };
	unsigned i = 0;

	CHECK(mk_writeID(c, id));
	if (si < 0) {
		while (i < 7 && c_si[i] == 0xff && c_si[i + 1] & 0x80)
			++i;
	} else {
		while (i < 7 && c_si[i] == 0 && !(c_si[i + 1] & 0x80))
			++i;
	}
	CHECK(mk_writeSize(c, 8 - i));
	CHECK(mk_appendContextData(c, c_si + i, 8 - i));
	return 0;
}

int mk_writeFloatRaw(mk_Context *c, float f)
{
	union {
		float f;
		unsigned u;
	} u;
	unsigned char c_f[4];

	u.f = f;
	c_f[0] = u.u >> 24;
	c_f[1] = u.u >> 16;
	c_f[2] = u.u >> 8;
	c_f[3] = u.u;

	return mk_appendContextData(c, c_f, 4);
}

int mk_writeFloat(mk_Context *c, unsigned id, float f)
{
	CHECK(mk_writeID(c, id));
	CHECK(mk_writeSize(c, 4));
	CHECK(mk_writeFloatRaw(c, f));
	return 0;
}

int mk_writeVoid(mk_Context *c, uint64_t length)
{
	/* It would probably be faster to do this on the stack. */
	char *c_void = calloc(length, sizeof(*c_void));

	CHECK(mk_writeID(c, EBML_ID_VOID));
	CHECK(mk_writeSize(c, length));
	CHECK(mk_appendContextData(c, c_void, length));
	free(c_void);

	return 0;
}

unsigned mk_ebmlSizeSize(uint64_t s)
{
	if (s < 0x7fll)
		return 1;
	if (s < 0x3fffll)
		return 2;
	if (s < 0x1fffffll)
		return 3;
	if (s < 0x0fffffffll)
		return 4;
	if (s < 0x07ffffffffll)
		return 5;
	if (s < 0x03ffffffffffll)
		return 6;
	if (s < 0x01ffffffffffffll)
		return 7;
	return 8;
}

unsigned mk_ebmlSIntSize(int64_t si)
{
	unsigned char c_si[8] =	{ si >> 56, si >> 48, si >> 40, si >> 32,
							  si >> 24, si >> 16, si >> 8, si };
	unsigned i = 0;

	if (si < 0) {
		while (i < 7 && c_si[i] == 0xff && c_si[i + 1] & 0x80)
			++i;
	} else {
		while (i < 7 && c_si[i] == 0 && !(c_si[i + 1] & 0x80))
			++i;
	}

	return 8 - i;
}

unsigned mk_ebmlUIntSize(uint64_t ui)
{
	unsigned char c_ui[8] = { ui >> 56, ui >> 48, ui >> 40, ui >> 32,
							  ui >> 24, ui >> 16, ui >> 8, ui };
	unsigned i = 0;

	while (i < 7 && c_ui[i] == 0)
		++i;

	return 8 - i;
}

