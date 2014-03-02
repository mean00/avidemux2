/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
  Initial port from MPlayer by Moonz
  Mplayer version is Copyright (C) 2006 Evgeniy Stepanov <eugeni.stepanov@gmail.com>

*/


#include "ADM_default.h"
#include "ADM_coreSubtitles.h"
#include "ADM_vidMisc.h"
namespace ADM_sub
{


static const char *ADMus2Time(uint64_t ams)
{
static char buffer[256];
uint32_t ms=(uint32_t)(ams/1000);
    uint32_t hh,mm,ss,mms;
    if(ams==ADM_NO_PTS)
        sprintf(buffer,"xx:xx:xx.xxx");
    else    
    {
        ms2time(ms,&hh,&mm,&ss,&mms);
        sprintf(buffer,"%01"PRIu32":%02"PRIu32":%02"PRIu32".%03"PRIu32,hh,mm,ss,mms);
    }
    return buffer;

}

#ifndef DIR_SEP
# ifdef WIN32
#   define DIR_SEP '\\'
#   define DEFAULT_FONT_DIR "c:"
# else
#   define DIR_SEP '/'
#   define DEFAULT_FONT_DIR "/usr/share/fonts/truetype/"
# endif
#endif
//*****************



/**
 * \fn srt2ssa
 * @param in
 * @param out
 * @return 
 * 
 *  Dialogue: 0,0:41:49.86,0:41:56.95,Default,,0000,0000,0000,,Transcript : www.ydy.com/bbs | Benj!\nResynchro : Benj!.
 * 
 */
bool srt2ssa(subtitleTextEntry &in,subtitleTextEntry &out)
{
    char buffer[1024];
    char buffer2[1024];
    std::string startTime=std::string(ADMus2Time(in.start));
    std::string endTime=std::string(ADMus2Time(in.stop));
    std::string result;
    
    sprintf(buffer,"Dialogue: 0,%s,%s,Default,,0000,0000,0000,,",startTime.c_str(),endTime.c_str());
    int m=in.texts.size();
    if(m)
    {
        strcpy(buffer2,in.texts[0].c_str());
        for(int i=1;i<m;i++)
        {
               strcat(buffer2,"\\n") ;
               strcat(buffer2,in.texts[i].c_str());
        }
    
    out.texts.clear();
    strcat(buffer,buffer2);
    out.texts.push_back(std::string(buffer));
    }
    out.start=in.start;
    out.stop=in.stop;
    return true;
}

} // namespace

