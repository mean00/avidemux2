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
 * Google Author(s): Behdad Esfahbod, Maysum Panju, Wojciech Baranowski
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "glyphy-common.hh"
#include "glyphy-geometry.hh"
#include "glyphy-arcs-bezier.hh"

using namespace GLyphy::Geometry;
using namespace GLyphy::ArcsBezier;



/*
 * Approximate outlines with multiple arcs
 */


struct glyphy_arc_accumulator_t {
  unsigned int refcount;

  double tolerance;
  double max_d;
  unsigned int d_bits;
  glyphy_arc_endpoint_accumulator_callback_t  callback;
  void                                       *user_data;

  glyphy_point_t start_point;
  glyphy_point_t current_point;
  bool           need_moveto;
  unsigned int   num_endpoints;
  double max_error;
  glyphy_bool_t success;
};


glyphy_arc_accumulator_t *
glyphy_arc_accumulator_create (void)
{
  glyphy_arc_accumulator_t *acc = (glyphy_arc_accumulator_t *) calloc (1, sizeof (glyphy_arc_accumulator_t));
  acc->refcount = 1;

  acc->tolerance = 5e-4;
  acc->max_d = GLYPHY_MAX_D;
  acc->d_bits = 8;
  acc->callback = NULL;
  acc->user_data = NULL;

  glyphy_arc_accumulator_reset (acc);

  return acc;
}

void
glyphy_arc_accumulator_reset (glyphy_arc_accumulator_t *acc)
{
  acc->start_point = acc->current_point = Point (0, 0);
  acc->need_moveto = true;
  acc->num_endpoints = 0;
  acc->max_error = 0;
  acc->success = true;
}

void
glyphy_arc_accumulator_destroy (glyphy_arc_accumulator_t *acc)
{
  if (!acc || --acc->refcount)
    return;

  free (acc);
}

glyphy_arc_accumulator_t *
glyphy_arc_accumulator_reference (glyphy_arc_accumulator_t *acc)
{
  if (acc)
    acc->refcount++;
  return acc;
}


/* Configure acc */

void
glyphy_arc_accumulator_set_tolerance (glyphy_arc_accumulator_t *acc,
				      double                    tolerance)
{
  acc->tolerance = tolerance;
}

double
glyphy_arc_accumulator_get_tolerance (glyphy_arc_accumulator_t *acc)
{
  return acc->tolerance;
}

void
glyphy_arc_accumulator_set_callback (glyphy_arc_accumulator_t *acc,
				     glyphy_arc_endpoint_accumulator_callback_t callback,
				     void                     *user_data)
{
  acc->callback = callback;
  acc->user_data = user_data;
}

void
glyphy_arc_accumulator_get_callback (glyphy_arc_accumulator_t  *acc,
				     glyphy_arc_endpoint_accumulator_callback_t *callback,
				     void                     **user_data)
{
  *callback = acc->callback;
  *user_data = acc->user_data;
}

void
glyphy_arc_accumulator_set_d_metrics (glyphy_arc_accumulator_t *acc,
				      double                    max_d,
				      double                    d_bits)
{
  acc->max_d = max_d;
  acc->d_bits = d_bits;
}

void
glyphy_arc_accumulator_get_d_metrics (glyphy_arc_accumulator_t *acc,
				      double                   *max_d,
				      double                   *d_bits)
{
  *max_d = acc->max_d;
  *d_bits = acc->d_bits;
}


/* Accumulation results */

unsigned int
glyphy_arc_accumulator_get_num_endpoints (glyphy_arc_accumulator_t *acc)
{
  return acc->num_endpoints;
}

double
glyphy_arc_accumulator_get_error (glyphy_arc_accumulator_t *acc)
{
  return acc->max_error;
}

glyphy_bool_t
glyphy_arc_accumulator_successful (glyphy_arc_accumulator_t *acc)
{
  return acc->success;
}


/* Accumulate */

static void
emit (glyphy_arc_accumulator_t *acc, const Point &p, double d)
{
  glyphy_arc_endpoint_t endpoint = {p, d};
  acc->success = acc->success && acc->callback (&endpoint, acc->user_data);
  if (acc->success) {
    acc->num_endpoints++;
    acc->current_point = p;
  }
}

