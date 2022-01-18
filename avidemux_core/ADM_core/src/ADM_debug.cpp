/***************************************************************************
                    Dummy function to redirect unwanted printf


    begin                : Fri Apr 20 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include <stdarg.h>
#include <math.h>
#include <pthread.h>



#define ADM_COLOR_YELLOW  "\e[33m"
#define ADM_COLOR_RED     "\e[31m"
#define ADM_COLOR_GREEN   "\e[32m"
#define ADM_DEFAULT_COLOR "\e[m"

#define ADM_DEBUG_LOG_SIZE  (1048576)  // 1MB is plenty

extern "C"
{

static pthread_mutex_t debugMutex;
static char * debugLogBuffer = NULL;
static char * debugLogReadoutBuffer = NULL;
static long unsigned int debugLogBufferHead=0;
static long unsigned int debugLogBufferTail=0;

void ADM_debugInit()
{
    if (pthread_mutex_init(&debugMutex,NULL))
        exit(errno);
    debugLogBuffer = (char *)malloc(ADM_DEBUG_LOG_SIZE*sizeof(char));
    if (debugLogBuffer==NULL)
        exit(-1);
    debugLogReadoutBuffer = (char *)malloc(ADM_DEBUG_LOG_SIZE*sizeof(char));
    if (debugLogReadoutBuffer==NULL)
        exit(-1);
    debugLogBufferHead=0;
    debugLogBufferTail=0;    
}

void ADM_debugFree()
{
    free(debugLogBuffer);
    debugLogBuffer=NULL;
    free(debugLogReadoutBuffer);
    debugLogReadoutBuffer=NULL;
}

static void ADM_debugLogWrite(const char *p)
{
    if (debugLogBuffer==NULL || p==NULL)
        return;
    debugLogBufferHead %= ADM_DEBUG_LOG_SIZE;
    debugLogBufferTail %= ADM_DEBUG_LOG_SIZE;
    int i=0;
    while(p[i])
    {
        debugLogBuffer[debugLogBufferHead] = p[i];
        debugLogBufferHead = (debugLogBufferHead+1)%ADM_DEBUG_LOG_SIZE;
        if (debugLogBufferTail==debugLogBufferHead)
            debugLogBufferTail = (debugLogBufferTail+1)%ADM_DEBUG_LOG_SIZE;
        i++;
    }
}

const char * ADM_debugLogRead(void)
{
    if (debugLogBuffer==NULL || debugLogReadoutBuffer==NULL)
        return NULL;
    long unsigned int head,tail;
    head = debugLogBufferHead % ADM_DEBUG_LOG_SIZE;
    tail = debugLogBufferTail % ADM_DEBUG_LOG_SIZE;
    long unsigned int i=0;
    for (i=0; i<ADM_DEBUG_LOG_SIZE; i++)
    {
        debugLogReadoutBuffer[i] = debugLogBuffer[tail];
        tail = (tail+1)%ADM_DEBUG_LOG_SIZE;
        if (tail==head)
            break;
    }
    i++;
    if (i >= ADM_DEBUG_LOG_SIZE)
        i = ADM_DEBUG_LOG_SIZE-1;
    debugLogReadoutBuffer[i]=0;
    debugLogReadoutBuffer[ADM_DEBUG_LOG_SIZE-1]=0;
    // skip first line, it is probably truncated
    for (i=0; i<ADM_DEBUG_LOG_SIZE-1; i++)
    {
        if (debugLogReadoutBuffer[i]==0)
            return debugLogReadoutBuffer+i;
        if (debugLogReadoutBuffer[i]=='\n')
            return debugLogReadoutBuffer+i+1;
    }
    return debugLogReadoutBuffer+ADM_DEBUG_LOG_SIZE-1;
}

static void ADM_prettyPrint(const char *func,const char *color, const char *p)
{
    static char print_buffer[1024];
    // construct time code
    struct timeval pz;
    TIMZ tz;
    gettimeofday(&pz, &tz);
    long int tvSec=pz.tv_sec;
    long int tvUSec=pz.tv_usec;

    long int mseconds = tvUSec/1000;
    long int seconds=(tvSec)%60;
    long int mn=((tvSec)/60)%60;
    long int hh=((tvSec)/3600)%24;
    
    snprintf(print_buffer, 1023, " [%s] %02d:%02d:%02d-%03d  ", func,(int)hh,(int)mn,(int)seconds,(int)mseconds);
    print_buffer[1023]=0; // ensure the string is terminated

#if _WIN32
    printf("%s%s", print_buffer,p);
#else
    if(isatty(STDOUT_FILENO))
        printf("%s%s%s%s",color,print_buffer,p,ADM_DEFAULT_COLOR);
    else
        printf("%s%s",print_buffer,p);
#endif
    ADM_debugLogWrite(print_buffer);
    ADM_debugLogWrite(p);
}

static bool verboseDebugLog=false;

void ADM_setVerboseLog(bool verbose)
{
    verboseDebugLog = verbose;
}

bool ADM_verboseLogging(void)
{
    return verboseDebugLog;
}

void ADM_info2(const char *func, const char *prf, ...)
{
    static char print_buffer[1024];

    if (pthread_mutex_lock(&debugMutex))
        return;

    va_list     list;
    va_start(list,    prf);
    vsnprintf(print_buffer,1023,prf,list);
    va_end(list);
    print_buffer[1023]=0; // ensure the string is terminated
    ADM_prettyPrint(func,ADM_COLOR_GREEN,print_buffer);
    pthread_mutex_unlock(&debugMutex);
}

void ADM_warning2( const char *func, const char *prf, ...)
{
    static char print_buffer[1024];

    if (pthread_mutex_lock(&debugMutex))
        return;

    va_list     list;
    va_start(list,    prf);
    vsnprintf(print_buffer,1023,prf,list);
    va_end(list);
    print_buffer[1023]=0; // ensure the string is terminated
    ADM_prettyPrint(func,ADM_COLOR_YELLOW,print_buffer);
    pthread_mutex_unlock(&debugMutex);
}

void ADM_error2( const char *func, const char *prf, ...)
{
    static char print_buffer[1024];

    if (pthread_mutex_lock(&debugMutex))
        return;

    va_list     list;
    va_start(list,    prf);
    vsnprintf(print_buffer,1023,prf,list);
    va_end(list);
    print_buffer[1023]=0; // ensure the string is terminated
    ADM_prettyPrint(func,ADM_COLOR_RED,print_buffer);
    pthread_mutex_unlock(&debugMutex);
}


}
//EOF
