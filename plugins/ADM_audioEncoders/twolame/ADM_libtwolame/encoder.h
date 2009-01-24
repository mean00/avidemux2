#ifndef ENCODER_DOT_H
#define ENCODER_DOT_H
#include "toolame.h"

/* General Definitions */

/* Default Input Arguments (for command line control) */

#define DFLT_LAY        2	/* default encoding layer is II */
#define DFLT_MOD        'j'	/* default mode is joint stereo */
#define DFLT_PSY        1	/* default psych model is 1 */
#define DFLT_SFQ        44.1	/* default input sampling rate is 44.1 kHz */
#define DFLT_EMP        'n'	/* default de-emphasis is none */
#define DFLT_EXT        ".mp2"	/* default output file extension */
#define DFLT_BRI        10	/* default bitrate_index = 10 (192kbps) */

#define FILETYPE_ENCODE 'TEXT'
#define CREATOR_ENCODE  'MpgD'

/* This is the smallest MNR a subband can have before it is counted
   as 'noisy' by the logic which chooses the number of JS subbands */

#define NOISY_MIN_MNR   0.0

/* Psychacoustic Model 1 Definitions */

#define CB_FRACTION     0.33
#define MAX_SNR         1000
#define NOISE           10
#define TONE            20
#define DBMIN           -200.0
#define LAST            -1
#define STOP            -100
#define POWERNORM       90.3090	/* = 20 * log10(32768) to normalize */
/* max output power to 96 dB per spec */

/* Psychoacoustic Model 2 Definitions */

#define LXMIN           32.0

/***********************************************************************
*
*  Encoder Type Definitions
*
***********************************************************************/

/* Psychoacoustic Model 1 Type Definitions */

typedef int IFFT2[FFT_SIZE / 2];
typedef int IFFT[FFT_SIZE];
typedef FLOAT D9[9];
typedef FLOAT D10[10];
typedef FLOAT D640[640];
typedef FLOAT D1408[1408];
typedef FLOAT DFFT2[FFT_SIZE / 2];
typedef FLOAT DFFT[FFT_SIZE];
typedef FLOAT DSBL[SBLIMIT];
typedef FLOAT D2SBL[2][SBLIMIT];


/* Psychoacoustic Model 2 Type Definitions */


#endif
