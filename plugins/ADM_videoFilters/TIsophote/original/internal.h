// Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
// http://www.avisynth.org

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
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.


#ifndef __Internal_H__
#define __Internal_H__

#define AVS_VERSION 2.54
#define AVS_VERSTR "AviSynth 2.54, build:"__DATE__" ["__TIME__"]"


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

  void asm_BitBlt_ISSE(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height);
  void asm_BitBlt_MMX(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height);

long GetCPUFlags();


class _PixelClip {
  enum { buffer=320 };
  BYTE clip[256+buffer*2];
public:
  _PixelClip() {  
    memset(clip, 0, buffer);
    for (int i=0; i<256; ++i) clip[i+buffer] = i;
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