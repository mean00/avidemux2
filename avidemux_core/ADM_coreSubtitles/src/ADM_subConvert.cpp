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
    std::string startTime=std::string(ADM_us2plain(in.start));
    std::string endTime=std::string(ADM_us2plain(in.start));
    std::string result;
    
    sprintf(buffer,"Dialogue: 0,%s:%s,Default,,0000,0000,0000,,",startTime.c_str(),endTime.c_str());
    out.text=std::string(buffer);
    out.text+=in.text;
    out.start=in.start;
    out.stop=in.stop;
    return true;
}

} // namespace

