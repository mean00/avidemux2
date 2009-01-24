/* benchmark.h, this file is part of the
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

#include <math.h>
#include <sys/time.h>

#define BENCHMARK_ARGS_BUFSIZE 256

struct benchmark_times {
  struct timeval total, min, max;
  char min_args[BENCHMARK_ARGS_BUFSIZE];
  char max_args[BENCHMARK_ARGS_BUFSIZE];
};

struct benchmark_stats {
    unsigned long counter;    /* number of times function is called */
    unsigned long iterations; /* value is calibrated for each function */
    unsigned long benchmark_runs; /* number of time benchmar is run */
    struct benchmark_times times[2];
};

#ifdef __cplusplus
extern "C" {
#endif

int calibrate_benchmark(double precision, double increment, int timelimit,
                        int reqpasses, int *passes, int *iterations,
                        int *calibration, struct timeval *time,
                        struct timeval *last_time);

void update_benchmark_times(const char *name,
                            struct benchmark_times *times,
                            struct timeval *time,
                            const char *fmt, ...)
                            __attribute__ ((format (printf, 4,5)));

void print_calibration_stats(int count, char* name[], int *calibration,
                             struct timeval *times);

void print_benchmark_diff(int count, char* name[], struct timeval *times);

void print_benchmark_stats(const char* name, struct benchmark_stats *stats);

void print_benchmark_statistics();

#ifdef __cplusplus
}
#endif

#define BENCHMARK_TIME(i,iterations,do,start,end,time)                       \
  gettimeofday((start), (struct timezone*)0);                                \
  for ((i) = 0; (i) < (iterations); (i)++) {                                 \
     do; do; do; do; do; do; do; do; do; do;                                 \
  }                                                                          \
  gettimeofday((end), (struct timezone*)0);                                  \
  timersub((end), (start), (time));                                          \



#define ALTIVEC_TEST_BENCHMARK(name,ret,defargs,pfmt,args)                   \
ret ALTIVEC_TEST_WITH(name) defargs;                                         \
struct benchmark_stats name##_altivec_benchmark_stats;                       \
ret name##_altivec_benchmark defargs {                                       \
  int i, j, passes, done;                                                    \
  struct timeval start, end, times[2], last_time;                            \
  char *fnames[2];                                                           \
  ret (*pfuncs[2]) defargs;                                                  \
  ret (*pfunc) defargs;                                                      \
  int iterations, calibration[2];                                            \
  AVRETDECL(ret,retval);                                                     \
                                                                             \
  if (name##_altivec_benchmark_stats.counter % BENCHMARK_FREQUENCY == 0 &&   \
      name##_altivec_benchmark_stats.benchmark_runs < BENCHMARK_MAX_RUNS)    \
  {                                                                          \
    name##_altivec_benchmark_stats.benchmark_runs++;                         \
    fnames[0] = AVSTR(name##_altivec);                                       \
    fnames[1] = AVSTR(ALTIVEC_TEST_WITH(name));                              \
    pfuncs[0] = name##_altivec;                                              \
    pfuncs[1] = ALTIVEC_TEST_WITH(name);                                     \
                                                                             \
    /* make sure calibration has been done */                                \
    if (name##_altivec_benchmark_stats.iterations == 0)                      \
    {                                                                        \
      mjpeg_info("starting benchmark calibration for " #name);               \
                                                                             \
      for (i = 0; i < 2; i++)                                                \
      {                                                                      \
        pfunc = pfuncs[i];                                                   \
        passes = 0;                                                          \
        iterations = BENCHMARK_ITERATIONS / 10; /* /10 for BENCHMARK_TIME */ \
        last_time.tv_sec = last_time.tv_usec = 0;                            \
                                                                             \
        do                                                                   \
        {                                                                    \
          BENCHMARK_TIME(j,iterations,(*pfunc)(args),&start,&end,&times[i]); \
          done = calibrate_benchmark(BENCHMARK_PRECISION,                    \
                                     BENCHMARK_INCREMENT,                    \
                                     BENCHMARK_TIMELIMIT,                    \
                                     BENCHMARK_PASSES,                       \
                                     &passes, &iterations, &calibration[i],  \
                                     &times[i], &last_time);                 \
        } while (!done);                                                     \
      }                                                                      \
                                                                             \
      calibration[0] *= 10; /* adjust for previous /10 for BENCHMARK_TIME */ \
      calibration[1] *= 10;                                                  \
                                                                             \
      iterations = (calibration[0] > calibration[1]                          \
                 ?  calibration[0]                                           \
                 :  calibration[1]);                                         \
      name##_altivec_benchmark_stats.iterations = iterations;                \
                                                                             \
      print_calibration_stats(2, fnames, calibration, times);                \
    }                                                                        \
                                                                             \
    BENCHMARK_PROLOG                                                         \
                                                                             \
    for (i = 0; i < 2; i++)                                                  \
    {                                                                        \
      pfunc = pfuncs[i];                                                     \
      iterations = name##_altivec_benchmark_stats.iterations / 10;           \
                                                                             \
      BENCHMARK_TIME(j,iterations,(*pfunc)(args),&start,&end,&times[i]);     \
                                                                             \
      update_benchmark_times(fnames[i],                                      \
                             &name##_altivec_benchmark_stats.times[i],       \
                             &times[i], "%s(" pfmt ")", fnames[i], args);    \
    }                                                                        \
    print_benchmark_diff(2, fnames, times);                                  \
                                                                             \
    BENCHMARK_EPILOG                                                         \
  }                                                                          \
  name##_altivec_benchmark_stats.counter++;                                  \
                                                                             \
  AVRETSET(ret,retval,name##_altivec(args));                                 \
  AVRETURN(ret,retval);                                                      \
}
