
/***************************************************************************
    Port of ColorYuv from avisynth to avidemux by mean
 ***************************************************************************/
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

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidColorYuv.h"


extern int DIA_coloryuv(COLOR_YUV_PARAM *param);

static FILTER_PARAM coloryuv_template={19,
    {"y_contrast","y_bright","y_gamma","y_gain",
    "u_contrast","u_bright","u_gamma","u_gain",
    "v_contrast","v_bright","v_gamma","v_gain",
    "matrix","levels","opt",
    "colorbars","analyze","autowhite","autogain",
    }};
//REGISTERX(VF_COLORS, "coloryuv",QT_TR_NOOP("Avisynth ColorYUV"),QT_TR_NOOP
//    ("Alter colors (auto white balance etc...). Ported from Avisynth."),VF_COLOR_YUV,1,coloryuv_create,coloryuv_script);

//******************************************
VF_DEFINE_FILTER_UI(ADMVideoColorYuv,coloryuv_template,
                coloryuv,
                QT_TR_NOOP("Avisynth ColorYUV"),
                1,
                VF_COLORS,
                QT_TR_NOOP("Alter colors (auto white balance etc...). Ported from Avisynth."));

//******************************************
uint8_t ADMVideoColorYuv::configure(AVDMGenericVideoStream *in)
{

   _in=in;
   if(DIA_coloryuv(_param))
   {
       MakeGammaLUT();
       return 1;
   }
   return 0;
}

char *ADMVideoColorYuv::printConf( void )
{
  ADM_FILTER_DECLARE_CONF(" ColorYuv ");
   
}

