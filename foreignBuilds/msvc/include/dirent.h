/*
 * DIRENT.H (formerly DIRLIB.H)
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 */

#ifndef _DIRENT_H_
#define _DIRENT_H_

#define _UNICODE

#include <wchar.h>
#include <tchar.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct _wdirent
{
	long		d_ino;		/* Always zero. */
	unsigned short	d_reclen;	/* Always zero. */
	unsigned short	d_namlen;	/* Length of name in d_name. */
	wchar_t		d_name[260]; /* [FILENAME_MAX] */ /* File name. */
};

typedef struct
{
	struct _wfinddata_t	dd_dta;
	struct _wdirent		dd_dir;
	intptr_t		dd_handle;
	int			dd_stat;
	wchar_t			dd_name[1];
} _WDIR;

#define _tdirent	_wdirent
#define _TDIR       _WDIR
#define _topendir	_wopendir
#define _tclosedir	_wclosedir
#define _treaddir	_wreaddir
#define _trewinddir	_wrewinddir

_TDIR *_topendir (const _TCHAR *szPath);
int _tclosedir (_TDIR * dirp);
struct _tdirent *_treaddir (_TDIR * dirp);
void _trewinddir (_TDIR * dirp);

#ifdef __cplusplus
}
#endif

#endif
