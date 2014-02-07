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

#define GRID_SIZE 24

using namespace GLyphy::Geometry;


#define UPPER_BITS(v,bits,total_bits) ((v) >> ((total_bits) - (bits)))
#define LOWER_BITS(v,bits,total_bits) ((v) & ((1 << (bits)) - 1))

#define MAX_X 4095
#define MAX_Y 4095

static inline glyphy_rgba_t
arc_endpoint_encode (unsigned int ix, unsigned int iy, double d)
{
  glyphy_rgba_t v;

  /* 12 bits for each of x and y, 8 bits for d */
  assert (ix <= MAX_X);
  assert (iy <= MAX_Y);
  unsigned int id;
  if (isinf (d))
    id = 0;
  else {
    assert (fabs (d) <= GLYPHY_MAX_D);
    id = 128 + lround (d * 127 / GLYPHY_MAX_D);
  }
  assert (id < 256);

  v.r = id;
  v.g = LOWER_BITS (ix, 8, 12);
  v.b = LOWER_BITS (iy, 8, 12);
  v.a = ((ix >> 8) << 4) | (iy >> 8);
  return v;
}

static inline glyphy_rgba_t
arc_list_encode (unsigned int offset, unsigned int num_points, int side)
{
  glyphy_rgba_t v;
  v.r = 0; // unused for arc-list encoding
  v.g = UPPER_BITS (offset, 8, 16);
  v.b = LOWER_BITS (offset, 8, 16);
  v.a = LOWER_BITS (num_points, 8, 8);
  if (side < 0 && !num_points)
    v.a = 255;
  return v;
}

static inline glyphy_rgba_t
line_encode (const Line &line)
{
  Line l = line.normalized ();
  double angle = l.n.angle ();
  double distance = l.c;

  int ia = lround (-angle / M_PI * 0x7FFF);
  unsigned int ua = ia + 0x8000;
  assert (0 == (ua & ~0xFFFF));

  int id = lround (distance * 0x1FFF);
  unsigned int ud = id + 0x4000;
  assert (0 == (ud & ~0x7FFF));

  /* Marker for line-encoded */
  ud |= 0x8000;

  glyphy_rgba_t v;
  v.r = ud >> 8;
  v.g = ud & 0xFF;
  v.b = ua >> 8;
  v.a = ua & 0xFF;
  return v;
}


/* Given a cell, fills the vector closest_arcs with arcs that may be closest to some point in the cell.
 * Uses idea that all close arcs to cell must be ~close to center of cell.
 */
static void
closest_arcs_to_cell (Point c0, Point c1, /* corners */
		      double faraway,
		      const glyphy_arc_endpoint_t *endpoints,
		      unsigned int num_endpoints,
		      std::vector<glyphy_arc_endpoint_t> &near_endpoints,
		      int *side)
{
  // Find distance between cell center
  Point c = c0.midpoint (c1);
  double min_dist = glyphy_sdf_from_arc_list (endpoints, num_endpoints, &c, NULL);

  *side = min_dist >= 0 ? +1 : -1;
  min_dist = fabs (min_dist);
  std::vector<Arc> near_arcs;

  // If d is the distance from the center of the square to the nearest arc, then
  // all nearest arcs to the square must be at most almost [d + half_diagonal] from the center.
  double half_diagonal = (c - c0).len ();
  double radius_squared = pow (min_dist + half_diagonal, 2);
  if (min_dist - half_diagonal <= faraway) {
    Point p0 (0, 0);
    for (unsigned int i = 0; i < num_endpoints; i++) {
      const glyphy_arc_endpoint_t &endpoint = endpoints[i];
      if (endpoint.d == GLYPHY_INFINITY) {
	p0 = endpoint.p;
	continue;
      }
      Arc arc (p0, endpoint.p, endpoint.d);
      p0 = endpoint.p;

      if (arc.squared_distance_to_point (c) <= radius_squared)
        near_arcs.push_back (arc);
    }
  }

  Point p1 = Point (0, 0);
  for (unsigned i = 0; i < near_arcs.size (); i++)
  {
    Arc arc = near_arcs[i];

    if (i == 0 || p1 != arc.p0) {
      glyphy_arc_endpoint_t endpoint = {arc.p0, GLYPHY_INFINITY};
      near_endpoints.push_back (endpoint);
      p1 = arc.p0;
    }

    glyphy_arc_endpoint_t endpoint = {arc.p1, arc.d};
    near_endpoints.push_back (endpoint);
    p1 = arc.p1;
  }
}


