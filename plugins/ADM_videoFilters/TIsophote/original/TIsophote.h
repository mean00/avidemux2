/*
**                     TIsophote v0.9.1 for AviSynth 2.5.x
**
**   TIsophote is a simple unconstrained level-set (isophote) smoothing filter.
**
**   Copyright (C) 2004 Kevin Stone
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   This program is distributed in the hope that it will be useful,
**   but WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**   GNU General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <windows.h>
#include "internal.h"

class TIsophote : public GenericVideoFilter
{
private:
	int iterations;
	double tStep;
	int type;
	bool chroma;
	int nfrms;
	PVideoFrame __stdcall TIsophote::GetFrameYV12(int n, IScriptEnvironment *env);
	PVideoFrame __stdcall TIsophote::GetFrameYUY2(int n, IScriptEnvironment *env);

public:
	PVideoFrame __stdcall TIsophote::GetFrame(int n, IScriptEnvironment *env);

	TIsophote(PClip _child, int _iterations, double _tStep, int _type, bool _chroma, IScriptEnvironment* env) : 
		GenericVideoFilter(_child), iterations(_iterations), tStep(_tStep), type(_type), chroma(_chroma)
	{
		if (!vi.IsYV12() && !vi.IsYUY2())
			env->ThrowError("TIsophote:  YV12 and YUY2 data only!");
		if (iterations <= 0)
			env->ThrowError("TIsophote:  iterations must be set to at least 1!");
		if (type < 0 || type > 2)
			env->ThrowError("TIsophote:  type must be set to 0, 1, or 2!");
		nfrms = vi.num_frames-1;
	}
	TIsophote::~TIsophote()
	{
		// nothing to free
	}
};