ADMVideoColorYuv::ADMVideoColorYuv(AVDMGenericVideoStream *in,CONFcouple *couples) 
{
   _in=in;
   memcpy(&_info,_in->getInfo(),sizeof(_info));    
   _info.encoding=1;
   _uncompressed=NULL;
   _param=NEW(COLOR_YUV_PARAM);
   if(couples)
   {
       double y_contrast, y_bright, y_gamma, y_gain;
       double u_contrast, u_bright, u_gamma, u_gain;
       double v_contrast, v_bright, v_gamma, v_gain;
       int32_t matrix, levels, opt;
       uint32_t colorbars, analyze, autowhite, autogain;
       GET(y_contrast);
       GET(y_bright);
       GET(y_gamma);
       GET(y_gain); 
       
       GET(u_contrast);
       GET(u_bright);
       GET(u_gamma);
       GET(u_gain); 
       
       GET(v_contrast);
       GET(v_bright);
       GET(v_gamma);
       GET(v_gain); 
       
       GET(matrix);
       GET(levels);
       GET(opt); 
       
       GET(colorbars);
       GET(analyze);
       GET(autowhite); 
       GET(autogain); 
       
       
   }
   else // Default
   {
#define MKP(x,y) _param->x=y;
            MKP(y_contrast,0);
            MKP(y_bright,0);
            MKP(y_gamma,0);
            MKP(y_gain,0);

            MKP(u_contrast,0);
            MKP(u_bright,0);
            MKP(u_gamma,0);
            MKP(u_gain,0);

            MKP(v_contrast,0);
            MKP(v_bright,0);
            MKP(v_gamma,0);
            MKP(v_gain,0);
            
            MKP(matrix,0);
            MKP(levels,0);
            MKP(opt,0); 
       
            MKP(colorbars,0);
            MKP(analyze,1);
            MKP(autowhite,1); 
            MKP(autogain,0); 

            
            
   }
   MakeGammaLUT();
}
//____________________________________________________________________
ADMVideoColorYuv::~ADMVideoColorYuv()
{
   delete _param;
   _param=NULL;
   _uncompressed=NULL;
}
#define BYTE uint8_t 
#ifndef MAX
#define MAX(x,y) ((x)>(y) ?(x):(y))
#endif
#ifndef MIN
#define MIN(x,y) ((x)<(y) ?(x):(y))
#endif
//______________________________________________________________
uint8_t ADMVideoColorYuv::getFrameNumberNoAlloc(uint32_t frame,
  uint32_t *len,
  ADMImage *data,
  uint32_t *flags)
  {
      uint32_t page=_info.width*_info.height,pitch;
      if(frame>_info.nb_frames-1) return 0;
      
      if(!_in->getFrameNumberNoAlloc(frame, len,data,flags)) 
      {
          printf("ColorYuv : Cannot read cache for frame %u\n",frame);
          return 0;
      }
      
      ADMImage *mysrc=NULL;
      mysrc=data;
      
      //*************************************************
      int i,j,wby4;
      int modulo,w,h;
//	int dmodulo;
      //PIXELDATA	pixel;

      pitch = mysrc->GetPitch(PLANAR_Y);
      w = mysrc->GetRowSize(PLANAR_Y);
      h = mysrc->GetHeight(PLANAR_Y);
  wby4 = w / 4;
  modulo = pitch - w;
//	dst = env->NewVideoFrame(vi);
//	dstp = (unsigned long *) dst->GetWritePtr();
//	dpitch = dst->GetPitch();
//	dmodulo = dpitch - dst->GetRowSize();
  if (_param->analyze||_param->autowhite||_param->autogain) {
      for (i=0;i<256;i++) {
          accum_Y[i]=0;
          accum_U[i]=0;
          accum_V[i]=0;
      }
      int uvdiv=1;  //UV divider (ratio between Y and UV pixels)
     {
          uvdiv=4;
          BYTE* srcp2 = (BYTE*) mysrc->GetReadPtr(PLANAR_Y);
          for (int y=0;y<h;y++) {
              for (int x=0;x<w;x++) {
                  accum_Y[srcp2[x]]++;
              }
              srcp2+=pitch;
          }
          pitch = mysrc->GetPitch(PLANAR_U);
          srcp2 = (BYTE*) mysrc->GetReadPtr(PLANAR_U);
          for (int y=0;y<h/2;y++) {
              for (int x=0;x<w/2;x++) {
                  accum_U[srcp2[x]]++;
              }
              srcp2+=pitch;
          }
          srcp2 = (BYTE*) mysrc->GetReadPtr(PLANAR_V);
          for (int y=0;y<h/2;y++) {
              for (int x=0;x<w/2;x++) {
                  accum_V[srcp2[x]]++;
              }
              srcp2+=pitch;
          }
          pitch = mysrc->GetPitch(PLANAR_Y);
      } 
      int pixels = _info.width*_info.height;
      float avg_u=0, avg_v=0, avg_y=0;
      int min_u=0, min_v=0, min_y=0;
      int max_u=0, max_v=0, max_y=0;
      bool hit_y=false,hit_u=false,hit_v=false;
      int Amin_u=0, Amin_v=0, Amin_y=0;
      int Amax_u=0, Amax_v=0, Amax_y=0;
      bool Ahit_miny=false,Ahit_minu=false,Ahit_minv=false;
      bool Ahit_maxy=false,Ahit_maxu=false,Ahit_maxv=false;
      int At_y2=(pixels/256); // When 1/256th of all pixels have been reached, trigger "Loose min/max"
      int At_uv2=(pixels/1024); 
   
      for (i=0;i<256;i++) {
          avg_y+=(float)accum_Y[i]*(float)i;
          avg_u+=(float)accum_U[i]*(float)i;
          avg_v+=(float)accum_V[i]*(float)i;
          if (accum_Y[i]!=0) {max_y=i;hit_y=true;} else {if (!hit_y) min_y=i+1;} 
          if (accum_U[i]!=0) {max_u=i;hit_u=true;} else {if (!hit_u) min_u=i+1;} 
          if (accum_V[i]!=0) {max_v=i;hit_v=true;} else {if (!hit_v) min_v=i+1;} 

          if (!Ahit_miny) {Amin_y+=accum_Y[i]; if (Amin_y>At_y2){Ahit_miny=true; Amin_y=i;} }
          if (!Ahit_minu) {Amin_u+=accum_U[i]; if (Amin_u>At_uv2){Ahit_minu=true; Amin_u=i;} }
          if (!Ahit_minv) {Amin_v+=accum_V[i]; if (Amin_v>At_uv2){Ahit_minv=true; Amin_v=i;} }

          if (!Ahit_maxy) {Amax_y+=accum_Y[255-i]; if (Amax_y>At_y2){Ahit_maxy=true; Amax_y=255-i;} }
          if (!Ahit_maxu) {Amax_u+=accum_U[255-i]; if (Amax_u>At_uv2){Ahit_maxu=true; Amax_u=255-i;} }
          if (!Ahit_maxv) {Amax_v+=accum_V[255-i]; if (Amax_v>At_uv2){Ahit_maxv=true; Amax_v=255-i;} }
      }

      float Favg_y=avg_y/(float)pixels;
      float Favg_u=(avg_u*(float)uvdiv)/(float)pixels;
      float Favg_v=(avg_v*(float)uvdiv)/(float)pixels;
      if (_param->analyze) {
          char text[400];
          sprintf(text,   "Frame  : %-8u ( Y /ChrU/ChrV )\n",frame);
          drawString(mysrc, 0, 5, text);
          
          sprintf(text,   "Avr:( %3.2f / %3.2f / %3.2f )",Favg_y,Favg_u,Favg_v);
          drawString(mysrc, 0, 6, text);
          
          sprintf(text,   "Minimum:  ( %3d / %3d / %3d )\n",
                  min_y,min_u,min_v);
          drawString(mysrc, 0, 7, text);                          
          sprintf(text,   "Maximum:  ( %3d / %3d / %3d )\n",
                  max_y,max_u,max_v);
          drawString(mysrc, 0, 8, text);
          
          sprintf(text,   "LooseMin  ( %3d / %3d / %3d )\n",
                  Amin_y,Amin_u,Amin_v);
          drawString(mysrc, 0, 9, text);
          
          sprintf(text,   "Loose Max:( %3d / %3d / %3d )\n",
                  Amax_y,Amax_u,Amax_v);
          drawString(mysrc, 0, 10, text);
          if (!(_param->autowhite||_param->autogain)) {
              return 1;
          }
      }
      if (_param->autowhite) {
          _param->u_bright=127-(int)Favg_u;
          _param->v_bright=127-(int)Favg_v;
      }
      if (_param->autogain) {
          Amax_y=MIN(Amax_y,236);
          Amin_y=MAX(Amin_y,16);  // Never scale above luma range!
          if (Amin_y!=Amax_y) {
              int y_range = Amax_y-Amin_y;
              double scale = (220.0 / y_range);
              _param->y_gain = (int) (256.0 * scale)-256;
              _param->y_bright = -(int)(scale * (double)(Amin_y)-16);
          }
      }
      MakeGammaLUT();
  }

  {
      BYTE* srcp2 = (BYTE*) mysrc->data;
      for (j = 0; j < h; j++) {
          for (i=0; i<w; i++) {
              srcp2[i]=LUT_Y[srcp2[i]];
          }
          srcp2 +=  pitch;
      }
      srcp2 = (BYTE*) mysrc->GetWritePtr(PLANAR_U);
      h=mysrc->GetHeight(PLANAR_U);
      w=mysrc->GetRowSize(PLANAR_U);
      pitch=mysrc->GetPitch(PLANAR_U);
      for (j = 0; j < h; j++) {
          for (i=0; i<w; i++) {
              srcp2[i]=LUT_U[srcp2[i]];
          }
          srcp2 +=  pitch;
      }
      srcp2 = (BYTE*) mysrc->GetWritePtr(PLANAR_V);
      for (j = 0; j < h; j++) {
          for (i=0; i<w; i++) {
              srcp2[i]=LUT_V[srcp2[i]];
          }
          srcp2 +=  pitch;
      }
  }


	return 1;

}


