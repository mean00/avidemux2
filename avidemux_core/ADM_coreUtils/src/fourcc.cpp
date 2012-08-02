/***************************************************************************
                          fourcc.cpp  -  description
                             -------------------
    begin                : Fri Nov 2 2001
    copyright            : (C) 2001 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef _WIN32
#	include <windows.h>
#endif

#include "ADM_default.h"
#include "avifmt.h"
#include "fourcc.h"

void fourCC::print(uint32_t fourcc)
{
    char s[5];
    s[4] = 0;

	s[3]=((fourcc & 0xff000000)>>24)&0xff;
	s[2]=((fourcc & 0xff0000)>>16)&0xff;
	s[1]=((fourcc & 0xff00)>>8)&0xff;
	s[0]=((fourcc & 0xff)>>0)&0xff;

    printf("%s (%08X)", s,fourcc);

}
void fourCC::printBE(uint32_t fourcc)
{
    char s[5];
    s[4] = 0;

	s[0]=((fourcc & 0xff000000)>>24)&0xff;
	s[1]=((fourcc & 0xff0000)>>16)&0xff;
	s[2]=((fourcc & 0xff00)>>8)&0xff;
	s[3]=((fourcc & 0xff)>>0)&0xff;

    printf("%s (%08X)", s,fourcc);

}
char *fourCC::tostring(uint32_t fourcc)
{
    static char s[5];
    s[4] = 0;

	s[3]=((fourcc & 0xff000000)>>24)&0xff;
	s[2]=((fourcc & 0xff0000)>>16)&0xff;
	s[1]=((fourcc & 0xff00)>>8)&0xff;
	s[0]=((fourcc & 0xff)>>0)&0xff;

    return s;
}
char *fourCC::tostringBE(uint32_t fourcc)
{
    static char s[5];
    s[4] = 0;
    uint8_t *p=(uint8_t *)&fourcc;
    s[0]=p[3];
    s[1]=p[2];
    s[2]=p[1];
    s[3]=p[0];
    return s;
}
//_____________________________________
int fourCC::check(uint8_t * in, uint8_t * cc)
{
    uint32_t *inb;

    inb = (uint32_t *) in;
#if defined( ADM_BIG_ENDIAN)
    if (*inb == (uint32_t) mmioFOURCC(*(cc+3), *(cc + 2), *(cc + 1), *(cc + 0)))
#else
    if (*inb == (uint32_t) mmioFOURCC(*cc, *(cc + 1), *(cc + 2), *(cc + 3)))
#endif
      {
	  return 1;

      }
    return 0;
}

//_____________________________________
uint32_t fourCC::get(const uint8_t * cc)
{
    uint32_t inb;

    inb = (uint32_t) mmioFOURCC(*cc, *(cc + 1), *(cc + 2), *(cc + 3));
    // bit clumsy but avoit endianness pb

    return inb;
}

int fourCC::check(const uint8_t * cc, uint32_t in)
{

    if (in == (uint32_t) mmioFOURCC(*cc, *(cc + 1), *(cc + 2), *(cc + 3)))
      {
	  return 1;

      }
    return 0;
}

int fourCC::check(uint32_t in, const uint8_t * cc)
{
    return check(cc, in);

}
