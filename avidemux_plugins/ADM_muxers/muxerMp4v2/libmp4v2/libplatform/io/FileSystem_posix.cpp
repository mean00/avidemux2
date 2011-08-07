#include "ADM_cpp.h"

#include "libplatform/impl.h"
#include <sys/stat.h>

#define ADM_LEGACY_PROGGY
#include "ADM_default.h"

// MEANX : Switch to our own wrappers

namespace mp4v2 { namespace platform { namespace io {

///////////////////////////////////////////////////////////////////////////////

bool
FileSystem::exists( string path_ )
{
    return ADM_fileExist(path_.c_str());
/*
    struct stat buf;
    return stat( path_.c_str(), &buf ) == 0;
*/
}

///////////////////////////////////////////////////////////////////////////////

bool
FileSystem::isDirectory( string path_ )
{
    struct stat buf;
    if( stat( path_.c_str(), &buf ))
        return false;
    return S_ISDIR( buf.st_mode );
}

///////////////////////////////////////////////////////////////////////////////

bool
FileSystem::isFile( string path_ )
{
    struct stat buf;
    if( stat( path_.c_str(), &buf ))
        return false;
    return S_ISREG( buf.st_mode );
}

///////////////////////////////////////////////////////////////////////////////

bool
FileSystem::getFileSize( string path_, File::Size& size_ )
{
/*
    size_ = 0;
    struct stat buf;
    if( stat( path_.c_str(), &buf ))
        return true;
    size_ = buf.st_size;
    return false;
*/
    size_=ADM_fileSize(path_.c_str());
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
FileSystem::rename( string from, string to )
{
    if(false==ADM_copyFile(from.c_str(),to.c_str()))
    {
        ADM_error("Cannot copy %s to %s\n",from.c_str(),to.c_str());
        return true;
    }
    if(false==ADM_eraseFile(from.c_str()))
    {
        ADM_error("Cannot delete %s\n",from.c_str());
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

string FileSystem::DIR_SEPARATOR  = "/";
string FileSystem::PATH_SEPARATOR = ":";

///////////////////////////////////////////////////////////////////////////////

}}} // namespace mp4v2::platform::io
