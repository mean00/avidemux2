/**
 * \author mean fixounet@free.fr
 * 
 * \brief loader for .srt format
 * 7
 * 00:00:25,400 --> 00:00:30,800
 * La prison m'a permis de rencontrer beaucoup de personnes
 * 
 * @param in
 * @param string
 * @return 
 */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_coreSubtitles.h"

namespace ADM_sub
{

typedef enum
{
    STATE_IDLE=0,
    STATE_LINENO,
    STATE_TIMING,
    STATE_DATA
}SRT_STATE;

/**
 * \fn splitSrtTiming
 * @param start
 * @param end
 * @param str
 * @return 
 *                                                                                                                                                                      
 * 00:00:43,800 --> 00:00:48,000

 */
static uint64_t split2us(const int h2,const int m2,const int s2,const int ms2)
{
    uint64_t r=h2;
    r=(r*60)+m2;
    r=(r*60)+s2;
    r=(r*1000)+ms2;
    r=r*1000;
    return r;
}
bool splitSrtTiming(const char *str,uint64_t &start,uint64_t &end )
{
    int h1,h2,m1,m2,s1,s2,ms1,ms2;
    int n=sscanf(str,"%d:%d:%d,%d --> %d:%d:%d,%d",&h1,&m1,&s1,&ms1,&h2,&m2,&s2,&ms2);
    if(n!=8)
    {
        return false;
    }
    start=split2us(h1,m1,s1,ms1);
    end=split2us(h2,m2,s2,ms2);
    return true;
}

/**
 * \fn loadSrt
 * @param file
 * @param lines
 * @return 
 */
bool loadSrt(const char *file,ListOfSubtitleLines &lines)
{
    FILE *fd=ADM_fopen(file,"rt");
    if(!fd)
    {
        ADM_warning("Cannot open subtitle %s\n,file");
        return false;
    }
    bool status=true;
    char buffer[1024];
    SRT_STATE state=STATE_LINENO;
    subtitleTextEntry entry;
    while(1)
    {
        if(!fgets(buffer,1023,fd)) 
        {
            break;
        }
        int length=strlen(buffer);
        // remove trailing CR/LF
        char *p=buffer+length-1;
        while((*p=='\n' || *p=='\r')&& (p>buffer)) p--;
        p[1]=0;
        length=strlen(buffer);
        
        int lineno;
        printf("%d\n",state);
        switch(state)
        {
            case STATE_LINENO: 
                        if(!length) continue;
                        lineno=atoi(buffer);
                        state=STATE_TIMING;
                        break;
            case STATE_TIMING:
                        if(length<2) 
                        { 
                            ADM_warning("Inconsistent file\n");
                            status=false;
                            break;
                        }
                        uint64_t start,end;
                        if(!splitSrtTiming(buffer,start,end))
                        {
                             ADM_warning("Inconsistent timing line\n");
                             status=false;
                             break;
                        }
                        entry.start=start;
                        entry.stop=end;
                        entry.text=std::string("");
                        state=STATE_DATA;
                        break;
            case STATE_DATA:
                        if(length<2)
                        {
                            lines.push_back(entry);
                            entry.text="";
                            state=STATE_LINENO;
                        }
                        if(entry.text.length()>1) entry.text+=std::string("\n");
                        entry.text+=std::string(buffer);
                        break;
        }
    }
    fclose(fd);
    ADM_info("%d entries loaded\n",lines.size());
    return true;
}
}