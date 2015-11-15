/**
    \file ADM_last.h
    \brief Deals with last read/write file & folders
 *

*/
#pragma once
#include <string>
#include "ADM_default.h"
#include "ADM_coreUtils6_export.h"


#ifdef _WIN32
#	include <windows.h>
#endif

namespace admCoreUtils
{

ADM_COREUTILS6_EXPORT void  setLastReadFolder(const std::string &folder);
ADM_COREUTILS6_EXPORT void  getLastReadFolder( std::string &folder);

ADM_COREUTILS6_EXPORT void  setLastWriteFolder(const std::string &folder);
ADM_COREUTILS6_EXPORT void  getLastWriteFolder( std::string &folder);

};
