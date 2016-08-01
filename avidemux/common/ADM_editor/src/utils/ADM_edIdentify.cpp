/***************************************************************************
                          ADM_edIdentify.cpp  -  description
                             -------------------
	Identify a file by reading its magic


    begin                : Thu Feb 28 2003
    copyright            : (C) 2002 by mean
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
#include "ADM_cpp.h"
using std::string;
#include "ADM_default.h"
#include "fourcc.h"
#include "ADM_edit.hxx"
#include "dmx_identify.h"


/**
	Read the magic of a file i.e. its 16 first bytes
*/
uint8_t ADM_Composer::getMagic (const char *name, uint32_t * magic)
{
  FILE *    tmp;
  uint8_t    ret =    1;

  tmp = ADM_fopen (name, "rb");
  if (!tmp)
    return 0;

  if (4 != fread (magic, 4, 4, tmp))
    ret = 0;

  fclose (tmp);
  return ret;

}
// EOF

