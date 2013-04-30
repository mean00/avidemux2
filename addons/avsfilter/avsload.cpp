/***************************************************************************
 avsload.cpp  -  description

 based on code from avs2yuv project

// Avs2YUV by Loren Merritt

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "internal.h"
#include "avspipecomm.h"
#include "winetmppath.h"
#include "cdebug.h"

#ifdef _MSC_VER
// what's up with MS's std libs?
#define dup _dup
#define popen _popen
#define pclose _pclose
#define fdopen _fdopen
#define setmode _setmode
#else
#include <unistd.h>
#endif

#ifndef INT_MAX
#define INT_MAX 0x7fffffff
#endif

typedef bool CB_SEND_DATA(int h_write, unsigned char *data, int sz);

typedef bool __stdcall SetPipeName(const char *pipe_read,
                                   int hw,
                                   CB_SEND_DATA *cb_send_data,
                                   int pipe_timeout);

bool pipe_test(int hr, int hw);
#define ZPREVENT_INTERMEDIATE_BUFFERS

#define BUF_SIZE (1920L * 1080L * 3L >> 1L)
//#define BUF_SIZE 65536

void test_pipe_speed(int h_read, int h_write,
                     char *copy_buf, int buf_sz, int time_sec)
{
  char cmd = 0;
  while (read(h_read, &cmd, sizeof(cmd)) == sizeof(cmd) && cmd != 'E')
  {
    int real_write = 0;
//    printf("Test_pipe : cmd is %C\n", cmd);
    switch(cmd)
    {
      case 'R':
        //buf_sz
        if ((real_write = write(h_write, copy_buf, buf_sz)) != buf_sz)
        {
          printf("Error write data to pipe\n"
                 "[real write only %d bytes, but need %d bytes errno %d]\n",
                 real_write, buf_sz, errno);
          printf("Error %d\n", GetLastError());
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

  __int64  zero_time = GetTickCount(), cur_time;
  __int64 loop_counter = 0;

  do {
    cmd = 'R';
    if (write(h_write, &cmd, sizeof(cmd)) != sizeof(cmd))
    {
      printf("Error write cmd to pipe\n");
      return;
    }

    if (read(h_read, copy_buf, buf_sz) != buf_sz)
    {
      printf("Error read data from pipe\n");
      return;
    }

    loop_counter++;
    cur_time = GetTickCount();
  } while ((cur_time - zero_time) < time_sec * 1000);

  printf("Loop counter %d\nTime delta %lu\n"
         "Data copy across memory is %lu Kb/sec\n",
         (int)loop_counter, (unsigned long)(cur_time - zero_time),
         (unsigned long)(((loop_counter * buf_sz * 1000L) / 1024L) / (cur_time - zero_time)));

  cmd = 'E';
  if (write(h_write, &cmd, sizeof(cmd)) != sizeof(cmd))
  {
    printf("Error write cmd to pipe\n");
    return;
  }
}

bool cb_send_data(int h_write, unsigned char *data, int sz)
{
//  printf("call cb_send_data\n");
  return (write(h_write, data, sz) == sz);
}

bool load_avisynth(IScriptEnvironment **env, HMODULE *avsdll)
{
  try{
    *avsdll = LoadLibrary("avisynth.dll");
    if(!*avsdll)
    {
      DEBUG_PRINTF("avsloader : failed to load avisynth.dll\n");
      fflush(stdout);
      return false;
    }

    IScriptEnvironment* (* CreateScriptEnvironment)(int version)
        = (IScriptEnvironment*(*)(int)) GetProcAddress(*avsdll, "CreateScriptEnvironment");
    if(!CreateScriptEnvironment)
    {
      DEBUG_PRINTF("avsloader : failed to load CreateScriptEnvironment()\n");
      fflush(stdout);

      free_lib:
      FreeLibrary(*avsdll);
      return false;
    }

    *env = CreateScriptEnvironment(AVISYNTH_INTERFACE_VERSION);
  }
  catch(AvisynthError err)
  {
    DEBUG_PRINTF("avsloader : avisynth error %s\n", err.msg);
    return false;
  }
  return true;
}

bool load_avs(char *infile, PClip &clip,
              IScriptEnvironment *env)
{
  try{
    AVSValue arg(infile);
    AVSValue res = env->Invoke("Import", AVSValue(&arg, 1));

    if(!res.IsClip())
    {
      DEBUG_PRINTF("avsloader : '%s' didn't return a video clip.\n", infile);
      fflush(stdout);
      return false;
    }

    clip = res.AsClip();
  }
  catch(AvisynthError err)
  {
    DEBUG_PRINTF("avsloader : avisynth error %s\n", err.msg);
    return false;
  }
  return true;
}

bool load_pipe_dll(const char *dllname,
                   const char *n_read,
                   int h_write,
                   CB_SEND_DATA *cb_send_data,
                   int pipe_timeout)
{
  HMODULE pipe_dll = LoadLibrary(dllname);
  bool ret = false;

  if(!pipe_dll)
  {
    DEBUG_PRINTF("failed to load %s\n", dllname);
    fflush(stdout);
    return false;
  }

  SetPipeName *set_pipe_name = (SetPipeName*) GetProcAddress(pipe_dll,
                                                             "SetPipeName");
  if(!set_pipe_name)
  {
    DEBUG_PRINTF("failed to get SetPipeName\n");
    fflush(stdout);

    free_lib:
    FreeLibrary(pipe_dll);
    return false;
  }

  ret = set_pipe_name(n_read, h_write, cb_send_data, pipe_timeout);

  if (!ret) FreeLibrary(pipe_dll);

  return ret;
}

#define PIPE_LOADER_READ 1
#define PIPE_LOADER_WRITE 0
#define PIPE_FILTER_READ 2

/*
           void BitBlt(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height) {
           if ( (!height)|| (!row_size)) return;
           if (GetCPUFlags() & CPUF_INTEGER_SSE) {
           if (height == 1 || (src_pitch == dst_pitch && dst_pitch == row_size)) {
           memcpy_amd(dstp, srcp, row_size*height);
           } else {
           asm_BitBlt_ISSE(dstp,dst_pitch,srcp,src_pitch,row_size,height);
           }
           return;
           }
           if (height == 1 || (dst_pitch == src_pitch && src_pitch == row_size)) {
           memcpy(dstp, srcp, row_size*height);
           } else {
           for (int y=height; y>0; --y) {
           memcpy(dstp, srcp, row_size);
           dstp += dst_pitch;
           srcp += src_pitch;
           }
           }

           */

