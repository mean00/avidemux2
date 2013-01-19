// Avisynth v1.0 beta.  Copyright 2000 Ben Rudiak-Gould.
// http://www.math.berkeley.edu/~benrg/avisynth.html

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .

#ifndef __Internal_H__
#define __Internal_H__


#define WIN32_LEAN_AND_MEAN
#include <objbase.h>
#include <vfw.h>
#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <vector>
#define __int64_t long long int
#define in64 (__int64)(unsigned short)


#define ATHLON  // comment this out if using the Intel compiler, or you need Pentium/K6 support

#define DEFAULT_PORT_TO_USE 9999
#define PORT_COMMAND "--port"

typedef unsigned long	Pixel;
typedef unsigned long	Pixel32;
typedef unsigned char Pixel8;
typedef long			PixCoord;
typedef	long			PixDim;
typedef	long			PixOffset;

#pragma hdrstop

#ifndef _MSC_VER
  #define _RPT0(a,b) ((void)0)
  #define _RPT1(a,b,c) ((void)0)
  #define _RPT2(a,b,c,d) ((void)0)
  #define _RPT3(a,b,c,d,e) ((void)0)
  #define _RPT4(a,b,c,d,e,f) ((void)0)
  
  #define _ASSERTE(x) assert(x)
  #include <assert.h>
#else  
  #include <crtdbg.h>
#endif


#include "avisynth.h"


struct AVSFunction {
  const char* name;
  const char* param_types;
  AVSValue (__cdecl *apply)(AVSValue args, void* user_data, IScriptEnvironment* env);
  void* user_data;
};


int RGB2YUV(int rgb);

PClip Create_MessageClip(const char* message, int width, int height,
  int pixel_type, bool shrink, int textcolor, int halocolor, int bgcolor,
  IScriptEnvironment* env);

PClip new_Splice(PClip _child1, PClip _child2, bool realign_sound, IScriptEnvironment* env);
PClip new_SeparateFields(PClip _child, IScriptEnvironment* env);
PClip new_AssumeFrameBased(PClip _child);

void BitBlt(BYTE* dstp, int dst_pitch, const BYTE* srcp, 
            int src_pitch, int row_size, int height);

long GetCPUFlags();


class _PixelClip {
  enum { buffer=320 };
  BYTE clip[256+buffer*2];
public:
  _PixelClip() {  
    memset(clip, 0, buffer);
    for (int i=0; i<256; ++i) clip[i+buffer] = (unsigned char) i;
    memset(clip+buffer+256, 255, buffer);
  }
  BYTE operator()(int i) { return clip[i+buffer]; }
};

extern _PixelClip PixelClip;


template<class ListNode>
static __inline void Relink(ListNode* newprev, ListNode* me, ListNode* newnext) {
  if (me == newprev || me == newnext) return;
  me->next->prev = me->prev;
  me->prev->next = me->next;
  me->prev = newprev;
  me->next = newnext;
  me->prev->next = me->next->prev = me;
}



/*** Inline helper methods ***/


static __inline BYTE ScaledPixelClip(int i) {
  return PixelClip((i+32768) >> 16);
}


static __inline bool IsClose(int a, int b, unsigned threshold) 
  { return (unsigned(a-b+threshold) <= threshold*2); }




#endif  // __Internal_H__
