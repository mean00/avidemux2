/***************************************************************************
 *   Copyright (C) 2007 by mean,    *
 *   fixounet@free.fr   *
 *                                                                         *
 *        EBML Handling code                                               *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ADM_ebml.h"
#include "mkv_tags.h"


typedef struct ADM_mkvTagEnum
{
    MKV_ELEM_ID   id;
    ADM_MKV_TYPE  type;
    const char    *name;
}ADM_mkvTagEnum;
#define ADM_BUILD_TAG
static const ADM_mkvTagEnum mkvTags[]={
#include "mkv_tagenum.h"
};

/**
    \fn ADM_searchMkvTag
    \brief Search the payload of this ELEM_ID
*/
uint8_t ADM_searchMkvTag(MKV_ELEM_ID tag,const char **asString,ADM_MKV_TYPE *type)
{
  int full=sizeof(mkvTags)/sizeof(ADM_mkvTagEnum);
  
  for(int i=0;i<full;i++)
  {
    if(mkvTags[i].id==tag)
    {
      *asString=mkvTags[i].name;
      *type  =mkvTags[i].type;
      return 1;
    }
    
  }
  *asString="??";
  *type=ADM_MKV_TYPE_UNKNOWN;
  return 0;
}
/**
    \fn ADM_mkvTypeAsString
    \brief Returns the type as a string
*/
const char *ADM_mkvTypeAsString(ADM_MKV_TYPE type)
{
#define CONV(x) if(type==x) return #x;
    CONV(ADM_MKV_TYPE_UNKNOWN)
    CONV(ADM_MKV_TYPE_CONTAINER)
    CONV(ADM_MKV_TYPE_STRING)
    CONV(ADM_MKV_TYPE_UTF8)
    CONV(ADM_MKV_TYPE_DATE)
    CONV(ADM_MKV_TYPE_FLOAT)
    CONV(ADM_MKV_TYPE_UINTEGER)
    CONV(ADM_MKV_TYPE_INTEGER)
    CONV(ADM_MKV_TYPE_BINARY)
        return "unknown type ???";
 
  
}
// EOF