bool send_bit_blt(int h,
                  const BYTE* srcp, int src_pitch,
                  int row_size, int height, BYTE *data)
{
  BYTE *org_data = data;
  for (int y = height; y > 0; --y)
  {
    int cp_sz;
    memcpy(data, (void*)srcp, row_size);
    srcp += src_pitch;
    data += row_size;
    cp_sz = data - org_data;
    if ( cp_sz >= PIPE_MAX_TRANSFER_SZ || y > 0)
      if (ppwrite(h, (void*)org_data, cp_sz) != cp_sz) return false;
      else data = org_data;
  }
  return true;
}

void processing_frames(AVS_PIPES *avs_pipes)
{
  PIPE_MSG_HEADER msg;
  int mem_sz = 0;
  unsigned char *data = NULL;
  PClip clip;
  IScriptEnvironment *env = NULL;
  VideoInfo inf;
  FRAME_DATA *fd, fd_copy;
  uint32_t frame_sz;
  HMODULE avsdll = NULL;

  if (!load_avisynth(&env, &avsdll))
  {
    DEBUG_PRINTF("avsloader : cannot load avisynth dll\n");
    fflush(stdout);
    return;
  }

  bool terminate = false;
  while(!terminate)
  {
    // get current command
    if (receive_cmd(avs_pipes[PIPE_LOADER_READ].hpipe,
                    &msg))
    {
      DEBUG_PRINTF("avsloader : receive loop\n");
      fflush(stdout);

      if (mem_sz < msg.sz)
      {
        if (data) free(data);
        data = (unsigned char *)malloc (msg.sz);
        if (!data) break;
        mem_sz = msg.sz;
      }

      // if current command have a data, then get it
      if (msg.sz)
        if (!receive_data(avs_pipes[PIPE_LOADER_READ].hpipe,
                         &msg, data)) break;

      DEBUG_PRINTF("avsloader : receive cmd %d\n",
             (int) msg.avs_cmd);
      fflush(stdout);

      switch(msg.avs_cmd)
      {
        case LOAD_AVS_SCRIPT:

          DEBUG_PRINTF("avsloader : try load avisynth.dll and script %s\n",
                 data);
          fflush(stdout);
          if (!load_avs((char*)data, clip, env)) return;

          // get video info
          inf = clip->GetVideoInfo();
          DEBUG_PRINTF("avsloader script ok -> %s: %dx%d\n", data, inf.width, inf.height);
          fflush(stdout);

          // get frame size and alloc it
          frame_sz = (inf.width*inf.height*3)>>1;
          if (data) free(data);
          data = (unsigned char*)malloc(frame_sz);
          mem_sz = frame_sz;

          // send out clip information to avsfilter
          ADV_Info info;
          info.width = inf.width;
          info.height = inf.height;
          info.nb_frames = inf.num_frames;
          info.fps1000 = inf.fps_numerator * 1000 / inf.fps_denominator;
          info.orgFrame = 0;

          if (!send_cmd(avs_pipes[PIPE_LOADER_WRITE].hpipe,
                        SET_CLIP_PARAMETER,
                        (void*)&info, sizeof(ADV_Info)))
          {
            DEBUG_PRINTF("avsloader : cannot send avisynth clip param to avsfilter\n");
            fflush(stdout);
            return;
          }

          break;

        case UNLOAD_AVS_SCRIPT:
          delete clip;
          DEBUG_PRINTF("avsloader : UNLOAD_AVS_SCRIPT ok\n");
          break;

        case UNLOAD_AVS_LOADER:
          DEBUG_PRINTF("avsloader : UNLOAD_AVS_LOADER ok\n");
          terminate = true;
          break;

        case GET_FRAME:
          // avidemux2 avsfilter tell : get frame
          fd = (FRAME_DATA *)data;
          DEBUG_PRINTF("avsloader : receive GET_FRAME %lu\n", fd->frame);
          fflush(stdout);

          /*inf = clip->GetVideoInfo();
          DEBUG_PRINTF("avsloader recheck infos %dx%d\n", inf.width, inf.height);
          fflush(stdout);*/

          // avsloader tell : avisynth, please, get frame number fd->frame
          PVideoFrame f = clip->GetFrame(fd->frame, env);
          //DEBUG_PRINTF("avsloader : avisynth return frame data\n");
          /*DEBUG_PRINTF("avsloader : row %d pitch %d\n",
           f->GetRowSize(), f->GetPitch());*/
          DEBUG_PRINTF("avsloader : GetFrame %d ok\n", fd->frame);
          fflush(stdout);

          fd_copy = *fd;
          /*
           void BitBlt(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height) {
           if ( (!height)|| (!row_size)) return;
           if (GetCPUFlags() & CPUF_INTEGER_SSE) {
           if (height == 1 || (src_pitch == dst_pitch && dst_pitch == row_size)) {
           memcpy_amd(dstp, srcp, row_size*height);
           } else {
           asm_BitBlt_ISSE(dstp,dst_pitch,srcp,src_pitch,row_size,height);
           }
           return;
           }
           if (height == 1 || (dst_pitch == src_pitch && src_pitch == row_size)) {
           memcpy(dstp, srcp, row_size*height);
           } else {
           for (int y=height; y>0; --y) {
           memcpy(dstp, srcp, row_size);
           dstp += dst_pitch;
           srcp += src_pitch;
           }
           }
           */
#ifdef PREVENT_INTERMEDIATE_BUFFERS
          if (!send_cmd_with_specified_size(avs_pipes[PIPE_LOADER_WRITE].hpipe,
                                            PUT_FRAME,
                                            (void*)&fd_copy, sizeof(FRAME_DATA),
                                            frame_sz))
          {
            DEBUG_PRINTF("avsloader : error send frame header to avsfilter\n");
            fflush(stdout);
          }
          else
            if (!send_bit_blt(avs_pipes[PIPE_LOADER_WRITE].hpipe,
                              f->GetReadPtr(), f->GetPitch(),
                              inf.width, inf.height, data) ||
                !send_bit_blt(avs_pipes[PIPE_LOADER_WRITE].hpipe,
                              f->GetReadPtr(PLANAR_V), f->GetPitch(PLANAR_V),
                              inf.width / 2, inf.height / 2, data) ||
                !send_bit_blt(avs_pipes[PIPE_LOADER_WRITE].hpipe,
                              f->GetReadPtr(PLANAR_U), f->GetPitch(PLANAR_U),
                              inf.width / 2, inf.height / 2, data))
            {
              DEBUG_PRINTF("avsloader : error send frame data to avsfilter\n");
              fflush(stdout);
            }
#else

          // copy Y, U and V with pitch
          env->BitBlt(data, inf.width, f->GetReadPtr(), f->GetPitch(), inf.width, inf.height);
          env->BitBlt(data + (inf.width * inf.height), inf.width / 2,
                      f->GetReadPtr(PLANAR_V), f->GetPitch(PLANAR_V),
                      inf.width / 2, inf.height / 2);
          env->BitBlt(data + (inf.width * inf.height) + ((inf.width * inf.height) >> 2),
                      inf.width / 2, f->GetReadPtr(PLANAR_U), f->GetPitch(PLANAR_U),
                      inf.width / 2, inf.height / 2);

          // return filtered data
          if (!send_cmd_by_two_part(avs_pipes[PIPE_LOADER_WRITE].hpipe,
                                    PUT_FRAME,
                                    (void*)&fd_copy, sizeof(FRAME_DATA),
                                    (void*)data, frame_sz))
          {
            DEBUG_PRINTF("avsloader : error send frame to avsfilter\n");
            fflush(stdout);
            return;
          }
#endif
          break;
      }

    }
    else terminate = true;
  }
  DEBUG_PRINTF("avsloader : Try exit\n");
//  if (avsdll) FreeLibrary(avsdll);
//  DEBUG_PRINTF("avsloader : avsdll unload\n");
//  if (data) free(data);
//  DEBUG_PRINTF("avsloader : data free\n");
}

