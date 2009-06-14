#ifndef AUDM_LAME_PARAM_H
#define AUDM_LAME_PARAM_H

typedef enum 
{
  ADM_LAME_PRESET_CBR,
  ADM_LAME_PRESET_ABR,
  ADM_LAME_PRESET_EXTREME
}ADM_LAME_PRESET;

typedef struct 
{
  ADM_LAME_PRESET preset;
  const char	*name;
}ADM_PRESET_DEFINITION;
static const ADM_PRESET_DEFINITION      presetDefinition[]=
{
  {ADM_LAME_PRESET_CBR,"CBR"},
  {ADM_LAME_PRESET_ABR,"ABR"},
  {ADM_LAME_PRESET_EXTREME,"Extreme"}
};    
/**

*/
typedef struct 
{
  uint32_t        bitrate; // in kbps
  ADM_LAME_PRESET preset;
  uint32_t        quality;
  uint32_t        disableReservoir; // usefull for strict CBR (FLV)
}LAME_encoderParam;

#endif
