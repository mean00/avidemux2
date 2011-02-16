///////////////////////////////////////////////////////////////////////////////
//
//  The contents of this file are subject to the Mozilla Public License
//  Version 1.1 (the "License"); you may not use this file except in
//  compliance with the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
//
//  Software distributed under the License is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
//  License for the specific language governing rights and limitations
//  under the License.
// 
//  The Original Code is MP4v2.
// 
//  The Initial Developer of the Original Code is Kona Blend.
//  Portions created by Kona Blend are Copyright (C) 2008.
//  All Rights Reserved.
//
//  Contributors:
//      Kona Blend, kona8lend@@gmail.com
//
///////////////////////////////////////////////////////////////////////////////

#include "libutil/impl.h"

namespace mp4v2 { namespace util {

///////////////////////////////////////////////////////////////////////////////

Utility::Utility( string name_, int argc_, char** argv_ )
    : _longOptions      ( NULL )
    , _name             ( name_ )
    , _argc             ( argc_ )
    , _argv             ( argv_ )
    , _optimize         ( false )
    , _dryrun           ( false )
    , _keepgoing        ( false )
    , _overwrite        ( false )
    , _force            ( false )
    , _debug            ( 0 )
    , _verbosity        ( 1 )
    , _jobCount         ( 0 )
    , _debugImplicits   ( false )
    , _group            ( "OPTIONS" )

,STD_OPTIMIZE( 'z', false, "optimize", false, LC_NONE, "optimize mp4 file after modification" )
,STD_DRYRUN( 'y', false, "dryrun", false, LC_NONE, "do not actually create or modify any files" )
,STD_KEEPGOING( 'k', false, "keepgoing", false, LC_NONE, "continue batch processing even after errors" )
,STD_OVERWRITE( 'o', false, "overwrite", false, LC_NONE, "overwrite existing files when creating" )
,STD_FORCE( 'f', false, "force", false, LC_NONE, "force overwrite even if file is read-only" )
,STD_QUIET( 'q', false, "quiet", false, LC_NONE, "equivalent to --verbose 0" )
,STD_DEBUG( 'd', false, "debug", true, LC_DEBUG, "increase debug or long-option to set NUM", "NUM",
    // 79-cols, inclusive, max desired width
    // |----------------------------------------------------------------------------|
    "\nDEBUG LEVELS (for raw mp4 file I/O)"
    "\n  0  supressed"
    "\n  1  add warnings and errors (default)"
    "\n  2  add table details"
    "\n  3  add implicits"
    "\n  4  everything" )
,STD_VERBOSE( 'v', false, "verbose", true, LC_VERBOSE, "increase verbosity or long-option to set NUM", "NUM",
    // 79-cols, inclusive, max desired width
    // |----------------------------------------------------------------------------|
    "\nVERBOSE LEVELS"
    "\n  0  warnings and errors"
    "\n  1  normal informative messages (default)"
    "\n  2  more informative messages"
    "\n  3  everything" )
,STD_HELP( 'h', false, "help", false, LC_HELP, "print brief help or long-option for extended help" )
,STD_VERSION( 0, false, "version", false, LC_VERSION, "print version information and exit" )
,STD_VERSIONX( 0, false, "versionx", false, LC_VERSIONX, "print extended version information", "ARG", "", true )

{
    debugUpdate( 1 );

    _usage = "<UNDEFINED>";
    _description = "<UNDEFINED>";
    _groups.push_back( &_group );
}

///////////////////////////////////////////////////////////////////////////////

Utility::~Utility()
{
    delete[] _longOptions;
}

///////////////////////////////////////////////////////////////////////////////

bool
Utility::batch( int argi )
{
    _jobCount = 0;
    _jobTotal = _argc - argi;

    // nothing to be done
    if( !_jobTotal )
        return SUCCESS;

    bool batchResult = FAILURE;
    for( int i = argi; i < _argc; i++ ) {
        bool subResult = FAILURE;
        try {
            if( !job( _argv[i] )) {
                batchResult = SUCCESS;
                subResult = SUCCESS;
            }
        }
        catch( Exception* x ) {
            mp4v2::impl::log.errorf(*x);
            delete x;
        }
 
        if( !_keepgoing && subResult == FAILURE )
            return FAILURE;
    }
    
    return batchResult;
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::debugUpdate( uint32_t debug )
{
    MP4LogLevel level;

    _debug = debug;
    verbose2f( "debug level: %u\n", _debug );

    switch( _debug ) {
        case 0:
            level = MP4_LOG_NONE;
            _debugImplicits = false;
            break;

        case 1:
            level = MP4_LOG_ERROR;
            _debugImplicits = false;
            break;

        case 2:
            level = MP4_LOG_VERBOSE2;
            _debugImplicits = false;
            break;

        case 3:
            level = MP4_LOG_VERBOSE2;
            _debugImplicits = true;
            break;

        case 4:
        default:
            level = MP4_LOG_VERBOSE4;
            _debugImplicits = true;
            break;
    }

    MP4LogSetLevel(level);
}

///////////////////////////////////////////////////////////////////////////////

bool
Utility::dryrunAbort()
{
    if( !_dryrun )
        return false;

    verbose2f( "skipping: dry-run mode enabled\n" );
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::errf( const char* format, ... )
{
    va_list ap;
    va_start( ap, format );
    vfprintf( stderr, format, ap );
    va_end( ap );
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::formatGroups()
{
    // determine longest long-option [+space +argname]
    int longMax = 0;
    list<Group*>::reverse_iterator ie = _groups.rend();
    for( list<Group*>::reverse_iterator it = _groups.rbegin(); it != ie; it++ ) {
        Group& group = **it;
        const Group::List::const_iterator ieo = group.options.end();
        for( Group::List::const_iterator ito = group.options.begin(); ito != ieo; ito++ ) {
            const Option& option = **ito;
            if( option.hidden )
                continue;

            int len = (int)option.lname.length();
            if( option.lhasarg )
                len += 1 + (int)option.argname.length();
            if( len > longMax )
                longMax = len;
        }
    }

    // format help output (no line-wrapping yet)
    ostringstream oss;

    int groupCount = 0;
    int optionCount = 0;
    ie = _groups.rend();
    for( list<Group*>::reverse_iterator it = _groups.rbegin(); it != ie; it++, groupCount++ ) {
        if( groupCount )
            oss << '\n';
        Group& group = **it;
        oss << '\n' << group.name;
        const Group::List::const_iterator ieo = group.options.end();
        for( Group::List::const_iterator ito = group.options.begin(); ito != ieo; ito++, optionCount++ ) {
            const Option& option = **ito;
            if( option.hidden )
                continue;

            oss << "\n ";

            if( option.scode == 0 )
                oss << "    --";
            else
                oss << '-' << option.scode << ", --";

            if( option.lhasarg ) {
                oss << option.lname << ' ' << option.argname;
                oss << setw( longMax - option.lname.length() - 1 - option.argname.length() ) << "";
            }
            else {
                oss << setw( longMax ) << left << option.lname;
            }

            oss << "  ";

            const string::size_type imax = option.descr.length();
            for( string::size_type i = 0; i < imax; i++ )
                oss << option.descr[i];
        }
    }

    _help = oss.str();

    // allocate and populate C-style options
    delete[] _longOptions;
    _longOptions = new prog::Option[optionCount + 1];

    // fill EOL marker
    _longOptions[optionCount].name = NULL;
    _longOptions[optionCount].type = prog::Option::NO_ARG;
    _longOptions[optionCount].flag = 0;
    _longOptions[optionCount].val  = 0;

    _shortOptions.clear();

    int optionIndex = 0;
    ie = _groups.rend();
    for( list<Group*>::reverse_iterator it = _groups.rbegin(); it != ie; it++ ) {
        Group& group = **it;
        const Group::List::const_iterator ieo = group.options.end();
        for( Group::List::const_iterator ito = group.options.begin(); ito != ieo; ito++, optionIndex++ ) {
            const Option& a = **ito;
            prog::Option& b = _longOptions[optionIndex];

            b.name = const_cast<char*>(a.lname.c_str());
            b.type = a.lhasarg ? prog::Option::REQUIRED_ARG : prog::Option::NO_ARG;
            b.flag = 0;
            b.val  = (a.lcode == LC_NONE) ? a.scode : a.lcode;

            if( a.scode != 0 ) {
                _shortOptions += a.scode;
                if( a.shasarg )
                    _shortOptions += ':';
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
Utility::job( string arg )
{
    verbose2f( "job begin: %s\n", arg.c_str() );

    // perform job
    JobContext job( arg );
    bool result = FAILURE;
    try {
        result = utility_job( job );
    }
    catch( Exception* x ) {
        mp4v2::impl::log.errorf(*x);
        delete x;
    }

    // close file handle flagged with job
    if( job.fileHandle != MP4_INVALID_FILE_HANDLE ) {
        verbose2f( "closing %s\n", job.file.c_str() );
        MP4Close( job.fileHandle );

        // invoke optimize if flagged
        if( _optimize && job.optimizeApplicable ) {
            verbose1f( "optimizing %s\n", job.file.c_str() );
            if( !MP4Optimize( job.file.c_str(), NULL ))
                hwarnf( "optimize failed: %s\n", job.file.c_str() );
        }
    }

    // free data flagged with job
    list<void*>::iterator ie = job.tofree.end();
    for( list<void*>::iterator it = job.tofree.begin(); it != ie; it++ )
        free( *it );


    verbose2f( "job end\n" );
    _jobCount++;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool
Utility::herrf( const char* format, ... )
{
    va_list ap;
    va_start( ap, format );

    if( _keepgoing ) {
        fprintf( stdout, "WARNING: " );
        vfprintf( stdout, format, ap );
    }
    else {
        fprintf( stderr, "ERROR: " );
        vfprintf( stderr, format, ap );
    }

    va_end( ap );
    return FAILURE;
}

///////////////////////////////////////////////////////////////////////////////

bool
Utility::hwarnf( const char* format, ... )
{
    fprintf( stdout, "WARNING: " );
    va_list ap;
    va_start( ap, format );
    vfprintf( stdout, format, ap );
    va_end( ap );
    return FAILURE;
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::outf( const char* format, ... )
{
    va_list ap;
    va_start( ap, format );
    vfprintf( stdout, format, ap );
    va_end( ap );
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::printHelp( bool extended, bool toerr )
{
    ostringstream oss;
    oss << "Usage: " << _name << " " << _usage << '\n' << _description << '\n' << _help;

    if( extended ) {
        const list<Group*>::reverse_iterator ie = _groups.rend();
        for( list<Group*>::reverse_iterator it = _groups.rbegin(); it != ie; it++ ) {
            Group& group = **it;
            const Group::List::const_iterator ieo = group.options.end();
            for( Group::List::const_iterator ito = group.options.begin(); ito != ieo; ito++ ) {
                const Option& option = **ito;
                if( option.help.empty() )
                    continue;

                oss << '\n' << option.help;
            }
        }
    }

    if( toerr )
        errf( "%s\n\n", oss.str().c_str() );
    else
        outf( "%s\n\n", oss.str().c_str() );
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::printUsage( bool toerr )
{
    ostringstream oss;
    oss <<   "Usage: " << _name << " " << _usage
        << "\nTry -h for brief help or --help for extended help";
 
    if( toerr )
        errf( "%s\n", oss.str().c_str() );
    else
        outf( "%s\n", oss.str().c_str() );
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::printVersion( bool extended )
{
    ostringstream oss;
    oss << left;

    if( extended ) {
        oss <<         setw(13) << "utility:" << _name
            << '\n' << setw(13) << "product:" << MP4V2_PROJECT_name
            << '\n' << setw(13) << "version:" << MP4V2_PROJECT_version
            << '\n' << setw(13) << "build date:" << MP4V2_PROJECT_build
            << '\n'
            << '\n' << setw(18) << "repository URL:" << MP4V2_PROJECT_repo_url
            << '\n' << setw(18) << "repository root:" << MP4V2_PROJECT_repo_root
            << '\n' << setw(18) << "repository UUID:" << MP4V2_PROJECT_repo_uuid
            << '\n' << setw(18) << "repository rev:" << MP4V2_PROJECT_repo_rev
            << '\n' << setw(18) << "repository date:" << MP4V2_PROJECT_repo_date
            << '\n' << setw(18) << "repository type:" << MP4V2_PROJECT_repo_type;
    }
    else {
        oss << _name << " - " << MP4V2_PROJECT_name_formal;
    }

    outf( "%s\n", oss.str().c_str() );
}

///////////////////////////////////////////////////////////////////////////////

bool
Utility::process()
{
    bool rv = true;

    try {
        rv = process_impl();
    }
    catch( Exception* x ) {
        _keepgoing = false;
        mp4v2::impl::log.errorf(*x);
        delete x;
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////

bool
Utility::process_impl()
{
    formatGroups();

    // populate code lookup set
    set<int> codes;
    const Group::List::const_iterator ie = _group.options.end();
    for( Group::List::const_iterator it = _group.options.begin(); it != ie; it++ ) {
        const Option& option = **it;
        if( option.scode != 0 )
            codes.insert( option.scode );
        if( option.lcode != LC_NONE )
            codes.insert( option.lcode );
    }

    for( ;; ) {
        const int code = prog::getOption( _argc, _argv, _shortOptions.c_str(), _longOptions, NULL );
        if( code == -1 )
            break;

        bool handled = false;
        if( utility_option( code, handled ))
            return FAILURE;
        if( handled )
            continue;

        if( codes.find( code ) == codes.end() )
            continue;

        switch( code ) {
            case 'z':
                _optimize = true;
                break;

            case 'y':
                _dryrun = true;
                break;

            case 'k':
                _keepgoing = true;
                break;

            case 'o':
                _overwrite = true;
                break;

            case 'f':
                _force = true;
                break;

            case 'q':
                _verbosity = 0;
                debugUpdate( 0 );
                break;

            case 'v':
                _verbosity++;
                break;

            case 'd':
                debugUpdate( _debug + 1 );
                break;

            case 'h':
                printHelp( false, false );
                return SUCCESS;

            case LC_DEBUG:
                debugUpdate( std::strtoul( prog::optarg, NULL, 0 ) );
                break;

            case LC_VERBOSE:
            {
                const uint32_t level = std::strtoul( prog::optarg, NULL, 0 );
                _verbosity = ( level < 4 ) ? level : 3;
                break;
            }

            case LC_HELP:
                printHelp( true, false );
                return SUCCESS;

            case LC_VERSION:
                printVersion( false );
                return SUCCESS;

            case LC_VERSIONX:
                printVersion( true );
                return SUCCESS;

            default:
                printUsage( true );
                return FAILURE;
        }
    }

    if( !(prog::optind < _argc) ) {
        printUsage( true );
        return FAILURE;
    }

    const bool result = batch( prog::optind );
    verbose2f( "exit code %d\n", result );
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool
Utility::openFileForWriting( io::File& file )
{
    // simple case is file does not exist
    if( !io::FileSystem::exists( file.name )) {
        if( file.open() )
            return herrf( "unable to open %s for write: %s\n", file.name.c_str(), sys::getLastErrorStr() );
        return SUCCESS;
    }

    // fail if overwrite is not enabled
    if( !_overwrite )
        return herrf( "file already exists: %s\n", file.name.c_str() );

    // only overwrite if it is a file
    if( !io::FileSystem::isFile( file.name ))
        return herrf( "cannot overwrite non-file: %s\n", file.name.c_str() );

    // first attemp to re-open/truncate so as to keep any file perms
    if( !file.open() )
        return SUCCESS;

    // fail if force is not enabled
    if( !_force )
        return herrf( "unable to overwrite file: %s\n", file.name.c_str() );

    // first attempt to open, truncating file
    if( !file.open() )
        return SUCCESS;

    // nuke file
    if( ::remove( file.name.c_str() ))
        return herrf( "unable to remove %s: %s\n", file.name.c_str(), sys::getLastErrorStr() );

    // final effort
    if( !file.open() )
        return SUCCESS;

    return herrf( "unable to open %s for write: %s\n", file.name.c_str(), sys::getLastErrorStr() );
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::verbose( uint32_t level, const char* format, va_list ap )
{
    if( level > _verbosity )
        return;
    vfprintf( stdout, format, ap );
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::verbose1f( const char* format, ... )
{
    va_list ap;
    va_start( ap, format );
    verbose( 1, format, ap );
    va_end( ap );
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::verbose2f( const char* format, ... )
{
    va_list ap;
    va_start( ap, format );
    verbose( 2, format, ap );
    va_end( ap );
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::verbose3f( const char* format, ... )
{
    va_list ap;
    va_start( ap, format );
    verbose( 3, format, ap );
    va_end( ap );
}

///////////////////////////////////////////////////////////////////////////////

const bool Utility::SUCCESS = false;
const bool Utility::FAILURE = true;

///////////////////////////////////////////////////////////////////////////////

Utility::Group::Group( string name_ )
    : name    ( name_ )
    , options ( _options )
{
}

///////////////////////////////////////////////////////////////////////////////

Utility::Group::~Group()
{
    const List::iterator ie = _optionsDelete.end();
    for( List::iterator it = _optionsDelete.begin(); it != ie; it++ )
        delete *it;
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::Group::add( const Option& option )
{
    _options.push_back( &option );
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::Group::add(
    char     scode,
    bool     shasarg,
    string   lname,
    bool     lhasarg,
    uint32_t lcode,
    string   descr,
    string   argname,
    string   help,
    bool     hidden )
{
    Option* o = new Option( scode, shasarg, lname, lhasarg, lcode, descr, argname, help, hidden );
    _options.push_back( o );
    _optionsDelete.push_back( o );
}

///////////////////////////////////////////////////////////////////////////////

void
Utility::Group::add( 
    string   lname,
    bool     lhasarg,
    uint32_t lcode,
    string   descr,
    string   argname,
    string   help,
    bool     hidden )
{
    add( 0, false, lname, lhasarg, lcode, descr, argname, help, hidden );
}

///////////////////////////////////////////////////////////////////////////////

Utility::Option::Option(
    char     scode_,
    bool     shasarg_,
    string   lname_,
    bool     lhasarg_,
    uint32_t lcode_,
    string   descr_,
    string   argname_,
    string   help_,
    bool     hidden_ )
        : scode   ( scode_ )
        , shasarg ( shasarg_ )
        , lname   ( lname_ )
        , lhasarg ( lhasarg_ )
        , lcode   ( lcode_ )
        , descr   ( descr_ )
        , argname ( argname_ )
        , help    ( help_ )
        , hidden  ( hidden_ )
{
}

///////////////////////////////////////////////////////////////////////////////

Utility::JobContext::JobContext( string file_ )
    : file               ( file_ )
    , fileHandle         ( MP4_INVALID_FILE_HANDLE )
    , optimizeApplicable ( false )
{
}

///////////////////////////////////////////////////////////////////////////////

}} // namespace mp4v2::util
