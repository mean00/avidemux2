/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once

class subtitleEntry
{
    uint64_t    start;
    uint64_t    stop;
    std::string text;
};

/**
 *      \class ADM_subtitle
 */
class ADM_subtitle
{
public:    
                ADM_subtitle();
     virtual   ~ADM_subtitle();
    
private:    
    bool       srt2ssa(const char *inputFile, const char *outputFile);
};