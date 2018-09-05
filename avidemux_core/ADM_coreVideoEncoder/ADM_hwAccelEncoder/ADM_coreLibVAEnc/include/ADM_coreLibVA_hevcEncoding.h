#pragma once

class ADM_VA_GlobalHEVC
{
public:
    ADM_VA_GlobalHEVC() { profile = VAProfileNone; }
    VAProfile profile;
};

const ADM_VA_GlobalHEVC *vaGetHevcEncoderProfile();
