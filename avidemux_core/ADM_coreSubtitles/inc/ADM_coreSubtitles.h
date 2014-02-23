/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once
#include <string>
#include <vector>
/**
 * \class subtitleTextEntry
 */
class subtitleTextEntry
{
public:    
    uint64_t    start;
    uint64_t    stop;
    std::string text;
};
typedef std::vector<subtitleTextEntry> ListOfSubtitleLines;
/**
 *      \class ADM_subtitle
 */
class ADM_subtitle
{
    typedef enum
    {
        SUB_TYPE_NONE,
        SUB_TYPE_SRT,
        SUB_TYPE_SSA
    }ADM_SUBTITLE_TYPE;
protected:
    ADM_SUBTITLE_TYPE    _type;
    ListOfSubtitleLines  _list;
public:    
                ADM_subtitle();
     virtual   ~ADM_subtitle();
     bool      load(const char *subtitleFile);
private:    
    bool       srt2ssa(const char *inputFile, const char *outputFile);
};