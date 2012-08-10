/**
    \file ADM_coreUtils.h
    \brief Declaration of coreUtils

*/
#ifndef ADM_CORE_UTILS_H
#define ADM_CORE_UTILS_H

#include "ADM_coreUtils6_export.h"

#ifdef _WIN32
#	include <windows.h>
#endif

#include "ADM_inttype.h"
#include "avifmt.h"
#include "avifmt2.h"
#include "ADM_image.h"

ADM_COREUTILS6_EXPORT bool        ADM_findMpegStartCode(uint8_t *start, uint8_t *end,uint8_t *outstartcode,uint32_t *offset);
ADM_COREUTILS6_EXPORT bool        ADM_findH264StartCode(uint8_t *start, uint8_t *end,uint8_t *outstartcode,uint32_t *offset);
char        *ADM_escape(const ADM_filename *incoming);
uint32_t    ADM_computeBitrate(uint32_t fps1000, uint32_t nbFrame, uint32_t sizeInMB);
ADM_COREUTILS6_EXPORT uint32_t    ADM_UsecFromFps1000(uint32_t fps1000);
ADM_COREUTILS6_EXPORT uint32_t    ADM_Fps1000FromUs(uint64_t us);
ADM_COREUTILS6_EXPORT bool        ADM_splitSequencedFile(const char *filename, char **left, char **right, uint32_t *nbDigit, uint32_t *base);
ADM_COREUTILS6_EXPORT bool        ADM_computeAverageBitrateFromDuration(uint64_t duration, uint32_t sizeInMB, uint32_t *avgInKbits);
ADM_COREUTILS6_EXPORT ADM_ASPECT  getAspectRatioFromAR(uint32_t width, uint32_t height,const char **s);
ADM_COREUTILS6_EXPORT int32_t     ADM_getNiceValue(uint32_t priorityLevel);
void        Endian_AviMainHeader(MainAVIHeader *m);
void        Endian_BitMapInfo( ADM_BITMAPINFOHEADER *b);
void        Endian_AviStreamHeader(AVIStreamHeader *s);
ADM_COREUTILS6_EXPORT void        Endian_WavHeader(WAVHeader *w);
ADM_COREUTILS6_EXPORT void        printBih(ADM_BITMAPINFOHEADER *bi);
ADM_COREUTILS6_EXPORT bool        ADM_probeSequencedFile(const char *fileName);
ADM_COREUTILS6_EXPORT uint8_t     mk_hex(uint8_t a, uint8_t b);
ADM_COREUTILS6_EXPORT void mixDump(uint8_t * ptr, uint32_t len);

#endif
