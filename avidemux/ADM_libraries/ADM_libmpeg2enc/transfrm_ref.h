/* transfrm.h, Low-level transformation (DCT/iDCT and prediction
 * difference picture handling) routines */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */
/*  Modifications and extension (C) 2000-2003 Andrew Stevens */

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

#ifndef _TRANSFRM_H
#define _TRANSFRM_H

#include "ADM_default.h"
#include "mjpeg_types.h"

// MEANX
/*
#ifdef  __cplusplus
extern "C" {
#endif
*/
/*
  Pointers to version of transform and prediction manipulation
  routines to be used..
 */

extern void (*pfdct)( int16_t * blk );
extern void (*pidct)( int16_t * blk );
extern void (*padd_pred) (uint8_t *pred, uint8_t *cur,
				   int lx, int16_t *blk);
extern void (*psub_pred) (uint8_t *pred, uint8_t *cur,
				   int lx, int16_t *blk);
extern int (*pfield_dct_best)( uint8_t *cur_lum_mb, uint8_t *pred_lum_mb);

int field_dct_best( uint8_t *cur_lum_mb, uint8_t *pred_lum_mb);

void add_pred (uint8_t *pred, uint8_t *cur,
               int lx, int16_t *blk);
void sub_pred (uint8_t *pred, uint8_t *cur,
               int lx, int16_t *blk);

void init_transform(void);

/*
void fdct( int16_t *blk );
void idct( int16_t *blk );
void init_fdct (void);
void init_idct (void);
*/
#ifdef HAVE_X86CPU
void init_mp2_fdct_sse();
void mp2_fdct_sse( int16_t *blk );
void mp2_idct_sse( int16_t *blk );
#endif
//MEANX
/*
#ifdef  __cplusplus
}
#endif
*/

#endif /*  _TRANSFRM_H */


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
