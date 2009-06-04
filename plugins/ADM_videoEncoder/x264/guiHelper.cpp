/***************************************************************************
                        GUI_x264.cpp  -  description
                        ----------------------------

      Common helper functions for x264 GUI.  Mostly modified from x264.

    begin                : Tue Apr 01 2008
    copyright            : (C) 2008 by gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_inttype.h"
#include "guiHelper.h"

char *x264_slurp_file(const char *filename)
{
	int b_error = 0;
	int i_size;
	char *buf;

	FILE *fh = fopen(filename, "rb");

	if (!fh)
		return NULL;

	b_error |= fseek(fh, 0, SEEK_END) < 0;
	b_error |= (i_size = ftell(fh) ) <= 0;
	b_error |= fseek(fh, 0, SEEK_SET) < 0;

	if(b_error)
		return NULL;

	buf = new char[i_size + 2];

	if (buf == NULL)
		return NULL;

	b_error |= fread(buf, 1, i_size, fh) != i_size;

	if (buf[i_size-1] != '\n')
		buf[i_size++] = '\n';

	buf[i_size] = 0;
	fclose(fh);

	if (b_error)
	{
		delete [] buf;
		return NULL;
	}

	return buf;
}

int x264_cqm_parse_jmlist(const char *buf, const char *name, uint8_t *cqm, uint8_t *defaultMatrix, int length)
{
	char *p;
	char *nextvar;
	int i;

	p = strstr(buf, name);

	if (!p)
	{
		memset(cqm, 16, length);
		return 0;
	}

	p += strlen(name);

	if (*p == 'U' || *p == 'V')
		p++;

	nextvar = strstr(p, "INT");

	for (i = 0; i < length && (p = strpbrk(p, " \t\n,")) && (p = strpbrk(p, "0123456789")); i++)
	{
		int coef = -1;

		sscanf(p, "%d", &coef);

		if (i == 0 && coef == 0)
		{
			memcpy(cqm, defaultMatrix, length);
			return 0;
		}

		if(coef < 1 || coef > 255)
		{
			//x264_log( h, X264_LOG_ERROR, "bad coefficient in list '%s'\n", name );
			return -1;
		}

		cqm[i] = coef;
	}

	if((nextvar && p > nextvar) || i != length)
	{
		//x264_log( h, X264_LOG_ERROR, "not enough coefficients in list '%s'\n", name );
		return -1;
	}

	return 0;
}

int x264_cqm_parse_file(const char *filename, uint8_t* intra4x4Luma, uint8_t* intra4x4Chroma, uint8_t* inter4x4Luma, 
						uint8_t* inter4x4Chroma, uint8_t* intra8x8Luma, uint8_t* inter8x8Luma)
{
	char *buf, *p;
	int b_error = 0;
	uint8_t* dummyMatrix = new uint8_t[64];

	buf = x264_slurp_file(filename);

	if (!buf)
	{
		//x264_log( h, X264_LOG_ERROR, "can't open file '%s'\n", filename );
		return -1;
	}

	while ((p = strchr(buf, '#')) != NULL)
		memset(p, ' ', strcspn(p, "\n"));

	memset(dummyMatrix, 16, 64);

	b_error |= x264_cqm_parse_jmlist(buf, "INTRA4X4_LUMA",   intra4x4Luma, dummyMatrix, 16);
	b_error |= x264_cqm_parse_jmlist(buf, "INTRA4X4_CHROMA", intra4x4Chroma, dummyMatrix, 16);
	b_error |= x264_cqm_parse_jmlist(buf, "INTER4X4_LUMA",   inter4x4Luma, dummyMatrix, 16);
	b_error |= x264_cqm_parse_jmlist(buf, "INTER4X4_CHROMA", inter4x4Chroma, dummyMatrix, 16);
	b_error |= x264_cqm_parse_jmlist(buf, "INTRA8X8_LUMA",   intra8x8Luma, dummyMatrix, 64);
	b_error |= x264_cqm_parse_jmlist(buf, "INTER8X8_LUMA",   inter8x8Luma, dummyMatrix, 64);

	delete [] buf;
	delete dummyMatrix;

	return b_error;
}