int __cdecl main(int argc, const char* argv[])
{
  if (argc == 2)
  {
    int pipe_timeout = 10;
    AVS_PIPES avs_pipes[3] =
    {
      {"", -1, O_BINARY | O_WRONLY},
      {"", -1, O_BINARY | O_RDONLY},
      {"", -1, O_BINARY | O_RDONLY}
    };

    wine_tmp_path (avs_pipes, sizeof(avs_pipes) / sizeof(AVS_PIPES));

    int i;
    bool rc_error = false;
    char *cpb;
    time_t t = time(NULL);
    printf("avsloader : %s %s %s %s\n",
           ctime(&t),
           avs_pipes[0].pipename,
           avs_pipes[1].pipename,
           avs_pipes[2].pipename);

    fflush(stdout);

#define WAIT_BEFORE_TRY 500

    Sleep (WAIT_BEFORE_TRY * 2);
    sscanf(argv[1], "%d", &pipe_timeout);
    DEBUG_PRINTF("avsloader : pipe_timeout is %d, pipe handles ", pipe_timeout);
    for (i = 0; i < 2; i++)
    {
      int j = (pipe_timeout * 1000) / WAIT_BEFORE_TRY, j_old = j;
      while ((avs_pipes[i].hpipe =
              open(avs_pipes[i].pipename, avs_pipes[i].flags)) == -1
             && j--)
      {
        DEBUG_PRINTF("error open %s, try %d, errno %d\n",
               avs_pipes[i].pipename, j_old - j, errno);
        Sleep(WAIT_BEFORE_TRY);
      }
      rc_error = (avs_pipes[i].hpipe == -1);
      DEBUG_PRINTF("%d ", avs_pipes[i].hpipe);
    }
    DEBUG_PRINTF("\n");
    fflush(stdout);

    if (rc_error) goto cleanup_close_pipes;

    DEBUG_PRINTF("avsload : start load source filter dll and test filter pipe\n");
    if (!load_pipe_dll("pipe_source.dll",
                       avs_pipes[PIPE_FILTER_READ].pipename,
                       avs_pipes[PIPE_LOADER_WRITE].hpipe,
                       &cb_send_data, pipe_timeout))
    {
      DEBUG_PRINTF("avsload : cannot call SetPipeHandle\n");
      goto cleanup_close_pipes;
    }

    DEBUG_PRINTF("avsload : Test loader pipes\n");
    if (!pipe_test (avs_pipes[PIPE_LOADER_READ].hpipe,
                    avs_pipes[PIPE_LOADER_WRITE].hpipe))
    {
      DEBUG_PRINTF("avsload : cannot test pipe from loader to avsfilter\n");
      goto cleanup_close_pipes;

    }
    DEBUG_PRINTF("avsload : test pipe from loader to avsfilter ok\n");
    fflush(stdout);

#if 0
    DEBUG_PRINTF("Special speed testing\n");
    fflush(stdout);
    cpb = (char*)malloc(BUF_SIZE);
    test_pipe_speed(avs_pipes[PIPE_LOADER_READ].hpipe,
                    avs_pipes[PIPE_LOADER_WRITE].hpipe,
                    cpb, 512, 1);

    test_pipe_speed(avs_pipes[PIPE_LOADER_READ].hpipe,
                    avs_pipes[PIPE_LOADER_WRITE].hpipe,
                    cpb, BUF_SIZE, 10);
    fflush(stdout);
#endif

    //setdbglog ("avsload.log");
    processing_frames(avs_pipes);

    cleanup_close_pipes:

    DEBUG_PRINTF("avsload : try exit\n");

    for (i = 0; i < 3; i++)
    {
      if (avs_pipes[i].hpipe != -1) close(avs_pipes[i].hpipe);
    }
  }

  DEBUG_PRINTF("avsload : really exit\n");

  return 0;
}
