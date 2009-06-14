/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Encoder - Lower Sampling Frequency Extension
 *
 * $Id$
 *
 * $Log: musicin.h,v $
 * Revision 1.1  2004/01/30 19:39:45  mean
 * Initial revision
 *
 * Revision 1.1  2004/01/10 17:12:59  mean
 * new
 *
 * Revision 1.1  1996/02/14 04:04:23  rowlands
 * Initial revision
 *
 * Received from Mike Coleman
 **********************************************************************/

#ifndef LOOP_DOT_H
#define LOOP_DOT_H
#include "common.h"

/**********************************************************************
 *   date   programmers                comment                        *
 * 25. 6.92  Toshiyuki Ishino          Ver 1.0                        *
 * 29.10.92  Masahiro Iwadare          Ver 2.0                        *
 * 17. 4.93  Masahiro Iwadare          Updated for IS Modification    *
 *                                                                    *
 *********************************************************************/

extern int cont_flag;

#define e              2.71828182845

#define CBLIMIT       21

#define SFB_LMAX 22
#define SFB_SMAX 13

extern int pretab[];

struct scalefac_struct
{
  int l[23];
  int s[14];
};

extern struct scalefac_struct sfBandIndex[];	/* Table B.8 -- in loop.c */

int nint (FLOAT in);

#define maximum(A,B) ( (A) > (B) ? (A) : (B) )
#define minimum(A,B) ( (A) < (B) ? (A) : (B) )
#define signum( A ) ( (A) > 0 ? 1 : -1 )

/* GLOBALE VARIABLE */

extern int bit_buffer[50000];

#endif
