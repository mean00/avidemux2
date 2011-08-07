#include "ADM_cpp.h"
#include "libplatform/impl.h"
#define ADM_LEGACY_PROGGY
#include "ADM_default.h"

// MEANX : Change fstream based io to FILE based io to be compatible with avidemux wrapper

namespace mp4v2 { namespace platform { namespace io {

///////////////////////////////////////////////////////////////////////////////

class StandardFileProvider : public FileProvider
{
public:
    StandardFileProvider();

    bool open( std::string name, Mode mode );
    bool seek( Size pos );
    bool read( void* buffer, Size size, Size& nin, Size maxChunkSize );
    bool write( const void* buffer, Size size, Size& nout, Size maxChunkSize );
    bool close();

private:
    bool         _seekg;
    bool         _seekp;
    FILE         *_file;
};

///////////////////////////////////////////////////////////////////////////////

StandardFileProvider::StandardFileProvider()
    : _seekg ( false )
    , _seekp ( false )
{
    _file=NULL;
}

bool
StandardFileProvider::open( std::string name, Mode mode )
{
    ios::openmode om = ios::binary;
    std::string fmode;
    switch( mode ) {
        case MODE_UNDEFINED:
        case MODE_READ:
        default:
            fmode+=std::string("r");
            _seekg = true;
            _seekp = false;
            break;

        case MODE_MODIFY:
            fmode+=std::string("rw");
            _seekg = true;
            _seekp = true;
            break;

        case MODE_CREATE:
            fmode+=std::string("w");
            _seekg = true;
            _seekp = true;
            break;
    }
    fmode+=std::string("b");
    _file=fopen(name.c_str(),fmode.c_str());
    if(!_file) 
    {
        ADM_error("Cannot create file %s mode %s\n",name.c_str(),fmode.c_str());
        return true;
    }
    ADM_info("Created file %s mode %s\n",name.c_str(),fmode.c_str());
    return false;
}

bool
StandardFileProvider::seek( Size pos )
{
int er=1;
    if( _seekg )
        er=fseeko(_file,pos,SEEK_SET);
    if( _seekp )
        er=fseeko(_file,pos,SEEK_SET);
    if(er)
    {
        ADM_error("Seek to %d failed\n",(int)pos);
        return true;
    }
    return false;
}

bool
StandardFileProvider::read( void* buffer, Size size, Size& nin, Size maxChunkSize )
{
    nin=fread(buffer,1,size,_file);
    if(nin<=0) return true;
    return false;
}

bool
StandardFileProvider::write( const void* buffer, Size size, Size& nout, Size maxChunkSize )
{
    nout=fwrite(buffer,1,size,_file);
    if(nout<=0) return true;
    return false;
}

bool
StandardFileProvider::close()
{
    if(_file)
        fclose(_file);
    _file=NULL;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

FileProvider&
FileProvider::standard()
{
    return *new StandardFileProvider();
}

///////////////////////////////////////////////////////////////////////////////

}}} // namespace mp4v2::platform::io
