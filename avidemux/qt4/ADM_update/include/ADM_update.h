/**
    \file ADM_update.h
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
#pragma once
#include <string>
typedef void ADM_updateComplete(int  version, const std::string &releaseDate,const std::string &downloadlink);

void ADM_checkForUpdate(ADM_updateComplete *cb);