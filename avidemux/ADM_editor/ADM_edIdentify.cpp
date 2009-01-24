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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ADM_assert.h"
#include "config.h"
#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"
#if 0
#include "ADM_inputs/ADM_inpics/ADM_pics.h"
#include "ADM_inputs/ADM_nuv/ADM_nuv.h"
#include "ADM_inputs/ADM_h263/ADM_h263.h"
//#include "ADM_3gp/ADM_3gp.h"
#include "ADM_inputs/ADM_mp4/ADM_mp4.h"
#include "ADM_inputs/ADM_avsproxy/ADM_avsproxy.h"
#endif


#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"

#include "ADM_coreDemuxerMpeg/include/dmx_identify.h"
#include "ADM_assert.h"

DMX_TYPE dmxIdentify(const char *name);

/**
	Read the magic of a file i.e. its 16 first bytes
*/
uint8_t ADM_Composer::getMagic (const char *name, uint32_t * magic)
{
  FILE *    tmp;
  uint8_t    ret =    1;

  tmp = fopen (name, "rb");
  if (!tmp)
    return 0;

  if (4 != fread (magic, 4, 4, tmp))
    ret = 0;

  fclose (tmp);
  return ret;

}
// EOF

