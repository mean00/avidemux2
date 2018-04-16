

#pragma once

class ADM_VA_GlobalH264
{
public:
  ADM_VA_GlobalH264()
  {
    profile        = VAProfileNone ; /// High = (1 << 3), main = (1 << 1)
    h264_maxref_p0 = 1;
    h264_maxref_p1 = 1;
    packedHeaderCapabilities=0;
  }
  vaSetAttributes newAttributes;  //
  int             packedHeaderCapabilities;
  VAProfile       profile;
  int             h264_maxref_p0;
  int             h264_maxref_p1;
};


const ADM_VA_GlobalH264 *vaGetH264EncoderProfile();