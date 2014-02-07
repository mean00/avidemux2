/*
 * Copyright 2012 Google, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Google Author(s): Behdad Esfahbod, Maysum Panju
 */

#ifndef GLYPHY_COMMON_HH
#define GLYPHY_COMMON_HH

#include <glyphy.h>

#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

#ifndef GLYPHY_EPSILON
#  define GLYPHY_EPSILON  1e-5
#endif
#ifndef GLYPHY_INFINITY
#  define GLYPHY_INFINITY INFINITY
#endif


static inline bool
iszero (double v)
{
  return fabs (v) < 2 * GLYPHY_EPSILON;
}


#define GLYPHY_MAX_D .5

#undef  ARRAY_LENGTH
#define ARRAY_LENGTH(__array) ((signed int) (sizeof (__array) / sizeof (__array[0])))

#define _ASSERT_STATIC1(_line, _cond) typedef int _static_assert_on_line_##_line##_failed[(_cond)?1:-1]
#define _ASSERT_STATIC0(_line, _cond) _ASSERT_STATIC1 (_line, (_cond))
#define ASSERT_STATIC(_cond) _ASSERT_STATIC0 (__LINE__, (_cond))

#ifdef __ANDROID__
#define log2(x) (log(x) / log(2.0))
#endif

#endif /* GLYPHY_COMMON_HH */
