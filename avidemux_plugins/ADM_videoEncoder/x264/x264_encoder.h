// Automatically generated, do not edit!
#ifndef ADM_x264_encoder_CONF_H
#define ADM_x264_encoder_CONF_H
typedef struct {
   COMPRES_PARAMS params;
   uint32_t MaxRefFrames;
   uint32_t MinIdr;
   uint32_t MaxIdr;
   uint32_t threads;
   bool _8x8;
   bool _8x8P;
   bool _8x8B;
   bool _4x4;
   bool _8x8I;
   bool _4x4I;
   uint32_t MaxBFrame;
   uint32_t profile;
   bool CABAC;
   bool Trellis;
}x264_encoder;
#endif //x264_encoder
//EOF
