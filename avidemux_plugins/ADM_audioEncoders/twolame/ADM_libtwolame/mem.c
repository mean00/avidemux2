/*
 *  TwoLAME: an optimized MPEG Audio Layer Two encoder
 *
 *  Copyright (C) 2001-2004 Michael Cheng
 *  Copyright (C) 2004-2005 The TwoLAME Project
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"

/*******************************************************************************
*
*  Allocate number of bytes of memory equal to "block".
*
*******************************************************************************/

void *twolame_malloc (unsigned long block, char *item)
{
	
	void *ptr;
	
	ptr = (void *) ADM_alloc (block);
	
	if (ptr != NULL) {
		memset (ptr, 0, block);
	} else {
		fprintf (stderr, "Unable to allocate %ld bytes for %s\n", block, item);
		return NULL;
	}
	
	return (ptr);
}


/****************************************************************************
*
*  Free memory pointed to by "*ptr_addr".
*
*****************************************************************************/

void twolame_free (void **ptr_addr)
{

	if (*ptr_addr != NULL) {
		ADM_dezalloc (*ptr_addr);
		*ptr_addr = NULL;
	}
}
