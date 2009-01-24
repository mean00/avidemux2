
#ifndef _ASHARP_PARAM
#define _ASHARP_PARAM
#define uc uint8_t
#define bool uint8_t

 void asharp_run_c(      uc* planeptr, int pitch,
                                        int height, int width, 
                                        int     T,int D, int B, int B2, bool bf );

typedef struct ASHARP_PARAM
{
        double          t; // Threshold 0:nothing max : 5 ?
        double          d; // Strength : 0 means nothing, >0  adaptative t is maximum
        double          b; // Block adaptative sharpening -1 disabled
        uint32_t        bf;
}ASHARP_PARAM;
#endif
