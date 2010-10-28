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

#include <math.h>
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidColorYuv.h"

#define BYTE uint8_t
#ifndef x_max
#define x_max(x,y) ((x)>(y) ?(x):(y))
#endif
#ifndef x_min
#define x_min(x,y) ((x)<(y) ?(x):(y))
#endif


/**
    \fn getNextFrame
*/
bool         vidColorYuv::getNextFrame(uint32_t *fn,ADMImage *image)
{
//x	PVideoFrame src;
//x	unsigned long *srcp;
    uint8_t *srcp;
	int pitch, w, h;
	int i,j,wby4;
	int modulo;

#ifdef _DEBUG
	COUNT y0,u0,v0;
	COUNT y,u,v;
	COUNT r,g,b;
	COUNT r0,g0,b0;
	PIXELDATA	pixel0;
	int   total,totalby2;
#endif

#if 0
  if (colorbars) {
    PVideoFrame dst= env->NewVideoFrame(vi);
    int* pdst=(int*)dst->GetWritePtr(PLANAR_Y);
    int Y=16+abs(219-((frame+219)%438));
    Y|=(Y<<8)|(Y<<16)|(Y<<24);
    for (int i = 0;i<224*224;i++)
      pdst[i] = Y;
    unsigned char* pdstb = dst->GetWritePtr(PLANAR_U);
    for (unsigned int y=0;y<224;y++) {
      for (unsigned int x=0;x<224;x++) {
        pdstb[x] = 16+x;
      }
      pdstb += dst->GetPitch(PLANAR_U);
    }

    pdstb = dst->GetWritePtr(PLANAR_V);
    for (y=0;y<224;y++) {
      for (unsigned int x=0;x<224;x++) {
        pdstb[x] = 16+y;
      }
      pdstb += dst->GetPitch(PLANAR_U);
    }
    return dst;
  }
#endif
    if(false==previousFilter->getNextFrame(fn,image))
    {
        return false;
    }
    ADMImage *src=image;
//x	src = child->GetFrame(frame, env);
//x	env->MakeWritable(&src);

//x	srcp = (unsigned long *) src->GetWritePtr();
//x	pitch = src->GetPitch();
    srcp=src->GetWritePtr(PLANAR_Y);
    pitch=src->GetPitch(PLANAR_Y);

	w = info.width;  //x src->GetRowSize();
	h = info.height; //x src->GetHeight();
	wby4 = w / 4;
	modulo = pitch - w;
//	dst = env->NewVideoFrame(vi);
//	dstp = (unsigned long *) dst->GetWritePtr();
//	dpitch = dst->GetPitch();
//	dmodulo = dpitch - dst->GetRowSize();
  if (param.analyze||param.autowhite||param.autogain) {
    unsigned int accum_Y[256],accum_U[256],accum_V[256];
    for (int i=0;i<256;i++) {
      accum_Y[i]=0;
      accum_U[i]=0;
      accum_V[i]=0;
    }
    int uvdiv=1;  //UV divider (ratio between Y and UV pixels)
    if (1) { //x vi.IsPlanar()) {
      uvdiv=4;
	    BYTE* srcp2 = (BYTE*) src->GetReadPtr(PLANAR_Y);
      for (int y=0;y<h;y++) {
        for (int x=0;x<w;x++) {
          accum_Y[srcp2[x]]++;
        }
        srcp2+=pitch;
      }
      pitch = src->GetPitch(PLANAR_U);
	    srcp2 = (BYTE*) src->GetReadPtr(PLANAR_U);
      for (int y=0;y<h/2;y++) {
        for (int x=0;x<w/2;x++) {
          accum_U[srcp2[x]]++;
        }
        srcp2+=pitch;
      }
	    srcp2 = (BYTE*) src->GetReadPtr(PLANAR_V);
      for (int y=0;y<h/2;y++) {
        for (int x=0;x<w/2;x++) {
          accum_V[srcp2[x]]++;
        }
        srcp2+=pitch;
      }
      pitch = src->GetPitch(PLANAR_Y);
    } 
#if 0 
    else {  // YUY2
      uvdiv=2;
      for (int y=0;y<h;y++) {
        for (int x=0;x<wby4;x++) {
          unsigned long p=srcp[x];
          accum_Y[p&0xff]++;
          accum_Y[(p>>16)&0xff]++;
          accum_U[(p>>8)&0xff]++;
          accum_V[(p>>24)&0xff]++;
        }
        srcp+=pitch/4;
      }
      srcp=(unsigned long *)src->GetReadPtr();
    }
#endif
    int pixels = info.width*info.height;
    float avg_u=0, avg_v=0, avg_y=0;
    int x_min_u=0, x_min_v=0, x_min_y=0;
    int x_max_u=0, x_max_v=0, x_max_y=0;
    bool hit_y=false,hit_u=false,hit_v=false;
    int Ax_min_u=0, Ax_min_v=0, Ax_min_y=0;
    int Ax_max_u=0, Ax_max_v=0, Ax_max_y=0;
    bool Ahit_x_miny=false,Ahit_x_minu=false,Ahit_x_minv=false;
    bool Ahit_x_maxy=false,Ahit_x_maxu=false,Ahit_x_maxv=false;
    int At_y2=(pixels/256); // When 1/256th of all pixels have been reached, trigger "Loose x_min/x_max"
    int At_uv2=(pixels/1024); 
   
    for (int i=0;i<256;i++) {
      avg_y+=(float)accum_Y[i]*(float)i;
      avg_u+=(float)accum_U[i]*(float)i;
      avg_v+=(float)accum_V[i]*(float)i;
      if (accum_Y[i]!=0) {x_max_y=i;hit_y=true;} else {if (!hit_y) x_min_y=i+1;} 
      if (accum_U[i]!=0) {x_max_u=i;hit_u=true;} else {if (!hit_u) x_min_u=i+1;} 
      if (accum_V[i]!=0) {x_max_v=i;hit_v=true;} else {if (!hit_v) x_min_v=i+1;} 

      if (!Ahit_x_miny) {Ax_min_y+=accum_Y[i]; if (Ax_min_y>At_y2){Ahit_x_miny=true; Ax_min_y=i;} }
      if (!Ahit_x_minu) {Ax_min_u+=accum_U[i]; if (Ax_min_u>At_uv2){Ahit_x_minu=true; Ax_min_u=i;} }
      if (!Ahit_x_minv) {Ax_min_v+=accum_V[i]; if (Ax_min_v>At_uv2){Ahit_x_minv=true; Ax_min_v=i;} }

      if (!Ahit_x_maxy) {Ax_max_y+=accum_Y[255-i]; if (Ax_max_y>At_y2){Ahit_x_maxy=true; Ax_max_y=255-i;} }
      if (!Ahit_x_maxu) {Ax_max_u+=accum_U[255-i]; if (Ax_max_u>At_uv2){Ahit_x_maxu=true; Ax_max_u=255-i;} }
      if (!Ahit_x_maxv) {Ax_max_v+=accum_V[255-i]; if (Ax_max_v>At_uv2){Ahit_x_maxv=true; Ax_max_v=255-i;} }
    }

    float Favg_y=avg_y/(float)pixels;
    float Favg_u=(avg_u*(float)uvdiv)/(float)pixels;
    float Favg_v=(avg_v*(float)uvdiv)/(float)pixels;
    if (param.analyze) {
#if 0
      char text[400];
      sprintf(text,
      "        Frame: %-8u ( Luma Y / ChromaU / ChromaV )\n"
      "      Average:      ( %7.2f / %7.2f / %7.2f )\n"
      "      x_minimum:      (   %3d   /   %3d   /   %3d    )\n"
      "      x_maximum:      (   %3d   /   %3d   /   %3d    )\n"
      "Loose x_minimum:      (   %3d   /   %3d   /   %3d    )\n"
      "Loose x_maximum:      (   %3d   /   %3d   /   %3d    )\n"
      ,
      frame,
      Favg_y,Favg_u,Favg_v,
      x_min_y,x_min_u,x_min_v,
      x_max_y,x_max_u,x_max_v,
      Ax_min_y,Ax_min_u,Ax_min_v,
      Ax_max_y,Ax_max_u,Ax_max_v
      );

      ApplyMessage(&src, vi, text, vi.width/4, 0xa0a0a0,0,0 , env );
      if (!(param.autowhite||param.autogain)) {
        return true; //x src;
      }
#endif
    }
    if (param.autowhite) {
      param.u_bright=127-(int)Favg_u;
      param.v_bright=127-(int)Favg_v;
    }
    if (param.autogain) {
      Ax_max_y=x_min(Ax_max_y,236);
      Ax_min_y=x_max(Ax_min_y,16);  // Never scale above luma range!
      if (Ax_min_y!=Ax_max_y) {
        int y_range = Ax_max_y-Ax_min_y;
        double scale = (220.0 / y_range);
        param.y_gain = (int) (256.0 * scale)-256;
        param.y_bright = -(int)(scale * (double)(Ax_min_y)-16);
      }
    }
	  MakeGammaLUT();
  }

#ifdef _DEBUG
	total = wby4 * h;
	totalby2 = total / 2;

	y.x_min  = u.x_min  = v.x_min  = 255;
	y.x_max  = u.x_max  = v.x_max  = 0;
	y.ave  = u.ave  = v.ave  = 0;
	y.over  = u.over  = v.over  = 0;
	y.under = u.under = v.under = 0;

	y0.x_min = u0.x_min = v0.x_min = 255;
	y0.x_max = u0.x_max = v0.x_max = 0;
	y0.ave = u0.ave = v0.ave = 0;
	y0.over  = u0.over  = v0.over  = 0;
	y0.under = u0.under = v0.under = 0;

	r.x_min   = g.x_min   = b.x_min   = 255;
	r.x_max   = g.x_max   = b.x_max   = 0;
	r.ave   = g.ave   = b.ave   = 0;
	r.over  = g.over  = b.over  = 0;
	r.under = g.under = b.under = 0;

	r0.x_min   = g0.x_min   = b0.x_min   = 255;
	r0.x_max   = g0.x_max   = b0.x_max   = 0;
	r0.ave   = g0.ave   = b0.ave   = 0;
	r0.over  = g0.over  = b0.over  = 0;
	r0.under = g0.under = b0.under = 0;
#endif
#if 0 //x
  if (vi.IsYUY2()) {
	  for (j = 0; j < h; j++)
	  {
		  for (i=0; i<wby4; i++)
		  {
			  pixel.data = *srcp;

#ifdef _DEBUG
			pixel0.data = pixel.data;
			CheckYUV(NULL, &pixel, &y0, &u0, &v0, 0);
			
			YUV2RGB(pixel.yuv.y0, pixel.yuv.u, pixel.yuv.v, &r0.d, &g0.d, &b0.d, matrix);
			CheckRGB(&r0, &g0, &b0);
			YUV2RGB(pixel.yuv.y1, pixel.yuv.u, pixel.yuv.v, &r0.d, &g0.d, &b0.d, matrix);
			CheckRGB(&r0, &g0, &b0);
#endif 

			  pixel.yuv.y0 = LUT_Y[pixel.yuv.y0];
			  pixel.yuv.u  = LUT_U[pixel.yuv.u ];
			  pixel.yuv.y1 = LUT_Y[pixel.yuv.y1];
			  pixel.yuv.v  = LUT_V[pixel.yuv.v ];
			  *srcp++ = pixel.data;

#ifdef _DEBUG
			CheckYUV(&pixel0, &pixel, &y, &u, &v, 1);

			YUV2RGB(pixel.yuv.y0, pixel.yuv.u, pixel.yuv.v, &r.d, &g.d, &b.d, matrix);
			CheckRGB(&r, &g, &b);
			YUV2RGB(pixel.yuv.y1, pixel.yuv.u, pixel.yuv.v, &r.d, &g.d, &b.d, matrix);
			CheckRGB(&r, &g, &b);
#endif
		  }
		  srcp = (unsigned long *)((unsigned char *)srcp + modulo) ;
	  }
  } else 
if (vi.IsPlanar()) 
#endif //x
    {
	  BYTE* srcp2 = (BYTE*) src->GetWritePtr(PLANAR_Y);
    for (j = 0; j < h; j++) {
		  for (i=0; i<w; i++) {
        srcp2[i]=LUT_Y[srcp2[i]];
      }
	    srcp2 +=  pitch;
    }
	  srcp2 = (BYTE*) src->GetWritePtr(PLANAR_U);
    h=src->GetHeight(PLANAR_U);
    w=src->GetRowSize(PLANAR_U);
    pitch=src->GetPitch(PLANAR_U);
    for (j = 0; j < h; j++) {
		  for (i=0; i<w; i++) {
        srcp2[i]=LUT_U[srcp2[i]];
      }
	    srcp2 +=  pitch;
    }
	  srcp2 = (BYTE*) src->GetWritePtr(PLANAR_V);
    for (j = 0; j < h; j++) {
		  for (i=0; i<w; i++) {
        srcp2[i]=LUT_V[srcp2[i]];
      }
	    srcp2 +=  pitch;
    }
  }

#ifdef _DEBUG
/*
	y.ave  = ( y.ave+total)/total/2; u.ave  = ( u.ave+totalby2)/total;   v.ave  = ( v.ave+totalby2)/total;
	y0.ave = (y0.ave+total)/total/2; u0.ave = (u0.ave+totalby2)/total;   v0.ave = (v0.ave+totalby2)/total;
	r.ave  = ( r.ave+total)/total/2; g.ave  = ( g.ave+total   )/total/2; b.ave  = ( b.ave+total   )/total/2;
	r0.ave = (r0.ave+total)/total/2; g0.ave = (g0.ave+total   )/total/2; b0.ave = (b0.ave+total   )/total/2;

  char buf[256];
  OutputDebugString("\n");
	sprintf(buf,"frame=%05d  x_minimun   Src=[Y:%03d  U:%03d  V:%03d  R:%03d  G:%03d  B:%03d]" \
				"  Dst=[Y:%03d  U:%03d  V:%03d  R:%03d  G:%03d  B:%03d]",
				frame, y0.x_min,u0.x_min,v0.x_min, r0.x_min,g0.x_min,b0.x_min,
				y.x_min,u.x_min,v.x_min, r.x_min,g.x_min,b.x_min);
	OutputDebugString(buf);

	sprintf(buf,"             x_maximum   Src=[Y:%03d  U:%03d  V:%03d  R:%03d  G:%03d  B:%03d]" \
				"  Dst=[Y:%03d  U:%03d  V:%03d  R:%03d  G:%03d  B:%03d]",
				y0.x_max,u0.x_max,v0.x_max, r0.x_max,g0.x_max,b0.x_max,
				y.x_max,u.x_max,v.x_max, r.x_max,g.x_max,b.x_max);
	OutputDebugString(buf);

	sprintf(buf,"             average   Src=[Y:%03d  U:%03d  V:%03d  R:%03d  G:%03d  B:%03d]" \
				"  Dst=[Y:%03d  U:%03d  V:%03d  R:%03d  G:%03d  B:%03d]",
				y0.ave,u0.ave,v0.ave, r0.ave,g0.ave,b0.ave,
				y.ave,u.ave,v.ave, r.ave,g.ave,b.ave);
	OutputDebugString(buf);

	sprintf(buf,"             underflow Src=[Y:%3d  U:%3d  V:%3d  R:%3d  G:%3d  B:%3d]" \
				"  Dst=[Y:%3d  U:%3d  V:%3d  R:%3d  G:%3d  B:%3d]",
				y0.under,u0.under,v0.under, r0.under,g0.under,b0.under,
				y.under,u.under,v.under, r.under,g.under,b.under
			);
	OutputDebugString(buf);

	sprintf(buf,"             overflow  Src=[Y:%3d  U:%3d  V:%3d  R:%3d  G:%3d  B:%3d]" \
				"  Dst=[Y:%3d  U:%3d  V:%3d  R:%3d  G:%3d  B:%3d]",
				y0.over,u0.over,v0.over, r0.over,g0.over,b0.over,
				y.over,u.over,v.over, r.over,g.over,b.over
			);
	OutputDebugString(buf);
*/
#endif

	return true;
}
/**
    \fn MakeGammaLUT
*/
void vidColorYuv::MakeGammaLUT(void)
{
	static const int scale = 256, shift = 2^10,
		coeff_y0 =  76309, coeff_y1 =  65536,
		coeff_u0 = 132201, coeff_u1 = 116129,
		coeff_v0 = 104597, coeff_v1 =  91881;
	int i;
	int val;
	double g,b,c,gain;
	double v;

	y_thresh1 = u_thresh1 = v_thresh1 = -1;
	y_thresh2 = u_thresh2 = v_thresh2 = 256;

	gain = ((double)param.y_gain + scale) / scale;
	c = ((double)param.y_contrast + scale) / scale;
	b = ((double)param.y_bright + scale) / scale;
	g = ((double)param.y_gamma + scale) / scale;
	if (g < 0.01)    g = 0.01;
	for (i = 0; i < 256; i++)
    {
		val = i * shift;
		switch (param.levels) {
			case 1:	// PC->TV
				val = (int)((val - 16 * shift) * coeff_y0 / coeff_y1 + shift / 2);
				break;
			case 2:	// TV->PC
			case 3:	// TV->PC.Y
				val = (int)(val * coeff_y1 / coeff_y0 + 16 * shift + shift / 2);
				break;
			default:	//none
				break;
		}
		val = val / shift;

		v = ((double)val) / 256;
		v = (v * gain) + ((v-0.5) * c + 0.5) - v + (b - 1);

		if (param.y_gamma != 0 && v > 0)
			v = pow( v, 1 / g);
		v = v * 256;
		
		v += 0.5;
		val = (int)floor(v);

		if (val > 255)
			val = 255;
		else if (val < 0)
			val = 0;

		if (val > 235) {
			if(y_thresh2 > 255)		y_thresh2 = i;
			if(param.opt)		val = 235;
		}
		else if (val < 16) {
			y_thresh1 = i;
			if(param.opt)		val = 16;
		}
		LUT_Y[i] = (unsigned char)val;
    }

	gain = ((double)param.u_gain + scale);
	c = ((double)param.u_contrast + scale);
	b = ((double)param.u_bright);
	for (i = 0; i < 256; i++)
    {
		val = i * shift;
		switch (param.levels) {
			case 1:	// PC->TV Scale
				val = (int)((val - 128 * shift) * coeff_u0 / coeff_u1 + 128 * shift + shift / 2);
				break;
			case 2:	// TV->PC Scale
				val = (int)((val - 128 * shift) * coeff_u1 / coeff_u0 + 128 * shift + shift / 2);
				break;
			default:	//none
				break;
		}
		val = val / shift;

		v = ((double)val);
		v = (v * gain / scale) + ((v-128) * c / scale + 128) - v + b;

		v += 0.5;
		val = (int)floor(v);
		
		if (val > 255)
			val = 255;
		else if (val < 0)
			val = 0;

		if (val > 240) {
			if(u_thresh2 > 255)		u_thresh2 = i;
			if(param.opt)		val = 240;
		}
		else if (val < 16) {
			u_thresh1 = i;
			if(param.opt)		val = 16;
		}
		LUT_U[i] = (unsigned char)val;
    }

	gain = ((double)param.v_gain + scale);
	c = ((double)param.v_contrast + scale);
	b = ((double)param.v_bright);
	for (i = 0; i < 256; i++)
    {
		val = i * shift;
		switch (param.levels) {
			case 1:	// PC->TV Scale
				val = (int)((val - 128 * shift) * coeff_v0 / coeff_v1 + 128 * shift + shift / 2);
				break;
			case 2:	// TV->PC Scale
				val = (int)((val - 128 * shift) * coeff_v1 / coeff_v0 + 128 * shift + shift / 2);
				break;
			default:	//none
				break;
		}
		val = val / shift;

		v = ((double)val);
		v = (v * gain / scale) + ((v-128) * c / scale + 128) - v + b;

		v += 0.5;
		val = (int)floor(v);
		
		if (val > 255)
			val = 255;
		else if (val < 0)
			val = 0;
		
		if (val > 240) {
			if(v_thresh2 > 255)		v_thresh2 = i;
			if(param.opt)		val = 240;
		}
		else if (val < 16) {
			v_thresh1 = i;
			if(param.opt)		val = 16;
		}
		LUT_V[i] = (unsigned char)val;
    }

#ifdef _DEBUG
	DumpLUT();
#endif

}
#if 0

