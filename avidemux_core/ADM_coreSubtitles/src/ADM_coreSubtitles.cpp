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
        int m=e.texts.size();
        for(int j=0;j<m;j++)
                printf(" --><%s> \n",e.texts[j].c_str());
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
static void writeSSAHeader(FILE *f)
{
#define W(x) fprintf(f,x"\n");
W("[Script Info]");
W("Title:");
W("Original Script:");
W("Original Translation:");
W("Original Editing:");
W("Original Timing:");
W("Synch Point:");
W("Script Updated By:");
W("Update Details:");
W("ScriptType: v4.00+");
W("Collisions: Normal");
W("PlayResY:");
W("PlayResX:");
W("PlayDepth:");
W("Timer: 100.0000");
W("WrapStyle:");
W("");
W("[V4+ Styles]");
W("Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding");
W("Style: Default,Arial,18,&H00ffffff,&H0000ffff,&H00000000,&H00000000,0,0,0,0,100,100,0,0.00,1,2,2,2,30,30,10,0");
W("");
W("[Events]");
W("Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text")
 
}
/**
 *      \fn saveAsSSA
 */
bool       ADM_subtitle::saveAsSSA(const char *out)
{
    ListOfSubtitleLines converted;
    if(_type!=SUB_TYPE_SSA)
    {
        ADM_warning("saveAsSSA: Input file is not SSA\n");
        return false;        
    }
    int n=_list.size();
    FILE *file=ADM_fopen(out,"wt");
    if(!file)
    {
        ADM_warning("Cannot create <%s>\n",out);
        return false;
    }
    writeSSAHeader(file);
    for(int i=0;i<n;i++)
    {
        subtitleTextEntry &in=_list[i];
        int m=in.texts.size();
        if(!m) continue;
        fprintf(file,"%s",in.texts[0].c_str());
        for(int j=1;j<m;j++)
        {
                fprintf(file,"\\n%s",in.texts[j].c_str());
        }
        fprintf(file,"\n");
    }
    ADM_info("%s written\n",out);
    fclose(file);
    return true;
}