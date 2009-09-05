/*
* cpu_accel.c
* Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
*
* This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
*
* mpeg2dec is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* mpeg2dec is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define ADM_LEGACY_PROGGY
#include "ADM_default.h"

#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "cpu_accel.h"
#include "mjpeg_logging.h"

#ifdef HAVE_ALTIVEC
extern int altivec_copy_v0();
#endif

/*
 * Don't add "mblocks_sub44_mests" to the list below because it does not
 * have a C-reference code counterpart (mblocks_sub44_mests only exists as
 * a SIMD routine).
*/

	const char *disable_simd_flags[] = {
		"sad_00",
		"sad_01",
		"sad_10",
		"sad_11",
		"sad_sub22",
		"sad_sub44",
		"bsad",
		"variance",
		"sumsq",
		"sumsq_sub22",
		"bsumsq_sub22",
		"bsumsq",
		"build_sub22_mests",
		"build_sub44_mests",
		"subsample_image",
		"find_best_one_pel",
		"quant_nonintra",
		"quant_weight_intra",
		"quant_weight_nonintra",
		"iquant_intra",
		"iquant_nonintra",
		NULL
		};

static char *parse_next(char **, const char *);

#ifdef HAVE_X86CPU 

/* Some miscelaneous stuff to allow checking whether SSE instructions cause
   illegal instruction errors.
*/
#if !defined(__MINGW32__)
static sigjmp_buf sigill_recover;

static RETSIGTYPE sigillhandler(int sig )
{
	siglongjmp( sigill_recover, 1 );
}

typedef RETSIGTYPE (*__sig_t)(int);
#endif

static int testsseill()
{
	int illegal;
#if defined(__CYGWIN__) || defined(__MINGW32__)
	/* SSE causes a crash on CYGWIN, apparently.
	   Perhaps the wrong signal is being caught or something along
	   those line ;-) or maybe SSE itself won't work...
	*/
	illegal = 1;
#else
	__sig_t old_handler = signal( SIGILL, sigillhandler);
	if( sigsetjmp( sigill_recover, 1 ) == 0 )
	{
		asm ( "movups %xmm0, %xmm0" );
		illegal = 0;
	}
	else
		illegal = 1;
	signal( SIGILL, old_handler );
#endif
	return illegal;
}

static int x86_accel (void)
{
    intptr_t eax, ebx, ecx, edx;
    int32_t AMD;
    int32_t caps;

	/* Slightly weirdified cpuid that preserves the ebx and edi required
	   by gcc for PIC offset table and frame pointer */

#ifdef HAVE_X86_64CPU
#  define REG_b "rbx"
#  define REG_S "rsi"
#else
#  define REG_b "ebx"
#  define REG_S "esi"
#endif
	   
#define cpuid(op,eax,ebx,ecx,edx)	\
    asm ( "push %%"REG_b"\n" \
	      "cpuid\n" \
	      "mov   %%"REG_b", %%"REG_S"\n" \
	      "pop   %%"REG_b"\n"  \
	 : "=a" (eax),			\
	   "=S" (ebx),			\
	   "=c" (ecx),			\
	   "=d" (edx)			\
	 : "a" (op)			\
	 : "cc", "edi")

    asm ("pushf\n\t"
	 "pop %0\n\t"
	 "mov %0,%1\n\t"
	 "xor $0x200000,%0\n\t"
	 "push %0\n\t"
	 "popf\n\t"
	 "pushf\n\t"
	 "pop %0"
         : "=a" (eax),
	       "=c" (ecx)
	 :
	 : "cc");


    if (eax == ecx)		// no cpuid
	return 0;

    cpuid (0x00000000, eax, ebx, ecx, edx);
    if (!eax)			// vendor string only
	return 0;

    AMD = (ebx == 0x68747541) && (ecx == 0x444d4163) && (edx == 0x69746e65);

    cpuid (0x00000001, eax, ebx, ecx, edx);
    if (! (edx & 0x00800000))	// no MMX
	return 0;

    caps = ACCEL_X86_MMX;
    /* If SSE capable CPU has same MMX extensions as AMD
	   and then some. However, to use SSE O.S. must have signalled
	   it use of FXSAVE/FXRSTOR through CR4.OSFXSR and hence FXSR (bit 24)
	   here
	*/
    if ((edx & 0x02000000))	
		caps = ACCEL_X86_MMX | ACCEL_X86_MMXEXT;
	if( (edx & 0x03000000) == 0x03000000 )
	{
		/* Check whether O.S. has SSE support... has to be done with
		   exception 'cos those Intel morons put the relevant bit
		   in a reg that is only accesible in ring 0... doh! 
		*/
		if( !testsseill() )
			caps |= ACCEL_X86_SSE;
	}

    cpuid (0x80000000, eax, ebx, ecx, edx);
    if (eax < 0x80000001)	// no extended capabilities
		return caps;

    cpuid (0x80000001, eax, ebx, ecx, edx);

    if (edx & 0x80000000)
	caps |= ACCEL_X86_3DNOW;

    if (AMD && (edx & 0x00400000))	// AMD MMX extensions
	{
		caps |= ACCEL_X86_MMXEXT;
	}

    return caps;
}
#endif


#ifdef HAVE_ALTIVEC
/* AltiVec optimized library for MJPEG tools MPEG-1/2 Video Encoder
 * Copyright (C) 2002  James Klicman <james@klicman.org>
 *
 * The altivec_detect() function has been moved here to workaround a bug in a
 * released version of GCC (3.3.3). When -maltivec and -mabi=altivec are
 * specified, the bug causes VRSAVE instructions at the beginning and end of
 * functions which do not use AltiVec. GCC 3.3.3 also lacks support for
 * '#pragma altivec_vrsave off' which would have been the preferred workaround.
 *
 * This AltiVec detection code relies on the operating system to provide an
 * illegal instruction signal if AltiVec is not present. It is known to work
 * on Mac OS X and Linux.
 */