uint8_t	ADMVideoColorYuv::getCoupledConf( CONFcouple **couples)
{
   
      ADM_assert(_param);
      *couples=new CONFcouple(19);
#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
       CSET(y_contrast);
       CSET(y_bright);
       CSET(y_gamma);
       CSET(y_gain); 
       
       CSET(u_contrast);
       CSET(u_bright);
       CSET(u_gamma);
       CSET(u_gain); 
       
       CSET(v_contrast);
       CSET(v_bright);
       CSET(v_gamma);
       CSET(v_gain); 
       
       CSET(matrix);
       CSET(levels);
       CSET(opt); 
       
       CSET(colorbars);
       CSET(analyze);
       CSET(autowhite); 
       CSET(autogain); 
   
      return 1;
}


void ADMVideoColorYuv::MakeGammaLUT(void)
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

    gain = ((double)_param->y_gain + scale) / scale;
    c = ((double)_param->y_contrast + scale) / scale;
    b = ((double)_param->y_bright + scale) / scale;
    g = ((double)_param->y_gamma + scale) / scale;
    if (g < 0.01)    g = 0.01;
    for (i = 0; i < 256; i++)
    {
        val = i * shift;
        switch (_param->levels) {
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

        if (_param->y_gamma != 0 && v > 0)
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
            if(_param->opt)		val = 235;
        }
        else if (val < 16) {
            y_thresh1 = i;
            if(_param->opt)		val = 16;
        }
        LUT_Y[i] = (unsigned char)val;
    }

    gain = ((double)_param->u_gain + scale);
    c = ((double)_param->u_contrast + scale);
    b = ((double)_param->u_bright);
    for (i = 0; i < 256; i++)
    {
        val = i * shift;
        switch (_param->levels) {
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
            if(_param->opt)		val = 240;
        }
        else if (val < 16) {
            u_thresh1 = i;
            if(_param->opt)		val = 16;
        }
        LUT_U[i] = (unsigned char)val;
    }

    gain = ((double)_param->v_gain + scale);
    c = ((double)_param->v_contrast + scale);
    b = ((double)_param->v_bright);
    for (i = 0; i < 256; i++)
    {
        val = i * shift;
        switch (_param->levels) {
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
            if(_param->opt)		val = 240;
        }
        else if (val < 16) {
            v_thresh1 = i;
            if(_param->opt)		val = 16;
        }
        LUT_V[i] = (unsigned char)val;
    }

#ifdef _DEBUG
	DumpLUT();
#endif

}



// EOF
