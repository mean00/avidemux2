#ifndef adm_encConfig_h
#define adm_encConfig_h

#include "ADM_vidEncode.hxx"

void saveEncoderConfig(void);
void videoCodecSetCodec(int codecIndex);
int videoCodecConfigure(char *cmdString, uint32_t optionSize, uint8_t * option);

const char *videoCodecGetName(void);
int videoCodecSelectByName (const char *name);
int videoCodecPluginSelectByGuid(const char *guid);

SelectCodecType videoCodecGetType(void);
const char* videoCodecPluginGetGuid(void);
void videoCodecSetConf(uint32_t extraLen, uint8_t * extraData);
uint8_t videoCodecGetConf(uint32_t * nbData, uint8_t ** data);

// Returns the mode (CQ/CBR...) as a string, suitable for a script
const char *videoCodecGetMode(void);
uint8_t videoCodecSetFinalSize(uint32_t size);

void setIpod_Xvid4Preset(void);
#endif	// adm_encConfig_h
