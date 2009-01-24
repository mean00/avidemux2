/* benchmark.c, this file is part of the
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "altivec_conf.h"

#include "../mjpeg_logging.h"

#ifdef ALTIVEC_BENCHMARK

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define MAX_TIME_VALUE (long)(((unsigned long)-1L)>>1)

#define TSECS(tvp) (((double)(tvp)->tv_sec)+(((double)(tvp)->tv_usec)*0.000001))


struct benchmark_info {
  char *name;
  char *cmpname;
  struct benchmark_stats *stats;
};

#define EXTERN_STATS(n) \
extern struct benchmark_stats n##_altivec_benchmark_stats

#if ALTIVEC_TEST_FUNCTION(sad_00)
  EXTERN_STATS(sad_00);
#endif
#if ALTIVEC_TEST_FUNCTION(sad_01)
  EXTERN_STATS(sad_01);
#endif
#if ALTIVEC_TEST_FUNCTION(sad_10)
  EXTERN_STATS(sad_10);
#endif
#if ALTIVEC_TEST_FUNCTION(sad_11)
  EXTERN_STATS(sad_11);
#endif
#if ALTIVEC_TEST_FUNCTION(bsad)
  EXTERN_STATS(bsad);
#endif
#if ALTIVEC_TEST_FUNCTION(sumsq)
  EXTERN_STATS(sumsq);
#endif
#if ALTIVEC_TEST_FUNCTION(sumsq_sub22)
  EXTERN_STATS(sumsq_sub22);
#endif
#if ALTIVEC_TEST_FUNCTION(bsumsq)
  EXTERN_STATS(bsumsq);
#endif
#if ALTIVEC_TEST_FUNCTION(bsumsq_sub22)
  EXTERN_STATS(bsumsq_sub22);
#endif
#if ALTIVEC_TEST_FUNCTION(find_best_one_pel)
  EXTERN_STATS(find_best_one_pel);
#endif
#if ALTIVEC_TEST_FUNCTION(build_sub22_mests)
  EXTERN_STATS(build_sub22_mests);
#endif
#if ALTIVEC_TEST_FUNCTION(build_sub44_mests)
  EXTERN_STATS(build_sub44_mests);
#endif
#if ALTIVEC_TEST_FUNCTION(quant_non_intra)
  EXTERN_STATS(quant_non_intra);
#endif
#if ALTIVEC_TEST_FUNCTION(quant_weight_coeff_sum)
  EXTERN_STATS(quant_weight_coeff_sum);
#endif
#if ALTIVEC_TEST_FUNCTION(iquant_non_intra_m1)
  EXTERN_STATS(iquant_non_intra_m1);
#endif
#if ALTIVEC_TEST_FUNCTION(iquant_non_intra_m2)
  EXTERN_STATS(iquant_non_intra_m2);
#endif
#if ALTIVEC_TEST_FUNCTION(iquant_intra_m1)
  EXTERN_STATS(iquant_intra_m1);
#endif
#if ALTIVEC_TEST_FUNCTION(iquant_intra_m2)
  EXTERN_STATS(iquant_intra_m2);
#endif
#if ALTIVEC_TEST_FUNCTION(variance)
  EXTERN_STATS(variance);
#endif
#if ALTIVEC_TEST_FUNCTION(sub_pred)
  EXTERN_STATS(sub_pred);
#endif
#if ALTIVEC_TEST_FUNCTION(add_pred)
  EXTERN_STATS(add_pred);
#endif
#if ALTIVEC_TEST_FUNCTION(pred_comp)
  EXTERN_STATS(pred_comp);
#endif
#if ALTIVEC_TEST_FUNCTION(subsample_image)
  EXTERN_STATS(subsample_image);
#endif
#if ALTIVEC_TEST_FUNCTION(sub_mean_reduction)
  EXTERN_STATS(sub_mean_reduction);
#endif
#if ALTIVEC_TEST_FUNCTION(fdct)
  EXTERN_STATS(fdct);
#endif
#if ALTIVEC_TEST_FUNCTION(idct)
  EXTERN_STATS(idct);
#endif


#define BENCHMARK_INFO(n) \
   AVSTR(n##_altivec),  AVSTR(ALTIVEC_TEST_WITH(n)), \
   &n##_altivec_benchmark_stats,

struct benchmark_info benchmarktab[] = {
#if ALTIVEC_TEST_FUNCTION(sad_00)
  BENCHMARK_INFO(sad_00)
#endif
#if ALTIVEC_TEST_FUNCTION(sad_01)
  BENCHMARK_INFO(sad_01)
#endif
#if ALTIVEC_TEST_FUNCTION(sad_10)
  BENCHMARK_INFO(sad_10)
#endif
#if ALTIVEC_TEST_FUNCTION(sad_11)
  BENCHMARK_INFO(sad_11)
#endif
#if ALTIVEC_TEST_FUNCTION(bsad)
  BENCHMARK_INFO(bsad)
#endif
#if ALTIVEC_TEST_FUNCTION(sumsq)
  BENCHMARK_INFO(sumsq)
#endif
#if ALTIVEC_TEST_FUNCTION(sumsq_sub22)
  BENCHMARK_INFO(sumsq_sub22)
#endif
#if ALTIVEC_TEST_FUNCTION(bsumsq)
  BENCHMARK_INFO(bsumsq)
#endif
#if ALTIVEC_TEST_FUNCTION(bsumsq_sub22)
  BENCHMARK_INFO(bsumsq_sub22)
#endif
#if ALTIVEC_TEST_FUNCTION(find_best_one_pel)
  BENCHMARK_INFO(find_best_one_pel)
#endif
#if ALTIVEC_TEST_FUNCTION(build_sub22_mests)
  BENCHMARK_INFO(build_sub22_mests)
#endif
#if ALTIVEC_TEST_FUNCTION(build_sub44_mests)
  BENCHMARK_INFO(build_sub44_mests)
#endif
#if ALTIVEC_TEST_FUNCTION(quant_non_intra)
  BENCHMARK_INFO(quant_non_intra)
#endif
#if ALTIVEC_TEST_FUNCTION(quant_weight_coeff_sum)
  BENCHMARK_INFO(quant_weight_coeff_sum)
#endif
#if ALTIVEC_TEST_FUNCTION(iquant_non_intra_m1)
  BENCHMARK_INFO(iquant_non_intra_m1)
#endif
#if ALTIVEC_TEST_FUNCTION(iquant_non_intra_m2)
  BENCHMARK_INFO(iquant_non_intra_m2)
#endif
#if ALTIVEC_TEST_FUNCTION(iquant_intra_m1)
  BENCHMARK_INFO(iquant_intra_m1)
#endif
#if ALTIVEC_TEST_FUNCTION(iquant_intra_m2)
  BENCHMARK_INFO(iquant_intra_m2)
#endif
#if ALTIVEC_TEST_FUNCTION(variance)
  BENCHMARK_INFO(variance)
#endif
#if ALTIVEC_TEST_FUNCTION(sub_pred)
  BENCHMARK_INFO(sub_pred)
#endif
#if ALTIVEC_TEST_FUNCTION(add_pred)
  BENCHMARK_INFO(add_pred)
#endif
#if ALTIVEC_TEST_FUNCTION(pred_comp)
  BENCHMARK_INFO(pred_comp)
#endif
#if ALTIVEC_TEST_FUNCTION(subsample_image)
  BENCHMARK_INFO(subsample_image)
#endif
#if ALTIVEC_TEST_FUNCTION(sub_mean_reduction)
  BENCHMARK_INFO(sub_mean_reduction)
#endif
#if ALTIVEC_TEST_FUNCTION(fdct)
  BENCHMARK_INFO(fdct)
#endif
#if ALTIVEC_TEST_FUNCTION(idct)
  BENCHMARK_INFO(idct)
#endif
};


int calibrate_benchmark(double precision, double increment, int timelimit,
                        int reqpasses, int *passes, int *iterations,
                        int *calibration, struct timeval *time,
                        struct timeval *last_time)
{
  double secs0, secs1;
  double err = precision;
  int done = 0;

  if (last_time->tv_sec != 0 && last_time->tv_usec != 0)
    {
      /* calculate error amount */
      secs0 = TSECS(time) / increment;
      secs1 = TSECS(last_time);
      if (secs0 > secs1)
        err = secs0 / secs1 - 1;
      else
        err = secs1 / secs0 - 1;
    }

  if (err >= precision)
    {
      *passes = 0;
      *calibration = *iterations;
      *iterations *= increment;
      last_time->tv_sec = time->tv_sec;
      last_time->tv_usec = time->tv_usec;
    }
  else
    {
      (*passes)++;
    }

  if (*passes == reqpasses)
    {
      time->tv_sec = last_time->tv_sec;
      time->tv_usec = last_time->tv_usec;
      done = 1;
    }
  else if (last_time->tv_sec >= timelimit)
    {
      done = 1;
    }

  if (done)
    mjpeg_debug("calibration passes=%d precision=%f time=%d.%06d",
               *passes, err, time->tv_sec, time->tv_usec);

  return done;
}