void	Color::YUV2RGB(int y, int u, int v, int *r, int *g, int *b, int matrix)
{
  if (matrix==0) {
	  const int cy  = int((255.0/219.0)*65536+0.5);
	  const int crv = int(1.596*65536+0.5);
	  const int cgv = int(0.813*65536+0.5);
	  const int cgu = int(0.391*65536+0.5);
	  const int cbu = int(2.018*65536+0.5);
	  int scaled_y = (y - 16) * cy;
	  *b = ((scaled_y + (u-128) * cbu) + 2^15) >> 16;					// blue
	  *g = ((scaled_y - (u-128) * cgu - (v-128) * cgv) + 2^15) >> 16;	// green
	  *r = ((scaled_y + (v-128) * crv) + 2^15) >> 16;					// red
  } else {
	  // ITU-R Recommendation  BT.709
	  const int cy  = 19077;
	  const int crv = 3960 * 2;
	  const int cgv = -4363 * 2;
	  const int cgu = -1744 * 2;
	  const int cbu = 17307 * 2;
	  const int shift = 14;
	  int scaled_y = (y - 16) * cy;
	  *b = ((scaled_y + (u-128) * cbu) + 2^13) >> 14;					// blue
	  *g = ((scaled_y - (u-128) * cgu - (v-128) * cgv) + 2^13) >> 14;	// green
	  *r = ((scaled_y + (v-128) * crv) + 2^13) >> 14;					// red
  }
}


