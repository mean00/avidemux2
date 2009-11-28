/**
    \file ADM_coreUtils.h
    \brief Declaration of coreUtils

*/
#ifndef ADM_CORE_UTILS_H
#define ADM_CORE_UTILS_H

bool        ADM_findMpegStartCode(uint8_t *start, uint8_t *end,uint8_t *outstartcode,uint32_t *offset);
char        *ADM_escape(const ADM_filename *incoming);
uint32_t    ADM_computeBitrate(uint32_t fps1000, uint32_t nbFrame, uint32_t sizeInMB);
uint32_t    ADM_UsecFromFps1000(uint32_t fps1000);
uint32_t    ADM_Fps1000FromUs(uint64_t us);



#endif