void update_benchmark_times(const char *name,
                            struct benchmark_times *times,
                            struct timeval *time,
                            const char *fmt, ...)
{
  va_list args;
  char buf[BENCHMARK_ARGS_BUFSIZE];

  /* sprintf function arguments */
  va_start(args, fmt);
  vsnprintf(buf, BENCHMARK_ARGS_BUFSIZE-1, fmt, args);
  va_end(args);

  if (times->total.tv_sec == 0 && times->total.tv_usec == 0) {
    /* first time, copy to both min & max */
    times->min.tv_sec  = times->max.tv_sec  = time->tv_sec;
    times->min.tv_usec = times->max.tv_usec = time->tv_usec;
    strncpy(times->min_args, buf, BENCHMARK_ARGS_BUFSIZE);
    strncpy(times->max_args, buf, BENCHMARK_ARGS_BUFSIZE);
  }
  else if (timercmp(time, &times->min, <))
  {
    times->min.tv_sec  = time->tv_sec;
    times->min.tv_usec = time->tv_usec;
    strncpy(times->min_args, buf, BENCHMARK_ARGS_BUFSIZE);
  }
  else if (timercmp(time, &times->max, >))
  {
    times->max.tv_sec  = time->tv_sec;
    times->max.tv_usec = time->tv_usec;
    strncpy(times->max_args, buf, BENCHMARK_ARGS_BUFSIZE);
  }

  timeradd(time, &times->total, &times->total); /* add difference to total */

  mjpeg_info("time: %d.%06d %s", time->tv_sec, time->tv_usec, buf);
}


