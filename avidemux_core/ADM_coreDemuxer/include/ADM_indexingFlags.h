/***************************************************************************
 *   Definitions used to store indexing settings for various demuxers      *
 *                     in a single uint32_t value                          *
 *   Copyright (C) 2026 eumagga0x2a                                        *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_IDX_FLAGS_H
#define ADM_IDX_FLAGS_H

// Allow a particular demuxer to create index files on disk.
// If unset, use existing ones if possible, else create index in-memory.
#define ADM_IDX_FLAG_WRITE_INDEX_FILE      1
// Create index in-memory, even if a matching valid index file on disk exists.
#define ADM_IDX_FLAG_IGNORE_INDEX_FILE    (1<<1)

// Each demuxer has 4 bits in a single uint32_t preference.
#define ADM_IDX_FLAGS_OFFSET_MPEGPS    0
#define ADM_IDX_FLAGS_OFFSET_MPEGTS    4
#define ADM_IDX_FLAGS_OFFSET_MATROSKA  8
#define ADM_IDX_FLAGS_OFFSET_MP4      12

// By default, create index files on disk.
#define ADM_IDX_FLAGS_DEFAULT \
((ADM_IDX_FLAG_WRITE_INDEX_FILE << ADM_IDX_FLAGS_OFFSET_MPEGPS) + \
 (ADM_IDX_FLAG_WRITE_INDEX_FILE << ADM_IDX_FLAGS_OFFSET_MPEGTS) + \
 (ADM_IDX_FLAG_WRITE_INDEX_FILE << ADM_IDX_FLAGS_OFFSET_MATROSKA) + \
 (ADM_IDX_FLAG_WRITE_INDEX_FILE << ADM_IDX_FLAGS_OFFSET_MP4))

#endif
