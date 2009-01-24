/* amber.c, this file is part of the
 * AltiVec optimized library for MJPEG tools MPEG-1/2 Video Encoder
 * Copyright (C) 2002  James Klicman <james@klicman.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <sys/errno.h>
#include <sys/stat.h>
#include <dirent.h>

#include "../mjpeg_logging.h"

#define SIGNAL_AMBER() \
    ({register long _value; \
	asm volatile ("mfspr %0, 1023" : "=r" (_value) ); \
	_value; })


long amber_start(const char *file, int line, int *trace_count, int max_traces)
{
    long value;

    value = 0;

    if (*trace_count < max_traces) {

	mjpeg_info("amber start %s:%d", file, line);

	value = SIGNAL_AMBER();
    }

    return value;
}

long amber_stop(const char *file, int line, int *trace_count, int max_traces,
    int max_exit)
{
    long value;

    if (*trace_count < max_traces) {
	value = SIGNAL_AMBER();

	mjpeg_info("amber stop %s:%d", file, line);

	(*trace_count)++;
	if (max_exit && *trace_count >= max_traces)
	    exit(0);
    } else {
	value = 0;
    }

    return value;
}


void amber_symtrace(const char *name) {
    DIR *dp;
    struct dirent *dir;
    struct stat sb;
    char tracename[PATH_MAX], sympath[PATH_MAX], symname[PATH_MAX];
    int found = 0;

    if ((dp = opendir(".")) == NULL)
	mjpeg_error_exit1("can't open current directory");

    while ((dir = readdir(dp)) != NULL) {
	if (dir->d_ino == 0)
	    continue;

	if (strncmp(dir->d_name, "trace_", 6) == 0) {
	    if (!found) {
		found = 1;
		strncpy(tracename, dir->d_name, PATH_MAX);
	    } else {
		if (strncmp(dir->d_name, tracename, PATH_MAX) > 0)
		    strncpy(tracename, dir->d_name, PATH_MAX);
	    }
	}
    }
    closedir(dp);

    if (found) {
	if (stat(name, &sb) != 0 && errno == ENOENT) {
	    if (mkdir(name, 0777) != 0)
		mjpeg_error_exit1("can't make directory %s", name);
	}
	snprintf(sympath, PATH_MAX, "../%s", tracename);
	snprintf(symname, PATH_MAX, "%s/%s", name, tracename);
	if (symlink(sympath, symname) != 0)
	    mjpeg_error_exit1("can't symlink %s to %s", tracename, symname);
    } else {
	mjpeg_error_exit1("trace not found");
    }
}
