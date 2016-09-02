/**
    \file ADM_update.cpp
    \brief Check for update
    \author mean (c) 2016
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_update.h"
#include "ADM_updateImpl.h"
#include "QTimer"
/**
 */
ADMCheckUpdate::ADMCheckUpdate()
{
    
}
/**
 */
ADMCheckUpdate::~ADMCheckUpdate()
{
    
}
/**
 */
void ADMCheckUpdate::execute()
{
    
}
/**
 */
void ADMCheckUpdate::downloadFinished(QNetworkReply *reply)
{
    
}
/**
 * 
 */
void ADM_checkForUpdate()
{
    ADMCheckUpdate *update=new ADMCheckUpdate;
    QTimer::singleShot(0, update, SLOT(execute()));
}
//EOF