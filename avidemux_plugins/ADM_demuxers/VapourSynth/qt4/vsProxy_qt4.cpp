/**
         \file Simple proxy for vsProxy
         \brief external job control
         \author mean fixounet@free.fr (c) 2015
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ui_vs.h"
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_threads.h"
#include "ADM_memsupport.h"
#include "ADM_crashdump.h"
#include "vsProxy_qt4.h"

/**
    \fn main
*/
int main(int argc, char *argv[])
{
    ADM_InitMemcpy();
#ifdef _WIN32
    win32_netInit();
#endif
    vsWindow vs(NULL);
    return vs.exec();

}
/**
 * 
 */
vsWindow::vsWindow(QWidget *parent)   : QDialog(parent)
{
    dialog.setupUi(NULL);
    
}
/**
 * 
 */
vsWindow::~vsWindow()
{
    
}
//EOF
