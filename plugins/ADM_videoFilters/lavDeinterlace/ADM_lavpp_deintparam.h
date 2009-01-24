//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
enum 
{
  PP_BM_NONE           =0x0000,
  PP_BM_LINEAR_BLEND   =0x0001, 
  PP_BM_LINEAR_INTER   =0x0002, 
  PP_BM_CUBIC_INTER    =0x0003, 
  PP_BM_MEDIAN_INTER   =0x0004, 
  PP_BM_FFMPEG_DEINT   =0x0005,  
};

typedef struct 
{
  uint32_t      deintType;
  uint32_t       autolevel;
}lavc_pp_param;
