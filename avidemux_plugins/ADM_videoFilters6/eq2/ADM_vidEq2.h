#pragma once
#define LUT16

#include "eq2.h"

#if defined( ADM_CPU_X86) && !defined(_MSC_VER)
#   define CAN_DO_INLINE_X86_ASM
#endif

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

/**
    \class ADMVideoEq2
*/
class ADMVideoEq2:public ADM_coreVideoFilter
{
  protected:
            eq2         _param;
            Eq2Settings settings;
            ADMImage    *mysrc;

            void        update(void);
  public:
                        ADMVideoEq2(ADM_coreVideoFilter *in,CONFcouple *couples);
                        ~ADMVideoEq2();
    virtual const char  *getConfiguration(void); /// Return current configuration as a human readable string
    virtual bool        getNextFrame(uint32_t *fn,ADMImage *image); /// Return the next image
    virtual bool        getCoupledConf(CONFcouple **couples); /// Return the current filter configuration
    virtual void        setCoupledConf(CONFcouple *couples);
    virtual bool        configure(void); /// Start graphical user interface

            static bool processPlane(oneSetting *par, ADMImage *srcImage, ADMImage *destImage, ADM_PLANE plane);
            static bool update_lut(Eq2Settings *settings, eq2 *cfg);

};
