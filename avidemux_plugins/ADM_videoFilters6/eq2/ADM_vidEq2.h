
#ifndef EQ2_PARAM
#define EQ2_PARAM
#define LUT16

#include "eq2.h"

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
uint8_t DIA_getEQ2Param(eq2 *param,ADM_coreVideoFilter *in);

void update_lut(Eq2Settings *settings,eq2 *_param);
void apply_lut (oneSetting *par, ADMImage *srcImage, ADMImage *destImage,ADM_PLANE plane);
void create_lut (oneSetting *par);

#ifdef ADM_CPU_X86
void affine_1d_MMX (oneSetting *par, ADMImage *srcImage, ADMImage *destImage,ADM_PLANE plane);
#endif


#endif