void	Color::CheckRGB(COUNT *r, COUNT *b, COUNT *g)
{
	if(r->d < 0)		{ r->under++;	/*r->d = 0;*/	}
	if(r->d > 255)		{ r->over++;	/*r->d = 255;*/	}
	if(g->d < 0)		{ g->under++;	/*g->d = 0;*/	}
	if(g->d > 255)		{ g->over++;	/*g->d = 255;*/	}
	if(b->d < 0)		{ b->under++;	/*b->d = 0;*/	}
	if(b->d > 255)		{ b->over++;	/*b->d = 255;*/	}
	if(r->d < r->x_min)	{ r->x_min = r->d;	}
	if(g->d < g->x_min)	{ g->x_min = g->d;	}
	if(b->d < b->x_min)	{ b->x_min = b->d;	}
	if(r->d > r->x_max)	{ r->x_max = r->d;	}
	if(g->d > g->x_max)	{ g->x_max = g->d;	}
	if(b->d > b->x_max)	{ b->x_max = b->d;	}
	r->ave += r->d;
	g->ave += g->d;
	b->ave += b->d;
}	

void	Color::CheckYUV(PIXELDATA *pixel0, PIXELDATA *pixel, COUNT *y, COUNT *u, COUNT *v, int terget )
{
	if(!terget)	{
		if(pixel->yuv.y0 < 16)			{ y->under++;	}
		if(pixel->yuv.y0 > 235)			{ y->over++;	}
		if(pixel->yuv.y1 < 16)			{ y->under++;	}
		if(pixel->yuv.y1 > 235)			{ y->over++;	}
	} else {
		if(pixel0->yuv.y0 <= y_thresh1)	{ y->under++;	}
		if(pixel0->yuv.y0 >= y_thresh2)	{ y->over++;	}
		if(pixel0->yuv.y1 <= y_thresh1)	{ y->under++;	}
		if(pixel0->yuv.y1 >= y_thresh2)	{ y->over++;	}
	}
	if(pixel->yuv.y0 < y->x_min)			{ y->x_min = pixel->yuv.y0;}
	if(pixel->yuv.y0 > y->x_max)			{ y->x_max = pixel->yuv.y0;}
	if(pixel->yuv.y1 < y->x_min)			{ y->x_min = pixel->yuv.y1;}
	if(pixel->yuv.y1 > y->x_max)			{ y->x_max = pixel->yuv.y1;}
	y->ave += pixel->yuv.y0;
	y->ave += pixel->yuv.y1;

	if(!terget)	{
		if(pixel->yuv.u < 16)			{ u->under++;	}
		if(pixel->yuv.u > 240)			{ u->over++;	}
	} else {
		if(pixel0->yuv.u <= u_thresh1)	{ u->under++;	}
		if(pixel0->yuv.u >= u_thresh2)	{ u->over++;	}
	}
	if(pixel->yuv.u < u->x_min)			{ u->x_min = pixel->yuv.u;}
	if(pixel->yuv.u > u->x_max)			{ u->x_max = pixel->yuv.u;}
	u->ave += pixel->yuv.u;

	if(!terget)	{
		if(pixel->yuv.v < 16)			{ v->under++;	}
		if(pixel->yuv.v > 240)			{ v->over++;	}
	} else {
		if(pixel0->yuv.v <= v_thresh1)	{ v->under++;	}
		if(pixel0->yuv.v >= v_thresh2)	{ v->over++;	}
	}
	if(pixel->yuv.v < v->x_min)			{ v->x_min = pixel->yuv.v;}
	if(pixel->yuv.v > v->x_max)			{ v->x_max = pixel->yuv.v;}
	v->ave += pixel->yuv.v;
}

