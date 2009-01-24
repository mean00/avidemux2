/* syntaxparams.h, Global constants controlling MPEG syntax  */

/*  (C) 2003 Andrew Stevens */

/* These modifications are free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */

#ifndef _SYNTAXCONSTS_H
#define _SYNTAXCONSTS_H


/* SCale factor for fast integer arithmetic routines */
/* Changed this and you *must* change the quantisation routines as
   they depend on its absolute value */

#define IQUANT_SCALE_POW2 16
#define IQUANT_SCALE (1<<IQUANT_SCALE_POW2)
#define COEFFSUM_SCALE (1<<16)

#define BITCOUNT_OFFSET  0LL


/* Globally defined MPEG syntax constants */

#define CHROMA_FORMAT YUV420
#define BLOCK_COUNT 6

#endif

/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