void print_benchmark_times(const char* name, struct benchmark_times *times)
{
  mjpeg_info("%s: min=%d.%06d %s", name, times->min.tv_sec,
             times->min.tv_usec, times->min_args);
  mjpeg_info("%s: max=%d.%06d %s", name,
             times->max.tv_sec, times->max.tv_usec, times->max_args);
  mjpeg_info("%s: total=%d.%06d", name,
             times->total.tv_sec, times->total.tv_usec);
}


void print_calibration_stats(int count, char* name[], int *calibration,
                             struct timeval *times)
{
  int i;
  for (i = 0; i < count; i++) {
    mjpeg_info("%s: calibration=%d in %d.%06d seconds", name[i],
               calibration[i], times[i].tv_sec, times[i].tv_usec);
  }
}


void print_benchmark_diff(int count, char* names[], struct timeval *times)
{
  double secs0, secs1, percent;
  char *speed;

  secs0 = TSECS(&times[0]);
  secs1 = TSECS(&times[1]);

  if (secs0 < secs1) {
    percent = (secs1/secs0-1.0)*100.0;
    speed = "faster";
  } else {
    percent = (secs0/secs1-1.0)*100.0;
    speed = "slower";
  }

  mjpeg_info("%s was %.2f%% %s than %s", names[0], percent, speed, names[1]);
}


void print_benchmark_statistics() {
  int i;
  char *names[2];
  struct timeval times[2];

  for (i = 0; i < sizeof(benchmarktab) / sizeof(struct benchmark_info); i++)
  {
     mjpeg_info("%s: counter=%d, iterations=%d", benchmarktab[i].name,
        benchmarktab[i].stats->counter, benchmarktab[i].stats->iterations);
     print_benchmark_times(benchmarktab[i].name,
	                       &(benchmarktab[i].stats->times[0]));    
     print_benchmark_times(benchmarktab[i].cmpname,
	                       &(benchmarktab[i].stats->times[1]));
     names[0] = benchmarktab[i].name;
     names[1] = benchmarktab[i].cmpname;
     times[0].tv_sec = benchmarktab[i].stats->times[0].total.tv_sec;
     times[0].tv_usec = benchmarktab[i].stats->times[0].total.tv_usec;
     times[1].tv_sec = benchmarktab[i].stats->times[1].total.tv_sec;
     times[1].tv_usec = benchmarktab[i].stats->times[1].total.tv_usec;
     print_benchmark_diff(2, names, times);
  }
}

#else

void print_benchmark_statistics() {
    mjpeg_info("AltiVec benchmarking not enabled");
}

#endif