static void
accumulate (glyphy_arc_accumulator_t *acc, const Point &p, double d)
{
  if (Point (acc->current_point) == p)
    return;
  if (d == GLYPHY_INFINITY) {
    /* Emit moveto lazily, for cleaner outlines */
    acc->need_moveto = true;
    acc->current_point = p;
    return;
  }
  if (acc->need_moveto) {
    emit (acc, acc->current_point, GLYPHY_INFINITY);
    if (acc->success) {
      acc->start_point = acc->current_point;
      acc->need_moveto = false;
    }
  }
  emit (acc, p, d);
}

static void
move_to (glyphy_arc_accumulator_t *acc, const Point &p)
{
  if (!acc->num_endpoints || p != acc->current_point)
    accumulate (acc, p, GLYPHY_INFINITY);
}

static void
arc_to (glyphy_arc_accumulator_t *acc, const Point &p1, double d)
{
  accumulate (acc, p1, d);
}

static void
bezier (glyphy_arc_accumulator_t *acc, const Bezier &b)
{
  double e;

  std::vector<Arc> arcs;
  typedef ArcBezierApproximatorQuantizedDefault _ArcBezierApproximator;
  _ArcBezierApproximator appx (acc->max_d, acc->d_bits);
  ArcsBezierApproximatorSpringSystem<_ArcBezierApproximator>
    ::approximate_bezier_with_arcs (b, acc->tolerance, appx, arcs, &e);

  acc->max_error = std::max (acc->max_error, e);

  move_to (acc, b.p0);
  for (unsigned int i = 0; i < arcs.size (); i++)
    arc_to (acc, arcs[i].p1, arcs[i].d);
}

static void
close_path (glyphy_arc_accumulator_t *acc)
{
  if (!acc->need_moveto && Point (acc->current_point) != Point (acc->start_point))
    arc_to (acc, acc->start_point, 0);
}

void
glyphy_arc_accumulator_move_to (glyphy_arc_accumulator_t *acc,
				const glyphy_point_t *p0)
{
  move_to (acc, *p0);
}

void
glyphy_arc_accumulator_line_to (glyphy_arc_accumulator_t *acc,
				const glyphy_point_t *p1)
{
  arc_to (acc, *p1, 0);
}

void
glyphy_arc_accumulator_conic_to (glyphy_arc_accumulator_t *acc,
				 const glyphy_point_t *p1,
				 const glyphy_point_t *p2)
{
  bezier (acc, Bezier (acc->current_point,
		       Point (acc->current_point).lerp (2/3., *p1),
		       Point (*p2).lerp (2/3., *p1),
		       *p2));
}

void
glyphy_arc_accumulator_cubic_to (glyphy_arc_accumulator_t *acc,
				 const glyphy_point_t *p1,
				 const glyphy_point_t *p2,
				 const glyphy_point_t *p3)
{
  bezier (acc, Bezier (acc->current_point, *p1, *p2, *p3));
}

void
glyphy_arc_accumulator_arc_to (glyphy_arc_accumulator_t *acc,
			       const glyphy_point_t *p1,
			       double         d)
{
  arc_to (acc, *p1, d);
}

void
glyphy_arc_accumulator_close_path (glyphy_arc_accumulator_t *acc)
{
  close_path (acc);
}



/*
 * Outline extents from arc list
 */


void
glyphy_arc_list_extents (const glyphy_arc_endpoint_t *endpoints,
			 unsigned int                 num_endpoints,
			 glyphy_extents_t            *extents)
{
  Point p0 (0, 0);
  glyphy_extents_clear (extents);
  for (unsigned int i = 0; i < num_endpoints; i++) {
    const glyphy_arc_endpoint_t &endpoint = endpoints[i];
    if (endpoint.d == GLYPHY_INFINITY) {
      p0 = endpoint.p;
      continue;
    }
    Arc arc (p0, endpoint.p, endpoint.d);
    p0 = endpoint.p;

    glyphy_extents_t arc_extents;
    arc.extents (arc_extents);
    glyphy_extents_extend (extents, &arc_extents);
  }
}
