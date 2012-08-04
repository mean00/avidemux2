/***************************************************************************
                          toolkit.cpp  -  description
                             -------------------



    begin                : Fri Dec 14 2001
    copyright            : (C) 2001 by mean
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "ADM_default.h"
#include "ADM_assert.h"
#include "ADM_win32.h"

#include "ADM_vidMisc.h"
#undef fopen
#undef fclose

// Get tick (in ms)
// Call with a 0 to initialize
// Call with a 1 to read
//_____________________
uint32_t getTime(int called)
{

    static struct timeval timev_s;

    static struct timeval timev;
    static TIMZ timez;

    int32_t tt;

    if (!called)
      {
	  called = 1;
	  gettimeofday(&timev_s, &timez);
	  return 0;
      }
    gettimeofday(&timev, &timez);
    tt = timev.tv_usec - timev_s.tv_usec;
    tt /= 1000;
    tt += 1000 * (timev.tv_sec - timev_s.tv_sec);
    return (tt);
}

/**
    \fn    ADM_getSecondsSinceEpoch
    \brief returns the number of seconds since epoch
*/
uint64_t ADM_getSecondsSinceEpoch(void)
{
    struct timeval timev;
    TIMZ timez;

    uint64_t tt;

    gettimeofday(&timev, &timez);
    tt =  (uint64_t)(timev.tv_sec);
    return tt;
}
/**
    \fn ADM_epochToString
    \brief convert number of second to string
*/
const char *ADM_epochToString(uint64_t epoch)
{
    time_t clock=(time_t)epoch;
    return ctime(&clock);
}

uint32_t getTimeOfTheDay(void)
{
  

     struct timeval timev;
     TIMZ timez;

    int32_t tt;

   
    gettimeofday(&timev, &timez);
    tt = timev.tv_usec;
    tt /= 1000;
    tt += 1000 * (timev.tv_sec);
    return (tt&0xffffff);

}
/// convert frame number and fps to hour/mn/sec/ms
void  frame2time(	uint32_t frame, uint32_t fps, uint32_t * hh, uint32_t * mm,
                                uint32_t * ss, uint32_t * ms)
{
    UNUSED_ARG(fps);
double d;
    uint32_t len2;
    d=frame;
    d=d/fps;
    d*=1000000.;
    
    len2 = (uint32_t)(d); //video_body->getTime(frame);
    ms2time(len2,hh,mm,ss,ms);
}

void            time2frame(uint32_t *frame, uint32_t fps, uint32_t hh, uint32_t mm,
                                uint32_t ss, uint32_t ms)
{
// convert everything to ms : uint32_t = 1000 hours, should be plenty enough
uint32_t count=0;
                count+=ms;
                count+=ss*1000;
                count+=mm*60*1000;
                count+=hh*3600*1000;
double d;
                d=count;                
                // ms
                d=d*fps;
                d/=1000;
                d/=1000;
                *frame= (uint32_t)(floor(d+0.5));

}

uint64_t ADM_swap64(uint64_t in)
{
uint32_t low,high;
uint64_t out;
        high=in>>32;
        low=in&0xffffffff;
        high=ADM_swap32(high);
        low=ADM_swap32(low);
        out=low;
        out=(out<<32)+high;
        return out;
  
}
// swap BE/LE : Ugly
uint32_t ADM_swap32( uint32_t in)
{
        uint8_t r[4],u;
        memcpy(&r[0],&in,4);
        u=r[0];
        r[0]=r[3];
        r[3]=u;
        u=r[1];
        r[1]=r[2];
        r[2]=u;
        memcpy(&in,&r[0],4);
        return in;
}
// swap BE/LE : Ugly
uint16_t ADM_swap16( uint16_t in)
{
	return ( (in>>8) & 0xff) + ( (in&0xff)<<8);
}
uint8_t 	identMovieType(uint32_t fps1000)
{
#define INRANGE(value,type)  \
      {\
              if((fps1000 > value-300) &&( fps1000 < value+300))\
              {\
                r=type;\
                printf("Looks like "#type" \n");\
                }\
      }
      uint8_t r=0;
      INRANGE(25000,FRAME_PAL);
      INRANGE(23976,FRAME_FILM);
      INRANGE(29970,FRAME_NTSC);

      return r;
}
uint8_t ms2time(uint32_t ms, uint32_t *h,uint32_t *m, uint32_t *s,uint32_t *mms)
{
      uint32_t sectogo;
      int  mm,ss,hh;


                              // d is in ms, divide by 1000 to get seconds
                              sectogo = (uint32_t) floor(ms / 1000.);
                              hh=sectogo/3600;
                              sectogo=sectogo-hh*3600;
                              mm=sectogo/60;
                              ss=sectogo%60;

                              *h=hh;
                              *m=mm;
                              *s=ss;
                              *mms=ms-1000*(ms/1000);
      return 1;
}
/**
    \fn The Office_6x08_HDTV.2HD.en
    \brief convert a time in us to a string in 0:0:0'0 format
*/
const char *ADM_us2plain(uint64_t ams)
{
static char buffer[256];
uint32_t ms=(uint32_t)(ams/1000);
    uint32_t hh,mm,ss,mms;
    if(ams==ADM_NO_PTS)
        sprintf(buffer," xx:xx:xx,xxx ");
    else    
    {
        ms2time(ms,&hh,&mm,&ss,&mms);
        sprintf(buffer," %02"LU":%02"LU":%02"LU",%03"LU" ",hh,mm,ss,mms);
    }
    return buffer;

}

/**
        \fn ADM_LowerCase
        \brief change to lower case in place the string
*/
void  ADM_LowerCase(char *string)
{
  int l=strlen(string)-1;
        for(int i=l;i>=0;i--)
        {
                string[i]=tolower(string[i]);
        }

}

uint8_t         ADM_fileExist(const char *name)
{
FILE *file;
                file=ADM_fopen(name,"rb");
                if(!file) return 0;
                fclose(file);
                return 1;

}
/*
    In some case (e.g. javascript), the reader expects unixish path 
    c:/foo/bar/c.c
    and the "natural" path is c:\foo\bar
    
    This function convert the later to the former

*/
extern char *ADM_slashToBackSlash(const char *in)
{
    char *out,*cout;
    int n;
    n=strlen(in);
    cout=out=(char *)ADM_alloc(n+1);   
    for(int i=0;i<n+1;i++)
    {
        if(   in[i]=='\\') out[i]='/';
        else    out[i]=in[i];
        
    }
    return cout;
    
}
/*
    
*/
void TLK_getDate(ADM_date *date)
{
  time_t timez;
  tm *t;
  time(&timez);
  t=localtime(&timez);
  if(t)
  {
    date->hours=t->tm_hour;
    date->minutes=t->tm_min;
    date->seconds=t->tm_sec;
  }
  
}

bool shutdown(void)
{
#ifdef _WIN32
	return (shutdown_win32() == 0);
#else
	return (system("shutdown -P 0") == 0);
#endif
}
//EOF
