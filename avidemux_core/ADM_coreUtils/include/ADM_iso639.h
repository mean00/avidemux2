/**
 * \file ADM_iso639.h
 * \brief iso639-2 to plaintext
 * Table borrowed from handbrake
 
 */
#pragma once
#include "ADM_coreUtils6_export.h"

typedef struct ADM_iso639_t
{
    const char * eng_name;        /* Description in English */
    const char * native_name;     /* Description in native language */
    const char * iso639_1;       /* ISO-639-1 (2 characters) code */
    const char * iso639_2;        /* ISO-639-2/t (3 character) code */
    const char * iso639_2b;       /* ISO-639-2/b code (if different from above) */

} iso639_lang_t;
ADM_COREUTILS6_EXPORT const char *ADM_iso639b_toPlaintext(const char *iso);
ADM_COREUTILS6_EXPORT const ADM_iso639_t *ADM_getLanguageList();
ADM_COREUTILS6_EXPORT int                 ADM_getLanguageListSize();
ADM_COREUTILS6_EXPORT int                 ADM_getIndexForIso639(const char *iso);
