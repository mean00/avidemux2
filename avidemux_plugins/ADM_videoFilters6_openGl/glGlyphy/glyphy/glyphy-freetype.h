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

/* Intentionally doesn't have include guards */

#include "glyphy.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H



#ifndef GLYPHY_FREETYPE_PREFIX
#define GLYPHY_FREETYPE_PREFIX glyphy_freetype_
#endif

#ifndef glyphy_freetype
#define glyphy_freetype(name) GLYPHY_PASTE (GLYPHY_FREETYPE_PREFIX, name)
#endif



static int
glyphy_freetype(move_to) (FT_Vector *to,
			  glyphy_arc_accumulator_t *acc)
{
  glyphy_point_t p1 = {to->x, to->y};
  glyphy_arc_accumulator_close_path (acc);
  glyphy_arc_accumulator_move_to (acc, &p1);
  return glyphy_arc_accumulator_successful (acc) ? FT_Err_Ok : FT_Err_Out_Of_Memory;
}

static int
glyphy_freetype(line_to) (FT_Vector *to,
			  glyphy_arc_accumulator_t *acc)
{
  glyphy_point_t p1 = {to->x, to->y};
  glyphy_arc_accumulator_line_to (acc, &p1);
  return glyphy_arc_accumulator_successful (acc) ? FT_Err_Ok : FT_Err_Out_Of_Memory;
}

static int
glyphy_freetype(conic_to) (FT_Vector *control, FT_Vector *to,
			   glyphy_arc_accumulator_t *acc)
{
  glyphy_point_t p1 = {control->x, control->y};
  glyphy_point_t p2 = {to->x, to->y};
  glyphy_arc_accumulator_conic_to (acc, &p1, &p2);
  return glyphy_arc_accumulator_successful (acc) ? FT_Err_Ok : FT_Err_Out_Of_Memory;
}

static int
glyphy_freetype(cubic_to) (FT_Vector *control1, FT_Vector *control2, FT_Vector *to,
			   glyphy_arc_accumulator_t *acc)
{
  glyphy_point_t p1 = {control1->x, control1->y};
  glyphy_point_t p2 = {control2->x, control2->y};
  glyphy_point_t p3 = {to->x, to->y};
  glyphy_arc_accumulator_cubic_to (acc, &p1, &p2, &p3);
  return glyphy_arc_accumulator_successful (acc) ? FT_Err_Ok : FT_Err_Out_Of_Memory;
}

static FT_Error
glyphy_freetype(outline_decompose) (const FT_Outline         *outline,
				    glyphy_arc_accumulator_t *acc)
{
  const FT_Outline_Funcs outline_funcs = {
    (FT_Outline_MoveToFunc) glyphy_freetype(move_to),
    (FT_Outline_LineToFunc) glyphy_freetype(line_to),
    (FT_Outline_ConicToFunc) glyphy_freetype(conic_to),
    (FT_Outline_CubicToFunc) glyphy_freetype(cubic_to),
    0, /* shift */
    0, /* delta */
  };

  return FT_Outline_Decompose ((FT_Outline *) outline, &outline_funcs, acc);
}
