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
    STATE_IDLE,
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
 */
bool splitSrtTiming(const char *str,uint64_t &start,uint64_t &end )
{
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
        if(!fgets(buffer,1023,fd)) break;
        int length=strlen(buffer);
        int lineno;
        switch(state)
        {
            case STATE_LINENO: 
                        if(!length) continue;
                        lineno=atoi(buffer);
                        state=STATE_TIMING;
                        break;
            case STATE_TIMING:
                        if(!length) 
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
                        state=STATE_DATA;
                        break;
            case STATE_DATA:
                        if(!length)
                        {
                            lines.push_back(entry);
                            entry.text="";
                            state=STATE_LINENO;
                        }
                        entry.text+=std::string("/n")+std::string(buffer);
                        break;
        }
    }
    fclose(fd);
    return true;
}
}