glyphy_bool_t
glyphy_arc_list_encode_blob (const glyphy_arc_endpoint_t *endpoints,
			     unsigned int                 num_endpoints,
			     glyphy_rgba_t               *blob,
			     unsigned int                 blob_size,
			     double                       faraway,
			     double                       avg_fetch_desired,
			     double                      *avg_fetch_achieved,
			     unsigned int                *output_len,
			     unsigned int                *nominal_width,  /* 8bit */
			     unsigned int                *nominal_height, /* 8bit */
			     glyphy_extents_t            *pextents)
{
  glyphy_extents_t extents;
  glyphy_extents_clear (&extents);

  glyphy_arc_list_extents (endpoints, num_endpoints, &extents);

  if (glyphy_extents_is_empty (&extents)) {
    *pextents = extents;
    if (!blob_size)
      return false;
    *blob = arc_list_encode (0, 0, +1);
    *avg_fetch_achieved = 1;
    *output_len = 1;
    *nominal_width = *nominal_height = 1;
    return true;
  }

  /* Add antialiasing padding */
  extents.min_x -= faraway;
  extents.min_y -= faraway;
  extents.max_x += faraway;
  extents.max_y += faraway;

  double glyph_width = extents.max_x - extents.min_x;
  double glyph_height = extents.max_y - extents.min_y;
  double unit = std::max (glyph_width, glyph_height);

  unsigned int grid_w = GRID_SIZE;
  unsigned int grid_h = GRID_SIZE;

  if (glyph_width > glyph_height) {
    while ((grid_h - 1) * unit / grid_w > glyph_height)
      grid_h--;
    glyph_height = grid_h * unit / grid_w;
    extents.max_y = extents.min_y + glyph_height;
  } else {
    while ((grid_w - 1) * unit / grid_h > glyph_width)
      grid_w--;
    glyph_width = grid_w * unit / grid_h;
    extents.max_x = extents.min_x + glyph_width;
  }

  double cell_unit = unit / std::max (grid_w, grid_h);

  std::vector<glyphy_rgba_t> tex_data;
  std::vector<glyphy_arc_endpoint_t> near_endpoints;

  unsigned int header_length = grid_w * grid_h;
  unsigned int offset = header_length;
  tex_data.resize (header_length);
  Point origin = Point (extents.min_x, extents.min_y);
  unsigned int total_arcs = 0;

  for (unsigned int row = 0; row < grid_h; row++)
    for (unsigned int col = 0; col < grid_w; col++)
    {
      Point cp0 = origin + Vector ((col + 0) * cell_unit, (row + 0) * cell_unit);
      Point cp1 = origin + Vector ((col + 1) * cell_unit, (row + 1) * cell_unit);
      near_endpoints.clear ();

      int side;
      closest_arcs_to_cell (cp0, cp1,
			    faraway,
			    endpoints, num_endpoints,
			    near_endpoints,
			    &side);

#define QUANTIZE_X(X) (lround (MAX_X * ((X - extents.min_x) / glyph_width )))
#define QUANTIZE_Y(Y) (lround (MAX_Y * ((Y - extents.min_y) / glyph_height)))
#define DEQUANTIZE_X(X) (double (X) / MAX_X * glyph_width  + extents.min_x)
#define DEQUANTIZE_Y(Y) (double (Y) / MAX_Y * glyph_height + extents.min_y)
#define SNAP(P) (Point (DEQUANTIZE_X (QUANTIZE_X ((P).x)), DEQUANTIZE_Y (QUANTIZE_Y ((P).y))))

      if (near_endpoints.size () == 2 && near_endpoints[1].d == 0) {
        Point c (extents.min_x + glyph_width * .5, extents.min_y + glyph_height * .5);
        Line line (SNAP (near_endpoints[0].p), SNAP (near_endpoints[1].p));
	line.c -= line.n * Vector (c);
	line.c /= unit;
	tex_data[row * grid_w + col] = line_encode (line);
	continue;
      }

      /* If the arclist is two arcs that can be combined in encoding if reordered,
       * do that. */
      if (near_endpoints.size () == 4 &&
	  isinf (near_endpoints[2].d) &&
	  near_endpoints[0].p.x == near_endpoints[3].p.x &&
	  near_endpoints[0].p.y == near_endpoints[3].p.y)
      {
        glyphy_arc_endpoint_t e0, e1, e2;
	e0 = near_endpoints[2];
	e1 = near_endpoints[3];
	e2 = near_endpoints[1];
	near_endpoints.resize (0);
	near_endpoints.push_back (e0);
	near_endpoints.push_back (e1);
	near_endpoints.push_back (e2);
      }

      for (unsigned i = 0; i < near_endpoints.size (); i++) {
        glyphy_arc_endpoint_t &endpoint = near_endpoints[i];
	tex_data.push_back (arc_endpoint_encode (QUANTIZE_X(endpoint.p.x), QUANTIZE_Y(endpoint.p.y), endpoint.d));
      }

      unsigned int current_endpoints = tex_data.size () - offset;

      /* See if we can fulfill this cell by using already-encoded arcs */
      const glyphy_rgba_t *needle = &tex_data[offset];
      unsigned int needle_len = current_endpoints;
      const glyphy_rgba_t *haystack = &tex_data[header_length];
      unsigned int haystack_len = offset - header_length;

      bool found = false;
      if (needle_len)
	while (haystack_len >= needle_len) {
	  /* Trick: we don't care about first endpoint's d value, so skip one
	   * byte in comparison.  This works because arc_encode() packs the
	   * d value in the first byte. */
	  if (0 == memcmp (1 + (const char *) needle,
			   1 + (const char *) haystack,
			   needle_len * sizeof (*needle) - 1)) {
	    found = true;
	    break;
	  }
	  haystack++;
	  haystack_len--;
	}
      if (found) {
	tex_data.resize (offset);
	offset = haystack - &tex_data[0];
      }

      tex_data[row * grid_w + col] = arc_list_encode (offset, current_endpoints, side);
      offset = tex_data.size ();

      total_arcs += current_endpoints;
    }

  if (avg_fetch_achieved)
    *avg_fetch_achieved = 1 + double (total_arcs) / (grid_w * grid_h);

  *pextents = extents;

  if (tex_data.size () > blob_size)
    return false;

  memcpy (blob, &tex_data[0], tex_data.size () * sizeof(tex_data[0]));
  *output_len = tex_data.size ();
  *nominal_width = grid_w;
  *nominal_height = grid_h;

  return true;
}
