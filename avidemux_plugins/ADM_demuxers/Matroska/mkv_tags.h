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
 
#ifndef ADM_MKV_TAGS
#define ADM_MKV_TAGS


typedef enum 
{
    ADM_MKV_TYPE_UNKNOWN,
    ADM_MKV_TYPE_CONTAINER,
    ADM_MKV_TYPE_STRING,
    ADM_MKV_TYPE_UTF8,
    ADM_MKV_TYPE_DATE,
    ADM_MKV_TYPE_FLOAT,
    ADM_MKV_TYPE_UINTEGER,
    ADM_MKV_TYPE_INTEGER,
    ADM_MKV_TYPE_BINARY
}ADM_MKV_TYPE;

typedef enum 
{
  ADM_MKV_PRIMARY, 
  ADM_MKV_SECONDARY
}ADM_MKV_SEARCHTYPE;

typedef enum 
{
#include "mkv_tagenum.h"
}MKV_ELEM_ID;


uint8_t ADM_searchMkvTag(MKV_ELEM_ID tag,const char **asString,ADM_MKV_TYPE *type);
const char *ADM_mkvTypeAsString(ADM_MKV_TYPE type);

#endif
