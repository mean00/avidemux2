#ifndef DIRENT_INCLUDED
#define DIRENT_INCLUDED

/*

    Declaration of POSIX directory browsing functions and types for Win32.

    Author:  Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
    History: Created March 1997. Updated June 2003.
    Rights:  See end of file.
    
*/

#include <wchar.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _WDIR _WDIR;

struct _wdirent
{
    wchar_t *d_name;
};

_WDIR         *_wopendir(const wchar_t *);
int           _wclosedir(_WDIR *);
struct _wdirent *_wreaddir(_WDIR *);
void          _wrewinddir(_WDIR *);

/*

    Copyright Kevlin Henney, 1997, 2003. All rights reserved.

    Permission to use, copy, modify, and distribute this software and its
    documentation for any purpose is hereby granted without fee, provided
    that this copyright and permissions notice appear in all copies and
    derivatives.
    
    This software is supplied "as is" without express or implied warranty.

    But that said, if there are any problems please get in touch.

*/

#ifdef __cplusplus
}
#endif

#endif
