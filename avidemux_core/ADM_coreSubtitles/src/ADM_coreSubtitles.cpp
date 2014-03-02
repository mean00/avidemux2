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
#include "ADM_vidMisc.h"

namespace ADM_sub
{
extern bool loadSrt(const char *file,ListOfSubtitleLines &lines);
extern bool srt2ssa(subtitleTextEntry &in,subtitleTextEntry &out);
};

/**
 * 
 * @param subtitleFile
 * @return 
 */
ADM_subtitle:: ADM_subtitle()
{
    _type=SUB_TYPE_NONE;
}
/**
 * 
 * @param subtitleFile
 * @return 
 */
ADM_subtitle::~ADM_subtitle()
{
    
}
/**
 * 
 * @param subtitleFile
 * @return 
 */        
bool      ADM_subtitle::load(const char *subtitleFile)
{
    int l=strlen(subtitleFile);
    if(l<4)
    {
        ADM_warning("Subtitle file is too short <%s>\n",subtitleFile);
        return false;
    }
    const char *ext=subtitleFile+l-3;
    if(!strcasecmp(ext,"srt"))
    {
        bool r=ADM_sub::loadSrt(subtitleFile,_list);
        if(!r)
        {
            return false;
        }
        _type=SUB_TYPE_SRT;
        return true;
    }
    ADM_warning("Unknown extension <%s>, or not supported\n",ext);
    return false;
}
/**
 * \fn dump
 * @return 
 */
bool      ADM_subtitle::dump(void)
{
    int n=_list.size();
    for(int i=0;i<n;i++)
    {
        subtitleTextEntry &e=_list[i];
        printf(" %s ->",ADM_us2plain(e.start));
        printf(" %s :",ADM_us2plain(e.stop));
        printf(" <%s> \n",e.text.c_str());
    }
    return true;
}

/**
 */
bool       ADM_subtitle::srt2ssa()
{
    ListOfSubtitleLines converted;
    if(_type!=SUB_TYPE_SRT)
    {
        ADM_warning("srt2ssa: Input file is not SRT\n");
        return false;        
    }
    int n=_list.size();
    for(int i=0;i<n;i++)
    {
        subtitleTextEntry in,out;
        in=_list[i];
        ADM_sub::srt2ssa(in,out);
        converted.push_back(out);        
    }
    _list.clear();
    _list=converted;
    _type=SUB_TYPE_SSA;
    ADM_info("Converted %d entries\n",_list.size());
    return true;
}