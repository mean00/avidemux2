/*
 *
 * qblockdist_sse.simd.h
 * Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>
 *
 *
 *
 * This file is part of mpeg2enc, a free MPEG-2 video stream encoder
 * based on the original MSSG reference design
 *
 * mpeg2enc is free software; you can redistribute new parts
 * and/or modify under the terms of the GNU General Public License 
 * as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2enc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * See the files for those sections (c) MSSG
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __SIMD_H__
#define __SIMD_H__
#include <config.h>
#include "stdlib.h"
#include "mjpeg_types.h"


/*
 * Interface to SIMD optimiseable low-level routines plus prototypes
 * neutral implementations
 *
 * N.b. linkage and implemenations of such functions is C not C++!
 * N.b. some of these reference implementations may be invoked by
 * architecture specific routines as fall-backs. Hence their inclusion here...
 *
 */
/*  (C) 2003 Andrew Stevens */

/* These modifications are free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */


#include "cpu_accel.h"
#include "quantize_ref.h"
#include "transfrm_ref.h"
#include "predict_ref.h"



#endif /* __SIMD_H__ */
