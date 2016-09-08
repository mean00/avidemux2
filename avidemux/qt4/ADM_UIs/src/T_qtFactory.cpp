/***************************************************************************
  \fn qtFactory
 * \ author mean (c) 2016
 * \brief factorize some code
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "T_menu.h"
#include "ADM_default.h"
#include "ADM_dialogFactoryQt4.h"

#include <QGridLayout>
#include <QLabel>

/**
 * \fn titleFromShortKey
 * \brief convert accelerator
 * @param in
 * @return 
 */
QtFactoryUtils::QtFactoryUtils(const char *in)
{
    titleFromShortKey(in);
}
bool    QtFactoryUtils::titleFromShortKey(const char *in)
{
    myQtTitle = QString::fromUtf8(in);
    myQtTitle.replace("&", "&&");
    myQtTitle.replace("_", "&");
    return true;
}
// EOF
