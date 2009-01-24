/* amber.h, this file is part of the
 * AltiVec optimized library for MJPEG tools MPEG-1/2 Video Encoder
 * Copyright (C) 2002  James Klicman <james@klicman.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef ALTIVEC_AMBER

#ifndef AMBER_MAX_TRACES
#define AMBER_MAX_TRACES 1
#endif

void amber_symtrace(const char *name);

#define ALTIVEC_TEST_AMBER(name,ret,defargs,pfmt,args) /* {{{ */             \
ret name##_altivec_amber defargs {                                           \
    static int trace_count = 0;                                              \
    register long sigreg;                                                    \
    AVRETDECL(ret,retval);                                                   \
    if (trace_count < AMBER_MAX_TRACES) {                                    \
        trace_count++;                                                       \
        asm volatile ("mfspr %0, 1023" : "=r" (sigreg)); /* start amber */   \
        AVRETSET(ret,retval,ALTIVEC_TEST_WITH(name)(args));                  \
        asm volatile ("mfspr %0, 1023" : "=r" (sigreg)); /* stop amber */    \
        amber_symtrace(AVSTR(ALTIVEC_TEST_WITH(name)));                      \
    } else {                                                                 \
        AVRETSET(ret,retval,ALTIVEC_TEST_WITH(name)(args));                  \
    }                                                                        \
    AVRETURN(ret,retval);                                                    \
} /* }}} */

#elif defined(AMBER_ENABLE) && !defined(ALTIVEC_VERIFY)

#ifndef AMBER_MAX_TRACES
#define AMBER_MAX_TRACES 1
#endif

#ifndef AMBER_MAX_EXIT
#define AMBER_MAX_EXIT 1
#endif

long amber_start(const char *file, int line,
                 int *trace_count, int max_traces);
long amber_stop(const char *file, int line,
                int *trace_count, int max_traces, int max_exit);

static int amber_trace_count = 0;

#define AMBER_START amber_start(__FILE__, __LINE__, &amber_trace_count, \
                                AMBER_MAX_TRACES)
#define AMBER_STOP  amber_stop(__FILE__, __LINE__, &amber_trace_count, \
                               AMBER_MAX_TRACES, AMBER_MAX_EXIT)
#endif


#ifndef AMBER_START
#  define AMBER_START
#endif
#ifndef AMBER_STOP
#  define AMBER_STOP
#endif
