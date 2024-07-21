#pragma once

class ADM_VA_GlobalAV1
{
public:
    ADM_VA_GlobalAV1() { profile = VAProfileNone; }
    VAProfile profile;
};

const ADM_VA_GlobalAV1 *vaGetAV1EncoderProfile();
