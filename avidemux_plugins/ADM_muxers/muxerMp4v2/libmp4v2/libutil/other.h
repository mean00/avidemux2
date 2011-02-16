#ifndef MP4V2_UTIL_OTHER_H
#define MP4V2_UTIL_OTHER_H

///////////////////////////////////////////////////////////////////////////////

namespace mp4v2 { namespace util {

///////////////////////////////////////////////////////////////////////////////

struct MP4V2_EXPORT FileSummaryInfo {
    typedef set<string>  BrandSet;

    // standard ftyp box attributes
    string   major_brand;
    uint32_t minor_version;
    BrandSet compatible_brands;

    uint32_t nlargesize;
    uint32_t nversion1;
    uint32_t nspecial;
};

///////////////////////////////////////////////////////////////////////////////
///
/// Fetch mp4 file summary information.
///
/// This function fetches summary information for <b>file</b> and information
/// is stored in <b>info</b>.
///
/// @return On success <b>true</b>.
///     On failure <b>false</b>, and contents of <b>info</b> are undefined.
///
MP4V2_EXPORT
bool fileFetchSummaryInfo( MP4FileHandle file, FileSummaryInfo& info );

///////////////////////////////////////////////////////////////////////////////

}} // namespace mp4v2::util

#endif // MP4V2_UTIL_OTHER_H
