/*
 * Buffered file io for ffmpeg system
 * Copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/avstring.h"
#include "avformat.h"
#include <fcntl.h>
#if HAVE_SETMODE
#include <io.h>
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "os_support.h"

// GRUNTSTER start
#ifdef __WIN32
#include <windows.h>

int utf8StringToWideChar(const char *utf8String, int utf8StringLength, wchar_t *wideCharString)
{
	int wideCharStringLength = MultiByteToWideChar(CP_UTF8, 0, utf8String, utf8StringLength, NULL, 0);

	if (wideCharString)
		MultiByteToWideChar(CP_UTF8, 0, utf8String, utf8StringLength, wideCharString, wideCharStringLength);

	return wideCharStringLength;
}

int ADM_open(const char *path, int oflag, ...)
{
	int fileNameLength = utf8StringToWideChar(path, -1, NULL);
	wchar_t wcFile[fileNameLength];
	int creation = 0, access = 0;
	HANDLE hFile;

	utf8StringToWideChar(path, -1, wcFile);

	if (oflag & O_WRONLY || oflag & O_RDWR)
	{
		access = GENERIC_WRITE;

		if (oflag & O_RDWR)
			access |= GENERIC_READ;

		if (oflag & O_CREAT)
		{
			if (oflag & O_EXCL)
				creation = CREATE_NEW;
			else if (oflag & O_TRUNC)
				creation = CREATE_ALWAYS;
			else
				creation = OPEN_ALWAYS;
		}
		else if (oflag & O_TRUNC)
			creation = TRUNCATE_EXISTING;
	}
	else if (oflag & O_RDONLY)
		creation = OPEN_EXISTING;

	if (creation & GENERIC_WRITE)
	{
		hFile = CreateFileW(wcFile, access, 0, NULL, creation, 0, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return -1;
		else
			CloseHandle(hFile);
	}

	hFile = CreateFileW(wcFile, access, FILE_SHARE_READ, NULL, creation, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return -1;
	else
		return _open_osfhandle((intptr_t)hFile, oflag);
}

#define open ADM_open
#endif
// GRUNTSTER end

/* standard file protocol */

static int file_read(URLContext *h, unsigned char *buf, int size)
{
    int fd = (intptr_t) h->priv_data;
    return read(fd, buf, size);
}

static int file_write(URLContext *h, const unsigned char *buf, int size)
{
    int fd = (intptr_t) h->priv_data;
    return write(fd, buf, size);
}

static int file_get_handle(URLContext *h)
{
    return (intptr_t) h->priv_data;
}

#if CONFIG_FILE_PROTOCOL

static int file_open(URLContext *h, const char *filename, int flags)
{
    int access;
    int fd;

    av_strstart(filename, "file:", &filename);

    if (flags & URL_RDWR) {
        access = O_CREAT | O_TRUNC | O_RDWR;
    } else if (flags & URL_WRONLY) {
        access = O_CREAT | O_TRUNC | O_WRONLY;
    } else {
        access = O_RDONLY;
    }
#ifdef O_BINARY
    access |= O_BINARY;
#endif
    fd = open(filename, access, 0666);
    if (fd == -1)
        return AVERROR(errno);
    h->priv_data = (void *) (intptr_t) fd;
    return 0;
}

/* XXX: use llseek */
static int64_t file_seek(URLContext *h, int64_t pos, int whence)
{
    int fd = (intptr_t) h->priv_data;
    if (whence == AVSEEK_SIZE) {
        struct stat st;
        int ret = fstat(fd, &st);
        return ret < 0 ? AVERROR(errno) : st.st_size;
    }
    return lseek(fd, pos, whence);
}

static int file_close(URLContext *h)
{
    int fd = (intptr_t) h->priv_data;
    return close(fd);
}

URLProtocol file_protocol = {
    "file",
    file_open,
    file_read,
    file_write,
    file_seek,
    file_close,
    .url_get_file_handle = file_get_handle,
};

#endif /* CONFIG_FILE_PROTOCOL */

#if CONFIG_PIPE_PROTOCOL

static int pipe_open(URLContext *h, const char *filename, int flags)
{
    int fd;
    char *final;
    av_strstart(filename, "pipe:", &filename);

    fd = strtol(filename, &final, 10);
    if((filename == final) || *final ) {/* No digits found, or something like 10ab */
        if (flags & URL_WRONLY) {
            fd = 1;
        } else {
            fd = 0;
        }
    }
#if HAVE_SETMODE
    setmode(fd, O_BINARY);
#endif
    h->priv_data = (void *) (intptr_t) fd;
    h->is_streamed = 1;
    return 0;
}

URLProtocol pipe_protocol = {
    "pipe",
    pipe_open,
    file_read,
    file_write,
    .url_get_file_handle = file_get_handle,
};

#endif /* CONFIG_PIPE_PROTOCOL */
