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

#include "TIsophote.h"

PVideoFrame __stdcall TIsophote::GetFrame(int n, IScriptEnvironment* env) 
{
	if (n<0) n = 0;
	else if (n>nfrms) n = nfrms;
	if (vi.IsYV12()) return(GetFrameYV12(n, env));
	else return(GetFrameYUY2(n, env));
}

PVideoFrame __stdcall TIsophote::GetFrameYV12(int n, IScriptEnvironment* env) 
{
	PVideoFrame src = child->GetFrame(n, env);
	PVideoFrame dst1 = env->NewVideoFrame(vi);
	PVideoFrame dst2 = env->NewVideoFrame(vi);
	const unsigned char *srcp1 = src->GetReadPtr(PLANAR_Y);
	int src1_pitch = src->GetPitch(PLANAR_Y);
	int width = src->GetRowSize(PLANAR_Y) - 1;
	int height = src->GetHeight(PLANAR_Y) - 1;
	unsigned char *srcpp, *srcp, *srcpn;
	unsigned char *dstp1 = dst1->GetWritePtr(PLANAR_Y);
	int dst1_pitch = dst1->GetPitch(PLANAR_Y);
	unsigned char *dstp2 = dst2->GetWritePtr(PLANAR_Y);
	int dst2_pitch = dst2->GetPitch(PLANAR_Y);
	unsigned char *dstp;
	double off = 0.0000000001;
	int x, y, b, dst_pitch, src_pitch, temp;
	int Ix, Ix2, Iy, Iy2, Ixy, Ixx, Iyy;
	env->BitBlt(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
	env->BitBlt(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
	for (b=0; b<iterations; ++b)
	{
		if (b&1)
		{
			srcp = dstp2 + dst2_pitch;
			src_pitch = dst2_pitch;
			srcpp = srcp - src_pitch;
			srcpn = srcp + src_pitch;
			dstp = dstp1 + dst1_pitch;
			dst_pitch = dst1_pitch;
		}
		else
		{
			srcp = dstp1 + dst1_pitch;
			src_pitch = dst1_pitch;
			srcpp = srcp - src_pitch;
			srcpn = srcp + src_pitch;
			dstp = dstp2 + dst2_pitch;
			dst_pitch = dst2_pitch;
		}
		if (type == 0)
		{
			for (y=1; y<height; ++y)
			{
				for (x=1; x<width; ++x)
				{
					Ix = srcp[x+1] - srcp[x-1];
					Ix2 = Ix*Ix;
					Iy = srcpn[x] - srcpp[x];
					Iy2 = Iy*Iy;
					Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
					Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
					Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
					temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
					if (temp > 255) temp = 255;
					else if (temp < 0) temp = 0;
					dstp[x] = temp;
				}
				srcpp += src_pitch;
				srcp += src_pitch;
				srcpn += src_pitch;
				dstp += dst_pitch;
			}
		}
		else if (type == 1)
		{
			for (y=1; y<height; ++y)
			{
				for (x=1; x<width; ++x)
				{
					Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
					Ix2 = Ix*Ix;
					Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
					Iy2 = Iy*Iy;
					Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
					Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
					Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
					temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
					if (temp > 255) temp = 255;
					else if (temp < 0) temp = 0;
					dstp[x] = temp;
				}
				srcpp += src_pitch;
				srcp += src_pitch;
				srcpn += src_pitch;
				dstp += dst_pitch;
			}
		}
		else
		{
			for (y=1; y<height; ++y)
			{
				for (x=1; x<width; ++x)
				{
					Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
					Ix2 = Ix*Ix;
					Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
					Iy2 = Iy*Iy;
					Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
					Ixx = srcpp[x+1] + srcp[x+1] + srcp[x+1] + srcpn[x+1] + srcpp[x-1] + srcp[x-1] + 
						srcp[x-1] + srcpn[x-1] - (srcpp[x]<<1) - (srcp[x]<<2) - (srcpn[x]<<1);
					Iyy = srcpp[x-1] + srcpp[x] + srcpp[x] + srcpp[x+1] + srcpn[x-1] + srcpn[x] + 
						srcpn[x] + srcpn[x+1] - (srcp[x-1]<<1) - (srcp[x]<<2) - (srcp[x+1]<<1);
					temp = srcp[x] + ((int)((((Ix2*Iyy - 4*Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 3) + off)) + 0.5f));
					if (temp > 255) temp = 255;
					else if (temp < 0) temp = 0;
					dstp[x] = temp;
				}
				srcpp += src_pitch;
				srcp += src_pitch;
				srcpn += src_pitch;
				dstp += dst_pitch;
			}
		}
	}
	srcp1 = src->GetReadPtr(PLANAR_U);
	src1_pitch = src->GetPitch(PLANAR_U);
	width = src->GetRowSize(PLANAR_U) - 1;
	height = src->GetHeight(PLANAR_U) - 1;
	dstp1 = dst1->GetWritePtr(PLANAR_U);
	dst1_pitch = dst1->GetPitch(PLANAR_U);
	dstp2 = dst2->GetWritePtr(PLANAR_U);
	dst2_pitch = dst2->GetPitch(PLANAR_U);
	if (chroma)
	{
		env->BitBlt(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
		env->BitBlt(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
		for (b=0; b<iterations; ++b)
		{
			if (b&1)
			{
				srcp = dstp2 + dst2_pitch;
				src_pitch = dst2_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp1 + dst1_pitch;
				dst_pitch = dst1_pitch;
			}
			else
			{
				srcp = dstp1 + dst1_pitch;
				src_pitch = dst1_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp2 + dst2_pitch;
				dst_pitch = dst2_pitch;
			}
			if (type == 0)
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcp[x+1] - srcp[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x] - srcpp[x];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else if (type == 1)
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcpp[x+1] + srcp[x+1] + srcp[x+1] + srcpn[x+1] + srcpp[x-1] + srcp[x-1] + 
							srcp[x-1] + srcpn[x-1] - (srcpp[x]<<1) - (srcp[x]<<2) - (srcpn[x]<<1);
						Iyy = srcpp[x-1] + srcpp[x] + srcpp[x] + srcpp[x+1] + srcpn[x-1] + srcpn[x] + 
							srcpn[x] + srcpn[x+1] - (srcp[x-1]<<1) - (srcp[x]<<2) - (srcp[x+1]<<1);
						temp = srcp[x] + ((int)((((Ix2*Iyy - 4*Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 3) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
		}
		srcp1 = src->GetReadPtr(PLANAR_V);
		dstp1 = dst1->GetWritePtr(PLANAR_V);
		dstp2 = dst2->GetWritePtr(PLANAR_V);
		env->BitBlt(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
		env->BitBlt(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
		for (b=0; b<iterations; ++b)
		{
			if (b&1)
			{
				srcp = dstp2 + dst2_pitch;
				src_pitch = dst2_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp1 + dst1_pitch;
				dst_pitch = dst1_pitch;
			}
			else
			{
				srcp = dstp1 + dst1_pitch;
				src_pitch = dst1_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp2 + dst2_pitch;
				dst_pitch = dst2_pitch;
			}
			if (type == 0)
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcp[x+1] - srcp[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x] - srcpp[x];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else if (type == 1)
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcp[x+1] + srcp[x-1] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else
			{
				for (y=1; y<height; ++y)
				{
					for (x=1; x<width; ++x)
					{
						Ix = srcpp[x+1]+srcp[x+1]+srcp[x+1]+srcpn[x+1]-srcpp[x-1]-srcp[x-1]-srcp[x-1]-srcpn[x-1];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-1]+srcpn[x]+srcpn[x]+srcpn[x+1]-srcpp[x-1]-srcpp[x]-srcpp[x]-srcpp[x+1];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-1] - srcpp[x+1] - srcpn[x-1] + srcpn[x+1];
						Ixx = srcpp[x+1] + srcp[x+1] + srcp[x+1] + srcpn[x+1] + srcpp[x-1] + srcp[x-1] + 
							srcp[x-1] + srcpn[x-1] - (srcpp[x]<<1) - (srcp[x]<<2) - (srcpn[x]<<1);
						Iyy = srcpp[x-1] + srcpp[x] + srcpp[x] + srcpp[x+1] + srcpn[x-1] + srcpn[x] + 
							srcpn[x] + srcpn[x+1] - (srcp[x-1]<<1) - (srcp[x]<<2) - (srcp[x+1]<<1);
						temp = srcp[x] + ((int)((((Ix2*Iyy - 4*Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 3) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
		}
	}
	else
	{
		if (iterations&1) env->BitBlt(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
		else env->BitBlt(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
		srcp1 = src->GetReadPtr(PLANAR_V);
		dstp1 = dst1->GetWritePtr(PLANAR_V);
		dstp2 = dst2->GetWritePtr(PLANAR_V);
		if (iterations&1) env->BitBlt(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
		else env->BitBlt(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
	}
	if (iterations&1) return dst2;
	return dst1;
}

PVideoFrame __stdcall TIsophote::GetFrameYUY2(int n, IScriptEnvironment* env) 
{
	PVideoFrame src = child->GetFrame(n, env);
	PVideoFrame dst1 = env->NewVideoFrame(vi);
	PVideoFrame dst2 = env->NewVideoFrame(vi);
	const unsigned char *srcp1 = src->GetReadPtr();
	int src1_pitch = src->GetPitch(PLANAR_Y);
	int width = src->GetRowSize() - 1;
	int height = src->GetHeight() - 1;
	unsigned char *srcpp, *srcp, *srcpn;
	unsigned char *dstp1 = dst1->GetWritePtr();
	int dst1_pitch = dst1->GetPitch();
	unsigned char *dstp2 = dst2->GetWritePtr();
	int dst2_pitch = dst2->GetPitch();
	unsigned char *dstp;
	double off = 0.0000000001;
	int x, y, b, dst_pitch, src_pitch, temp;
	int Ix, Ix2, Iy, Iy2, Ixy, Ixx, Iyy;
	env->BitBlt(dstp1,dst1_pitch,srcp1,src1_pitch,width+1,height+1);
	env->BitBlt(dstp2,dst2_pitch,srcp1,src1_pitch,width+1,height+1);
	if (!chroma)
	{
		for (b=0; b<iterations; ++b)
		{
			if (b&1)
			{
				srcp = dstp2 + dst2_pitch;
				src_pitch = dst2_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp1 + dst1_pitch;
				dst_pitch = dst1_pitch;
			}
			else
			{
				srcp = dstp1 + dst1_pitch;
				src_pitch = dst1_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp2 + dst2_pitch;
				dst_pitch = dst2_pitch;
			}
			if (type == 0)
			{
				for (y=1; y<height; ++y)
				{
					for (x=2; x<width; x+=2)
					{
						Ix = srcp[x+2] - srcp[x-2];
						Ix2 = Ix*Ix;
						Iy = srcpn[x] - srcpp[x];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-2] - srcpp[x+2] - srcpn[x-2] + srcpn[x+2];
						Ixx = srcp[x+2] + srcp[x-2] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else if (type == 1)
			{
				for (y=1; y<height; ++y)
				{
					for (x=2; x<width; x+=2)
					{
						Ix = srcpp[x+2]+srcp[x+2]+srcp[x+2]+srcpn[x+2]-srcpp[x-2]-srcp[x-2]-srcp[x-2]-srcpn[x-2];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-2]+srcpn[x]+srcpn[x]+srcpn[x+2]-srcpp[x-2]-srcpp[x]-srcpp[x]-srcpp[x+2];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-2] - srcpp[x+2] - srcpn[x-2] + srcpn[x+2];
						Ixx = srcp[x+2] + srcp[x-2] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else
			{
				for (y=1; y<height; ++y)
				{
					for (x=2; x<width; x+=2)
					{
						Ix = srcpp[x+2]+srcp[x+2]+srcp[x+2]+srcpn[x+2]-srcpp[x-2]-srcp[x-2]-srcp[x-2]-srcpn[x-2];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-2]+srcpn[x]+srcpn[x]+srcpn[x+2]-srcpp[x-2]-srcpp[x]-srcpp[x]-srcpp[x+2];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-2] - srcpp[x+2] - srcpn[x-2] + srcpn[x+2];
						Ixx = srcpp[x+2] + srcp[x+2] + srcp[x+2] + srcpn[x+2] + srcpp[x-2] + srcp[x-2] + 
							srcp[x-2] + srcpn[x-2] - (srcpp[x]<<1) - (srcp[x]<<2) - (srcpn[x]<<1);
						Iyy = srcpp[x-2] + srcpp[x] + srcpp[x] + srcpp[x+2] + srcpn[x-2] + srcpn[x] + 
							srcpn[x] + srcpn[x+2] - (srcp[x-2]<<1) - (srcp[x]<<2) - (srcp[x+2]<<1);
						temp = srcp[x] + ((int)((((Ix2*Iyy - 4*Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 3) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
		}
	}
	else
	{
		width -= 3;
		for (b=0; b<iterations; ++b)
		{
			if (b&1)
			{
				srcp = dstp2 + dst2_pitch;
				src_pitch = dst2_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp1 + dst1_pitch;
				dst_pitch = dst1_pitch;
			}
			else
			{
				srcp = dstp1 + dst1_pitch;
				src_pitch = dst1_pitch;
				srcpp = srcp - src_pitch;
				srcpn = srcp + src_pitch;
				dstp = dstp2 + dst2_pitch;
				dst_pitch = dst2_pitch;
			}
			if (type == 0)
			{
				for (y=1; y<height; ++y)
				{
					for (x=4; x<width; ++x)
					{
						Ix = srcp[x+2] - srcp[x-2];
						Ix2 = Ix*Ix;
						Iy = srcpn[x] - srcpp[x];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-2] - srcpp[x+2] - srcpn[x-2] + srcpn[x+2];
						Ixx = srcp[x+2] + srcp[x-2] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
						++x;
						Ix = srcp[x+4] - srcp[x-4];
						Ix2 = Ix*Ix;
						Iy = srcpn[x] - srcpp[x];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-4] - srcpp[x+4] - srcpn[x-4] + srcpn[x+4];
						Ixx = srcp[x+4] + srcp[x-4] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else if (type == 1)
			{
				for (y=1; y<height; ++y)
				{
					for (x=4; x<width; ++x)
					{
						Ix = srcpp[x+2]+srcp[x+2]+srcp[x+2]+srcpn[x+2]-srcpp[x-2]-srcp[x-2]-srcp[x-2]-srcpn[x-2];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-2]+srcpn[x]+srcpn[x]+srcpn[x+2]-srcpp[x-2]-srcpp[x]-srcpp[x]-srcpp[x+2];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-2] - srcpp[x+2] - srcpn[x-2] + srcpn[x+2];
						Ixx = srcp[x+2] + srcp[x-2] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
						++x;
						Ix = srcpp[x+4]+srcp[x+4]+srcp[x+4]+srcpn[x+4]-srcpp[x-4]-srcp[x-4]-srcp[x-4]-srcpn[x-4];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-4]+srcpn[x]+srcpn[x]+srcpn[x+4]-srcpp[x-4]-srcpp[x]-srcpp[x]-srcpp[x+4];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-4] - srcpp[x+4] - srcpn[x-4] + srcpn[x+4];
						Ixx = srcp[x+4] + srcp[x-4] - srcp[x] - srcp[x];
						Iyy = srcpp[x] + srcpn[x] - srcp[x] - srcp[x];
						temp = srcp[x] + ((int)((((Ix2*Iyy - Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 1) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
			else
			{
				for (y=1; y<height; ++y)
				{
					for (x=4; x<width; ++x)
					{
						Ix = srcpp[x+2]+srcp[x+2]+srcp[x+2]+srcpn[x+2]-srcpp[x-2]-srcp[x-2]-srcp[x-2]-srcpn[x-2];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-2]+srcpn[x]+srcpn[x]+srcpn[x+2]-srcpp[x-2]-srcpp[x]-srcpp[x]-srcpp[x+2];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-2] - srcpp[x+2] - srcpn[x-2] + srcpn[x+2];
						Ixx = srcpp[x+2] + srcp[x+2] + srcp[x+2] + srcpn[x+2] + srcpp[x-2] + srcp[x-2] + 
							srcp[x-2] + srcpn[x-2] - (srcpp[x]<<1) - (srcp[x]<<2) - (srcpn[x]<<1);
						Iyy = srcpp[x-2] + srcpp[x] + srcpp[x] + srcpp[x+2] + srcpn[x-2] + srcpn[x] + 
							srcpn[x] + srcpn[x+2] - (srcp[x-2]<<1) - (srcp[x]<<2) - (srcp[x+2]<<1);
						temp = srcp[x] + ((int)((((Ix2*Iyy - 4*Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 3) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
						++x;
						Ix = srcpp[x+4]+srcp[x+4]+srcp[x+4]+srcpn[x+4]-srcpp[x-4]-srcp[x-4]-srcp[x-4]-srcpn[x-4];
						Ix2 = Ix*Ix;
						Iy = srcpn[x-4]+srcpn[x]+srcpn[x]+srcpn[x+4]-srcpp[x-4]-srcpp[x]-srcpp[x]-srcpp[x+4];
						Iy2 = Iy*Iy;
						Ixy = srcpp[x-4] - srcpp[x+4] - srcpn[x-4] + srcpn[x+4];
						Ixx = srcpp[x+4] + srcp[x+4] + srcp[x+4] + srcpn[x+4] + srcpp[x-4] + srcp[x-4] + 
							srcp[x-4] + srcpn[x-4] - (srcpp[x]<<1) - (srcp[x]<<2) - (srcpn[x]<<1);
						Iyy = srcpp[x-4] + srcpp[x] + srcpp[x] + srcpp[x+4] + srcpn[x-4] + srcpn[x] + 
							srcpn[x] + srcpn[x+4] - (srcp[x-4]<<1) - (srcp[x]<<2) - (srcp[x+4]<<1);
						temp = srcp[x] + ((int)((((Ix2*Iyy - 4*Ix*Iy*Ixy + Iy2*Ixx)*tStep) / (((Ix2 + Iy2) << 3) + off)) + 0.5f));
						if (temp > 255) temp = 255;
						else if (temp < 0) temp = 0;
						dstp[x] = temp;
					}
					srcpp += src_pitch;
					srcp += src_pitch;
					srcpn += src_pitch;
					dstp += dst_pitch;
				}
			}
		}
	}
	if (iterations&1) return dst2;
	return dst1;
}

AVSValue __cdecl Create_TIsophote(AVSValue args, void* user_data, IScriptEnvironment* env) 
{
	int iterations = 4;
	double tStep = 0.2f;
	int type = 2;
	bool chroma = false;
    return new TIsophote(args[0].AsClip(),
						args[1].AsInt(iterations),
						args[2].AsFloat(tStep),
						args[3].AsInt(type),
						args[4].AsBool(chroma),
						env);
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) 
{
    env->AddFunction("TIsophote", "c[iterations]i[tStep]f[type]i[chroma]b", Create_TIsophote, 0);
    return 0;
}