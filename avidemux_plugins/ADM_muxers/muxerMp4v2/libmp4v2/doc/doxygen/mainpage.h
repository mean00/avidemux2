/** @mainpage MP4v2 API Reference

@section api Public API

The public API is documented in the <a href="modules.html">modules</a>
section (as seen at the top of this page).
Other documentation available for C++ namespaces and classes are for
internal documentation purposes and not to be used or depended-upon by
code external to this project.

@section introducation Introduction

The MP4v2 library provides an API to create and modify mp4 files as defined
by ISO base media file format, document number ISO/IEC 14496-12:2005(E). It
is a very powerful and extensible format that can accomodate practically
any type of media.

The conceptual structure of an mp4 file is that it is a container for one
or more tracks of media. Each track represents exactly one type of media
such as audio or video. Each track is composed of high-level structural
data which describes the raw media data stream in a manner that is as
generic as possible. Tracks have their own timeline, properties and
samples. An example of a sample is a frame of video. Thus, the file
file describes how to synchronize the timeline of the tracks.

In a self-contained file, the samples accounts for the majority of file
size. For playback performance reasons, when an mp4 file contains multiple
media tracks their samples (raw media data) are usually interleaved to
provide optimal playback performance.

@section invocation Invocation

The mp4 library is focussed on providing an easy to use API for the mp4
file format. It has been used with an encoder, a server, a player, and a
number of mp4 utilities. However, it may not be adequate for multimedia
editors that wish to work directly with mp4 files. It may be used by these
type of tools to export an mp4 file.

In providing a easy to use API not all the information in the mp4 file is
directly exposed via the API. To accomodate applications that need access
to information not otherwise available via the API there are file and
track level generic get and set property routines that use arbitary string
property names. To use these routines you will need to be familar with the
mp4 file specification or be willing to wade through the output of a
file-dump which may be produced using MP4Dump() or with some of the
command-line tools bundled with MP4v2.

@section headers Public Headers

The public libary API is defined in @c <mp4v2/mp4v2.h> which includes all the
necessary dependent header files. <b>You must never use any other header
files</b> for public API. Using other header files or symbols which are
not exported via the public API is expressly not supported and may change
at any time without notice.

The MP4v2 library can be used by either C or C++ programs. The calling
convention is C but for convenience if C++ is used default function arguments
have been added where appropriate.

@example example/provider/provider.c
@example example/itmf/generic.c
@example example/itmf/tags.c
*/
