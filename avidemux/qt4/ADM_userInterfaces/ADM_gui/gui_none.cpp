// C++ Interface: 
//
// Description: 
//
//
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "config.h"

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

void UI_setAProcessToggleStatus( uint8_t status ) {}
void UI_setVProcessToggleStatus( uint8_t status ) {}
void UI_iconify( void ) {}
void UI_deiconify( void ) {}
void UI_JumpDone(void) {}
void UI_toogleSide(void) {}
void UI_toogleMain(void) {}
uint8_t UI_arrow_enabled(void) {return 1;}
uint8_t UI_arrow_disabled(void) {return 1;}

// EOF
