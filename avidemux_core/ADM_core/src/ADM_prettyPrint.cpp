/** *************************************************************************
    \file ADM_frameType
    \brief Return frametype from bitstream
                      
    copyright            : (C) 2009 by mean
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_prettyPrint.h"
#include "ADM_vidMisc.h"

static std::string myMinutes(int mm)
{
        char buffer[1024];
        sprintf(buffer,QT_TRANSLATE_NOOP("asm","%d minutes"),mm);
        return std::string(buffer);
}

static std::string myHour(int m)
{
        char buffer[1024];
        sprintf(buffer,QT_TRANSLATE_NOOP("asm","%d hours"),m);
        return std::string(buffer);
}
/**
    \fn ADM_durationToString
*/

bool ADM_durationToString(uint32_t durationInMs, std::string &outputString)
{
    uint32_t hh,mm,ss,mms;
    
   // ms2time(durationInUs/1000,&hh,&mm,&ss,&mms);
    
    if(!hh)
    {
        if(!mm)
        {
            if(ss>10)
                outputString=QT_TRANSLATE_NOOP("adm","Less than a minute");
            else
                outputString=QT_TRANSLATE_NOOP("adm","A few seconds");
            return true;
        }
        outputString=myMinutes(mm+1);
        return true;
    }
    outputString=myHour(hh)+myMinutes(mm+1);
    return true;
    
}        
        


// EOF