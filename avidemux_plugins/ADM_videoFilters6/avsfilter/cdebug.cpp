/***************************************************************************
 cdebug.cpp  -  description
 -------------------
 begin                : 28-04-2008
 copyright            : (C) 2008 by fahr
 email                : fahr at inbox dot ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef DEBUGMSG
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
FILE *dbglog = stdout;

#define RED_TEXT "\033[31m"
#define END_TEXT "\033[0m"

extern "C" void setdbglog (const char *fname)
{
  FILE *out;
  if ((out = fopen(fname, "w")) != NULL)
    dbglog = out;
}

extern "C" void dbgprintf (const char *format, ...)
{
  time_t t = time(NULL);
  struct tm *tmp = localtime(&t);
  va_list args;
  va_start (args, format);
  if (strcmp(format,"\n"))
   fprintf (dbglog,"%02d:%02d:%02d ", tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
  if (dbglog == stdout) vprintf (format, args);
  else vfprintf (dbglog, format, args);
  fflush(dbglog);
  va_end (args);
}

extern "C" void dbgprintf_RED (const char *format, ...)
{
  va_list args;
  va_start (args, format);
  if (dbglog == stdout) printf(RED_TEXT);
  dbgprintf (format, args);
  if (dbglog == stdout) printf(END_TEXT);
  va_end (args);
}

#endif

#if 0

#define BUF_SIZE (1920L * 1080L * 3L >> 1L)
//#define BUF_SIZE 65536

void test_pipe_speed(int h_read, int h_write,
                     char *copy_buf, int buf_sz, int time_sec)
{
  char cmd = 0;

  unsigned long long  zero_time = clock(), cur_time;
  unsigned long long loop_counter = 0;
  int real_read = 0;

  do {
    cmd = 'R';
    if (write(h_write, &cmd, sizeof(cmd)) != sizeof(cmd))
    {
      printf("Error write cmd to pipe\n");
      return;
    }

    if ((real_read = read(h_read, copy_buf, buf_sz)) != buf_sz)
    {
      printf("Error read data from pipe at loop %d\n"
             "[read %d]\n", loop_counter, real_read);
      return;
    }

    loop_counter++;
    cur_time = clock();
  } while ((cur_time - zero_time) < time_sec * CLOCKS_PER_SEC);

  printf("Loop counter %d\nTime delta %lu\n"
         "Data copy across pipe is %lu Kb/sec\n",
         (int)loop_counter, (unsigned long)(cur_time - zero_time),
         (unsigned long)((((loop_counter * buf_sz * CLOCKS_PER_SEC)) / (cur_time - zero_time))/1024L));

  cmd = 'E';
  if (write(h_write, &cmd, sizeof(cmd)) != sizeof(cmd))
  {
    printf("Error write cmd to pipe\n");
    return;
  }

  cmd = 0;

  while (read(h_read, &cmd, sizeof(cmd)) == sizeof(cmd) && cmd != 'E')
  {
    switch(cmd)
    {
      case 'R':
        if (write(h_write, copy_buf, buf_sz) != buf_sz)
        {
          printf("Error write to pipe\n");
          return;
        }
        break;
      default:
        printf("Bad cmd read\n");
        return;
    }
  }

  if (cmd != 'E')
  {
    printf("Error read cmd from pipe\n");
    return;
  }
}

DEBUG_PRINTF("Special speed testing\n");
#if 0
  char *cpb = (char*)ADM_alloc(BUF_SIZE);
  test_pipe_speed(avs_pipes[PIPE_LOADER_READ].hpipe,
                  avs_pipes[PIPE_LOADER_WRITE].hpipe,
                  cpb, 512, 1);
  test_pipe_speed(avs_pipes[PIPE_LOADER_READ].hpipe,
                  avs_pipes[PIPE_LOADER_WRITE].hpipe,
                  cpb, BUF_SIZE, 10);
#endif

#endif

