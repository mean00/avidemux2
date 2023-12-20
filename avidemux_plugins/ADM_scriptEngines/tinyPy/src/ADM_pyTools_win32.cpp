/***************************************************************************
    \file ADM_pyTools_win32.cpp
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

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

/* Stolen from https://stackoverflow.com/questions/10905892/equivalent-of-gettimeofday-for-windows */

double pyTool_time(IEditor *editor)
{
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t epochOffset = 116444736000000000ULL;

    SYSTEMTIME sysTime;
    FILETIME fileTime;
    uint64_t time;

    GetSystemTime(&sysTime);
    SystemTimeToFileTime(&sysTime, &fileTime);

    time = fileTime.dwLowDateTime;
    time += ((uint64_t)fileTime.dwHighDateTime) << 32;

    if (epochOffset > time)
        return 0.0;
    time -= epochOffset;
    double t = sysTime.wMilliseconds;
    t /= 1000.0;
    t += time / 10000000ULL;
    return t;
}

char * pyTool_date(IEditor *editor)
{
    SYSTEMTIME st;

    GetLocalTime(&st); // should the returned struct be validated?

#define TM_BUFLEN 64
    char tmbuf[TM_BUFLEN];
    snprintf(tmbuf, TM_BUFLEN, "%04d-%02d-%02d %02dh%02dm%02ds", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return ADM_strdup(tmbuf);
}

static uint32_t rng_state = 0;

int pyTool_randint(IEditor *editor, int start, int stop)
{
    while (rng_state == 0)
    {
        SYSTEMTIME st;
        GetSystemTime(&st);
        if (!st.wMilliseconds)
        {
            ADM_usleep(1000);
            continue;
        }
        rng_state = st.wMilliseconds;
        rng_state *= 1000;
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
