/***************************************************************************
 pipe_source.cpp  -  description
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
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "windows.h"
#include "avisynth.h"
#include "avspipecomm.h"
#include "cdebug.h"

bool pipe_test(int hr, int hw);
int h_read = -1, h_write = -1;
uint32_t frame_sz = 0;
unsigned char *frame_data = NULL;

#define WAIT_BEFORE_TRY 500
#define OPEN_PIPE(n,h,f,to)   {int j = to*1000/WAIT_BEFORE_TRY, j_old=j;\
  while ((h =\
          open(n,f)) == -1\
         && j--)\
  {\
    DEBUG_PRINTF("error open %s, try %d, errno %d\n",\
           n, j_old - j, errno);\
    Sleep(WAIT_BEFORE_TRY);\
  }\
  }

typedef bool CB_SEND_DATA(int hw, unsigned char *data, int sz);
CB_SEND_DATA *pcb_send_data;

extern "C" __declspec(dllexport) bool __stdcall SetPipeName(const char *piper,
                                                            int hw,
                                                            CB_SEND_DATA *cb_send_data,
                                                            int pipe_timeout)
{
  DEBUG_PRINTF("pipe_source : pipe name %s, pipe_timeout is %d\n",
               piper, pipe_timeout);
  OPEN_PIPE(piper,h_read,O_BINARY|O_RDONLY,pipe_timeout);
  if (h_read != -1)
  {
    uint32_t test_r1 = 0;
    int sz1, j = pipe_timeout*1000/WAIT_BEFORE_TRY, j_old = j;
    while ((sz1 = read(h_read, &test_r1, sizeof(uint32_t))) == 0 && j--)
    {
      DEBUG_PRINTF("pipe_source : try read %d\n", j_old - j);
      Sleep(WAIT_BEFORE_TRY);
    }
    if (sz1 != sizeof(uint32_t))
    {
      DEBUG_PRINTF("pipe_source : cannot read test data - read %d\n", sz1);
      fflush(stdout);
      close (h_read);
      return false;
    }
    if (cb_send_data(hw, (unsigned char*)&test_r1, sz1))
    {
      h_write = hw;
      pcb_send_data = cb_send_data;
      return true;
    }
    else
    {
      DEBUG_PRINTF("pipe_source : cannot write test data\n");
      close (h_read);
    }
  }
  return false;
}

class PipeSource: public IClip {

  VideoInfo vi;
public:
  PipeSource(IScriptEnvironment* env) ;
  virtual ~PipeSource();

  // avisynth virtual functions
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  bool __stdcall GetParity(int n);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {};
  const VideoInfo& __stdcall GetVideoInfo();
  void __stdcall SetCacheHints(int cachehints,int frame_range) {};
};

PipeSource::PipeSource (IScriptEnvironment* env) {

  PIPE_MSG_HEADER msg;
  ADV_Info ai;

  vi.width = 720;
  vi.height = 576;
  vi.fps_numerator = 25;
  vi.fps_denominator = 1;
  vi.SetFieldBased(false);
  vi.audio_samples_per_second=0;
  vi.num_frames = 100;
  vi.pixel_type = VideoInfo::CS_YV12;

  DEBUG_PRINTF("pipe_source : receive clip info, handle %d\n",
         h_read);
  fflush(stdout);
  if (receive_cmd(h_read,
                  &msg))
    if (msg.sz == sizeof(ADV_Info) &&
        msg.avs_cmd == SET_CLIP_PARAMETER)
      if (receive_data(h_read,
                       &msg, &ai))
      {
        DEBUG_PRINTF("pipe_source : receive cmd %d [width %d height %d]\n",
               (int) msg.avs_cmd,
               ai.width, ai.height);
        fflush(stdout);

        vi.width = ai.width;
        vi.height = ai.height;
        vi.num_frames = ai.nb_frames + ai.orgFrame;
        vi.fps_numerator = ai.fps1000;
        vi.fps_denominator = 1000;
        frame_sz = (vi.width * vi.height * 3) >> 1;
        frame_data = (unsigned char*)malloc(frame_sz);
      }
      else
        DEBUG_PRINTF("pipe_source : error receive_data with info\n");
    else
      DEBUG_PRINTF("pipe_source : receive_data return wrong header\n");
  else
    DEBUG_PRINTF("pipe_source : error receive_data with header\n");
}


PipeSource::~PipeSource() {
  DEBUG_PRINTF("Delete PipeSource\n");
  fflush(stdout);
}

PVideoFrame __stdcall PipeSource::GetFrame(int n, IScriptEnvironment* env) {

  PVideoFrame dst;
  dst = env->NewVideoFrame(vi);

  PIPE_MSG_HEADER msg = {GET_FRAME, sizeof(FRAME_DATA)};
  FRAME_DATA fd = {n};

  DEBUG_PRINTF("pipe_source : invoke GetFrame %d [num_frames %d]\n", n, vi.num_frames);
  DEBUG_PRINTF("pipe_source : frame pitch %d\n", dst->GetPitch());
  fflush(stdout);

  // send GET_FRAME to avidemux2/avsfilter
  if (!pcb_send_data(h_write, (unsigned char*)&msg, sizeof(msg)) ||
      !pcb_send_data(h_write, (unsigned char*)&fd, sizeof(fd)))
  {
    DEBUG_PRINTF("pipe_source : error send GET_FRAME to avsfilter\n", n);
    fflush(stdout);
    return dst;
  }

  DEBUG_PRINTF("pipe_source : send GET_FRAME ok\n");
  fflush(stdout);
  
  int test_sz = 0;
  // receive frame from avsfilter
  if (!receive_cmd(h_read, &msg) ||
      msg.avs_cmd != PUT_FRAME ||
      ppread(h_read, &fd, sizeof(fd)) != sizeof(fd) ||
      fd.frame != n || (frame_sz + sizeof(fd)) != msg.sz ||
      (test_sz = ppread(h_read, frame_data, frame_sz)) != frame_sz)
  {
    DEBUG_PRINTF("pipe_source : error get frame, par [code %d sz %d frame %d] chk par [frame_sz %d read_sz %d]\n",
           msg.avs_cmd, msg.sz, fd.frame, frame_sz, test_sz);
    fflush(stdout);
    return dst;
  }

  // copy Y, U and V with pitch
  env->BitBlt(dst->GetWritePtr(), dst->GetPitch(), frame_data, vi.width, vi.width, vi.height);
  env->BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V),
              frame_data + (vi.width * vi.height), vi.width / 2, vi.width / 2, vi.height / 2);
  env->BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U),
              frame_data + (vi.width * vi.height) + ((vi.width * vi.height) >> 2),
              vi.width / 2, vi.width / 2, vi.height / 2);

  DEBUG_PRINTF("pipe_source : return frame %d data ok\n", fd.frame);
  fflush(stdout);

  return dst;
}


bool __stdcall PipeSource::GetParity(int n) { return false; }
const VideoInfo& __stdcall PipeSource::GetVideoInfo() { return vi;}
AVSValue __cdecl Create_PipeSource(AVSValue args, void* user_data, IScriptEnvironment* env) {
  return new PipeSource(env);
}
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{
  env->AddFunction("PipeSource","",Create_PipeSource,0);
  return 0;
}
