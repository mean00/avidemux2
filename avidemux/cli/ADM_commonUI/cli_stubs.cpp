#include "ADM_default.h"
//
bool DIA_job_select(char **jobname, char **filename)
{
    return false;
}
//
int DIA_getHDRParams(uint32_t *toneMappingMethod, float *saturationAdjust, float *boostAdjust, bool *adaptiveRGB,
                     uint32_t *gamutMethod)
{
    return 0;
}
//
int DIA_getMPParams(uint32_t *pplevel, uint32_t *ppstrength, bool *swap)
{
    return 0;
}
//
class ADM_AUDIOFILTER_CONFIG;
int DIA_getAudioFilter(ADM_AUDIOFILTER_CONFIG *config, double tempoHint)
{
    return 0;
}
//
uint8_t DIA_pluginsInfo(void)
{
    return 0;
}
//
uint8_t DIA_builtin(void)
{
    return 0;
}
//
uint8_t DIA_gotoTime(uint32_t *hh, uint32_t *mm, uint32_t *ss, uint32_t *ms)
{
    return 0;
}
//
uint8_t DIA_Preferences(void)
{
    return 0;
}
// EOF
