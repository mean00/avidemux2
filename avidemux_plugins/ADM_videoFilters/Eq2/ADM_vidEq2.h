
#ifndef EQ2_PARAM
#define EQ2_PARAM
#define LUT16
typedef struct Eq2_Param
{
  float     contrast ;      /* 1.0 means do nothing..*/    
  float     brightness;     /* 0 means */
  float     saturation;
  
  float     gamma;
  float     gamma_weight;
  float     rgamma;
  float     ggamma;
  float     bgamma;
}Eq2_Param;
//*************************


typedef struct oneSetting {
  unsigned char lut[256];
#ifdef LUT16
  uint16_t lut16[256*256];
#endif
  int           lut_clean;

  double        c;
  double        b;
  double        g;
  double        w;
} oneSetting;
typedef struct Eq2Settings {
  oneSetting param[3];

  double        contrast;
  double        brightness;
  double        saturation;

  double        gamma;
  double        gamma_weight;
  double        rgamma;
  double        ggamma;
  double        bgamma;

 
} Eq2Settings;
//*************************
uint8_t DIA_getEQ2Param(Eq2_Param *param,AVDMGenericVideoStream *in);

void update_lut(Eq2Settings *settings,Eq2_Param *_param);
void apply_lut (oneSetting *par, unsigned char *dst, unsigned char *src,
  unsigned int w, unsigned int h);
void create_lut (oneSetting *par);

#ifdef ADM_CPU_X86
void affine_1d_MMX (oneSetting *par, unsigned char *dst, unsigned char *src,
                unsigned int w, unsigned int h);
#endif


#endif