static sigjmp_buf jmpbuf;

static void sig_ill(int sig)
{
    siglongjmp(jmpbuf, 1);
}

int detect_altivec()
{
    volatile int detected = 0; /* volatile (modified after sigsetjmp) */
    struct sigaction act, oact;

    act.sa_handler = sig_ill;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(SIGILL, &act, &oact)) {
	perror("sigaction");
	return 0;
    }

    if (sigsetjmp(jmpbuf, 1))
	goto noAltiVec;

    /* try to read an AltiVec register */ 
    altivec_copy_v0();

    detected = 1;

noAltiVec:
    if (sigaction(SIGILL, &oact, (struct sigaction *)0))
	perror("sigaction");

    return detected;
}
#endif


int32_t cpu_accel (void)
{
#ifdef HAVE_X86CPU 
    static int got_accel = 0;
    static int accel;

    if (!got_accel) {
		got_accel = 1;
		accel = x86_accel ();
    }

    return accel;
#elif defined(HAVE_ALTIVEC)
    return detect_altivec();
#else
    return 0;
#endif
}

/*****************************
 *
 * Allocate memory aligned to suit SIMD 
 *
 ****************************/

#define powerof2(x)     ((((x)-1)&(x))==0)

#if	!defined(HAVE_POSIX_MEMALIGN)

int
posix_memalign(void **ptr, size_t alignment, size_t size)
{
	void *mem;

	if	(alignment % sizeof (void *) != 0 || !powerof2(alignment) != 0)
		return(EINVAL);
	mem = malloc((size + alignment - 1) & ~(alignment - 1));
	if	(mem != NULL)
	{
		*ptr = mem;
		return(0);
	}
	return(ENOMEM);
}
#endif

#if	!defined(HAVE_MEMALIGN)
void *
memalign(size_t alignment, size_t size)
{

	if 	(alignment % sizeof (void *) || !powerof2(alignment))
	{
		errno = EINVAL;
		return(NULL);
	}
	return(malloc((size + alignment - 1) & ~(alignment - 1)));
}
#endif

/***********************
 * Implement fmax() for systems which do not have it.  Not a strictly
 * conforming implementation - we don't bother checking for NaN which if
 * mpeg2enc gets means big trouble I suspect ;)
************************/

#if	!defined(HAVE_FMAX)
double
fmax(double x, double y)
{
	if	(x > y)
		return(x);
	return(y);
}
#endif

void *bufalloc( size_t size )
{
	static size_t simd_alignment = 16;
	static int bufalloc_init = 0;
	int  pgsize;
	void *buf = NULL;

	if( !bufalloc_init )
	{
#ifdef HAVE_X86CPU 
		if( (cpu_accel() &  (ACCEL_X86_SSE|ACCEL_X86_3DNOW)) != 0 )
		{
			simd_alignment = 64;
			bufalloc_init = 1;
		}
#endif		
	}
		
#ifdef __MINGW32__	// GRUNTSTER
	pgsize = 4096;
#else
	pgsize = sysconf(_SC_PAGESIZE);
#endif

/*
 * If posix_memalign fails it could be a broken glibc that caused the error,
 * so try again with a page aligned memalign request
*/
	if (posix_memalign( &buf, simd_alignment, size))
		buf = memalign(pgsize, size);
	if (buf && ((size_t)buf & (simd_alignment - 1)))
	{
		free(buf);
		buf = memalign(pgsize, size);
	}
	if (buf == NULL)
		mjpeg_error_exit1("malloc of %d bytes failed", (int)size);
	if ((size_t)buf & (simd_alignment - 1))
		mjpeg_error_exit1("could not allocate %d bytes aligned on a %d byte boundary", (int)size, (int)simd_alignment);
	return buf;
}

int
disable_simd(char *name)
	{
	int	foundit;
	char	*cp, *simd_env, *dup_backup;
	const char **dft;

	if	((cp = getenv("MJPEGTOOLS_SIMD_DISABLE")) == NULL)
		return(0);

/*
 * Special case for "all" so that all 22 or whatever names don't have to be
 * explicitly specified.  If "all" is seen as the only name in the environment
 * variable then always return 1.
*/
	if	(strcasecmp(cp, "all") == 0)
		return(1);

/*
 * First check that the routine being tested for disabled status exists in
 * the list of known functions.
*/
	foundit = 0;
	for	(dft = disable_simd_flags; *dft; dft++)
		{
		if	(strcasecmp(name, *dft) == 0)
			{
			foundit = 1;
			break;
			}
		}
	if	(foundit == 0)
		return(0);

/*
 * Next compare the function name passed as input to the comma separated
 * names in the environment variable.  If a match is found then return 1
*/
	dup_backup = simd_env = strdup(cp);
	while	((cp = parse_next(&simd_env, ",")))
		{
		foundit = 0;
		if	(strcasecmp(cp, name) == 0)
			{
			foundit = 1;
			break;
			}
		}
	free(dup_backup);
	return(foundit);
	}

static char *parse_next(char **sptr, const char *delim)
{
	char *start, *ret;
	start = ret = *sptr;
	if ((ret == NULL) || ret == '\0') {
	   return (NULL);
	}

	while (*ret != '\0' &&
		   strchr(delim, *ret) == NULL) {
		ret++;
	}
	if (*ret == '\0') {
		*sptr = NULL;
	} else {
	    *ret = '\0';
	    ret++;
	    *sptr = ret;
	}
	return (start);
}
