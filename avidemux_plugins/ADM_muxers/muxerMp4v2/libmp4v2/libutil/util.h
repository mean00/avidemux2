#ifndef MP4V2_UTIL_UTIL_H
#define MP4V2_UTIL_UTIL_H

///////////////////////////////////////////////////////////////////////////////

#include "src/src.h"

///////////////////////////////////////////////////////////////////////////////

/// @namespace mp4v2::util (private) Command-line utility support.
/// <b>WARNING: THIS IS A PRIVATE NAMESPACE. NOT FOR PUBLIC CONSUMPTION.</b>
///
/// This namespace is used for command-line utilities. Some symbols from this
/// namespace are exported from libmp4v2 in order to support new functionality
/// which may or may not make it into some form of public API, at which time
/// it will be moved out of this namespace.
///
namespace mp4v2 { namespace util {
    using namespace std;
    using namespace mp4v2::impl;
}} // namespace mp4v2::util

///////////////////////////////////////////////////////////////////////////////

#include "Database.h"
#include "Timecode.h"
#include "TrackModifier.h"
#include "Utility.h"
#include "crc.h"
#include "other.h"

///////////////////////////////////////////////////////////////////////////////

#endif // MP4V2_UTIL_UTIL_H
