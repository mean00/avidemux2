//
// C++ Interface: audiofilter_dolby
//
// Description: 
//
//
// Author: Mihail Zenkov <kreator@tut.by>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef AUDM_DOLBY_H
#define AUDM_DOLBY_H

extern void DolbyInit();
extern void DolbySkip(bool on);
extern float DolbyShiftLeft(float isamp);
extern float DolbyShiftRight(float isamp);

#endif
