/***************************************************************************
                      NAL TYPE for H264
                      
**************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once

#define LIST_OF_NAL_TYPE\
    NAME(NAL_H265_TRAIL_N,      0)    , \
    NAME(NAL_H265_TRAIL_R,      1)    , \
    NAME(NAL_H265_TSA_N      , 2),\
    NAME(NAL_H265_TSA_R      , 3),\
    NAME(NAL_H265_STSA_N     , 4),\
    NAME(NAL_H265_STSA_R     , 5),\
    NAME(NAL_H265_RADL_N     , 6),\
    NAME(NAL_H265_RADL_R     , 7),\
    NAME(NAL_H265_RASL_N     , 8),\
    NAME(NAL_H265_RASL_R     , 9),\
    NAME(NAL_H265_BLA_W_LP   , 16),\
    NAME(NAL_H265_BLA_W_RADL , 17),\
    NAME(NAL_H265_BLA_N_LP   , 18),\
    NAME(NAL_H265_IDR_W_RADL , 19),\
    NAME(NAL_H265_IDR_N_LP,     20) , \
    NAME(NAL_H265_CRA_NUT    ,  21),\
    NAME(NAL_H265_IRAP_VCL23 ,  23),\
    NAME(NAL_H265_VPS  ,        32)    ,\
    NAME(NAL_H265_SPS  ,        33)    ,\
    NAME(NAL_H265_PPS  ,        34)    ,\
    NAME(NAL_H265_AUD  ,        35)    ,\
    NAME(NAL_H265_FD_NUT  ,        38)    ,\
    NAME(NAL_H265_SEI_PREFIX,   39),\
    NAME(NAL_H265_SEI_SUFFIX,   40),\


#define NAME(x,y) x= y

enum{
LIST_OF_NAL_TYPE
};
#undef NAME
#define NAME(x,y) {y,#x}

typedef struct NAL_DESC{int value; const char *name;}NAL_DESC;

const NAL_DESC nalDesc[]={
    LIST_OF_NAL_TYPE
};
    

//EOF
