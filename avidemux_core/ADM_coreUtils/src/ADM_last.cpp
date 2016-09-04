/***************************************************************************
   
    \file  ADM_last.cpp
    \brief Some utility functions to deal with last read file/folder
    copyright            : (C) 2015
    email                : fixounet@free.fr
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_last.h"
#include "prefs.h"

/**
 * \fn setLastReadFolder
 * @param folder
 */
static void internalSetFolder(options tag,const std::string &folder)
{
        if (!prefs->set(tag, folder))
             ADM_warning("Cannot set last Read folder for %s\n",folder.c_str());   
}
/**
 * \fn getLastReadFolder
 * @param folder
 */
static void internalGetFolder(options tag,  std::string &folder)
{
    std::string tmp;
    if (!prefs->get(tag, folder))
    {
        ADM_warning("Cannot set last Read folder for %s\n",folder.c_str());   
        folder=std::string("");
    }
}


/**
 * \fn setLastReadFolder
 * @param folder
 */
void admCoreUtils::setLastReadFolder(const std::string &folder)
{
    internalSetFolder(LASTFILES_LASTDIR_READ,folder);
}
/**
 * 
 * @param folder
 */
void admCoreUtils::setLastWriteFolder(const std::string &folder)
{
    internalSetFolder(LASTFILES_LASTDIR_WRITE,folder);
}

/**
 * \fn getLastReadFolder
 * @param folder
 */
void admCoreUtils::getLastReadFolder( std::string &folder)
{
    internalGetFolder(LASTFILES_LASTDIR_READ,folder);      
    
}
void admCoreUtils::getLastWriteFolder( std::string &folder)
{
    internalGetFolder(LASTFILES_LASTDIR_WRITE,folder);      
    
}

// EOF