bool Color::CheckParms(const char *_levels, const char *_matrix, const char *_opt)
{
	int i;
	static const char	*LevelsTbl[] = { "", "TV->PC", "PC->TV", "PC->TV.Y" },
		*MatrixTbl[] = { "", "rec.709" },
		*OptTbl[] = { "", "coring" };

	levels = -1;
	if (_levels) {
		for (i=0; i<4 ; i++) {
			if (!lstrcmpi(_levels, LevelsTbl[i])) 
			{
				levels = i;
				break;
			}
		}
	} else {		
		levels = 0;
	}

	matrix = -1;
	if (_matrix) {
		for (i=0; i<2 ; i++) {
			if (!lstrcmpi(_matrix, MatrixTbl[i])) 
			{
				matrix = i;
				break;
			}
		}
	} else {		
		matrix = 0;
	}

	opt = -1;
	if (_opt) {
		for (i=0; i<2 ; i++) {
			if (!lstrcmpi(_opt, OptTbl[i])) 
			{
				opt = i;
				break;
			}
		}
	} else {		
		opt = 0;
	}

	if ( levels < 0 || matrix < 0 || opt < 0 )	return FALSE;
	return TRUE;
}


void Color::DumpLUT(void)
{
/*
  static const char *TitleTbl[] = {
		"Color Adjust Look-up Table : Y, lux_minance\n",
		"Color Adjust Look-up Table : U, Cb, Color Difference(blue)\n",
		"Color Adjust Look-up Table : V, Cr, Color Difference(red)\n"
	};
	static const BYTE *LUT[] = { (BYTE *)&LUT_Y, (BYTE *)&LUT_U, (BYTE *)&LUT_V };
	int	index,i,j;
	char buf[512];

	for(index=0; index<3; index++)
	{
		OutputDebugString( TitleTbl[index] );
		for(i=0; i<16;i++)
		{
			sprintf(buf,"%03d(%02X) : ", i * 16, i * 16);
			for(j=0;j<8;j++)
			{
				sprintf(&buf[j*4+10], "%03d ",LUT[index][i*16+j]);
			}
			sprintf(&buf[8*4+10], " - ");
			for(j=8;j<16;j++)
			{
				sprintf(&buf[j*4+13], "%03d ",LUT[index][i*16+j]);
			}
			OutputDebugString(buf);
		}
		OutputDebugString("\n");
	}
*/
}
AVSValue __cdecl Color::Create(AVSValue args, void* user_data, IScriptEnvironment* env) {
    try {	// HIDE DAMN SEH COMPILER BUG!!!
		return new Color(args[0].AsClip(),
						args[1].AsFloat(0.0),		// gain_y
						args[2].AsFloat(0.0),		// off_y      bright
						args[3].AsFloat(0.0),		// gamma_y
						args[4].AsFloat(0.0),		// cont_y
						args[5].AsFloat(0.0),		// gain_u
						args[6].AsFloat(0.0),		// off_u      bright
						args[7].AsFloat(0.0),		// gamma_u
						args[8].AsFloat(0.0),		// cont_u
						args[9].AsFloat(0.0),		// gain_v
						args[10].AsFloat(0.0),		// off_v
						args[11].AsFloat(0.0),		// gamma_v
						args[12].AsFloat(0.0),		// cont_v
						args[13].AsString(""),		// levels = "", "TV->PC", "PC->TV"
						args[14].AsString(""),		// opt = "", "coring"
						args[15].AsString(""),		// matrix = "", "rec.709"
						args[16].AsBool(false),		// colorbars
						args[17].AsBool(false),		// analyze
						args[18].AsBool(false),		// autowhite
						args[19].AsBool(false),		// autogain
						env);
	}
	catch (...) { throw; }
}
#endif
// EOF
