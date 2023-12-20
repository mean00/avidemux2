/***************************************************************************
   \file ADM_pyTools.cpp
    \brief misc. helpers, not strictly related to ADM
    \author szlldm 2023
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_pyTools.h"
#include "ADM_default.h"

double pyTool_time(IEditor *editor)
{
    struct timeval pz;
    TIMZ tz;
    gettimeofday(&pz, &tz);
    double t = pz.tv_usec;
    t /= 1000000.0;
    t += pz.tv_sec;
    return t;
}

char * pyTool_date(IEditor *editor)
{
    struct timeval pz;
    TIMZ tz;
    gettimeofday(&pz, &tz);
    time_t timez;
    tm *t;
    time(&timez);
    t=localtime(&timez);
    char tmbuf[64];
    const char *formatString =
#ifdef _WIN32
    "%Y-%m-%d %Hh%Mm%Ss";
#else
    "%Y-%m-%d %H:%M:%S";
#endif
    strftime(tmbuf, 63, formatString, t);
    return ADM_strdup(tmbuf);
}

static uint32_t rng_state = 0;

int pyTool_randint(IEditor *editor, int start, int stop)
{
    while (rng_state == 0)
    {
        struct timeval pz;
        TIMZ tz;
        gettimeofday(&pz, &tz);
        if (!pz.tv_usec)
        {
            ADM_usleep(1);
            continue;
        }
        rng_state = pz.tv_usec;
    }
    if (start == stop) return start;
    if (stop < start)
    {
        int tmp = stop;
        stop = start;
        start = tmp;
    }

    uint64_t rng_product;    
    rng_product = (uint64_t)rng_state * 48271;
    rng_state = (rng_product & 0x7fffffff) + (rng_product >> 31);
    rng_state = (rng_state & 0x7fffffff) + (rng_state >> 31);
    uint32_t rn = rng_state;
    rn %= (stop + 1 - start);
    int ri = rn;
    return ri + start;
}

int pyTool_isalnum(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!isalnum(str[i])) return 0; }
    return 1;
}

int pyTool_isalpha(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!isalpha(str[i])) return 0; }
    return 1;
}

int pyTool_isdigit(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!isdigit(str[i])) return 0; }
    return 1;
}

int pyTool_islower(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!islower(str[i])) return 0; }
    return 1;
}

int pyTool_isspace(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!isspace(str[i])) return 0; }
    return 1;
}

int pyTool_isupper(IEditor *editor, const char * str)
{
    for (int i=0; i<strlen(str); i++) { if (!isupper(str[i])) return 0; }
    return 1;
}

char* pyTool_upper(IEditor *editor, const char * str)
{
    char * t = ADM_strdup(str);
    for (int i=0; i<strlen(str); i++) { t[i] = toupper(t[i]); }
    return t;
}

char* pyTool_lower(IEditor *editor, const char * str)
{
    char * t = ADM_strdup(str);
    for (int i=0; i<strlen(str); i++) { t[i] = tolower(t[i]); }
    return t;
}

// EOF
