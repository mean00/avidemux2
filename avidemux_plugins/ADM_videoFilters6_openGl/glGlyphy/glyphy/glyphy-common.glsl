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


#ifndef GLYPHY_INFINITY
#  define GLYPHY_INFINITY 1e9
#endif
#ifndef GLYPHY_EPSILON
#  define GLYPHY_EPSILON  1e-5
#endif

#ifndef GLYPHY_RGBA
#  ifdef GLYPHY_BGRA
#    define GLYPHY_RGBA(v) glyphy_bgra (v)
#  else
#    define GLYPHY_RGBA(v) glyphy_rgba (v)
#  endif
#endif

vec4
glyphy_rgba (const vec4 v)
{
  return v.rgba;
}

vec4
glyphy_bgra (const vec4 v)
{
  return v.bgra;
}


struct glyphy_arc_t {
  vec2  p0;
  vec2  p1;
  float d;
};

struct glyphy_arc_endpoint_t {
  /* Second arc endpoint */
  vec2  p;
  /* Infinity if this endpoint does not form an arc with the previous
   * endpoint.  Ie. a "move_to".  Test with glyphy_isinf().
   * Arc depth otherwise.  */
  float d;
};

struct glyphy_arc_list_t {
  /* Number of endpoints in the list.
   * Will be zero if we're far away inside or outside, in which case side is set.
   * Will be -1 if this arc-list encodes a single line, in which case line_* are set. */
  int num_endpoints;

  /* If num_endpoints is zero, this specifies whether we are inside (-1)
   * or outside (+1).  Otherwise we're unsure (0). */
  int side;
  /* Offset to the arc-endpoints from the beginning of the glyph blob */
  int offset;

  /* A single line is all we care about.  It's right here. */
  float line_angle;
  float line_distance; /* From nominal glyph center */
};

bool
glyphy_isinf (const float v)
{
  return abs (v) >= GLYPHY_INFINITY * .5;
}

bool
glyphy_iszero (const float v)
{
  return abs (v) <= GLYPHY_EPSILON * 2.;
}

vec2
glyphy_ortho (const vec2 v)
{
  return vec2 (-v.y, v.x);
}

int
glyphy_float_to_byte (const float v)
{
  return int (v * (256. - GLYPHY_EPSILON));
}

ivec4
glyphy_vec4_to_bytes (const vec4 v)
{
  return ivec4 (v * (256. - GLYPHY_EPSILON));
}

ivec2
glyphy_float_to_two_nimbles (const float v)
{
  int f = glyphy_float_to_byte (v);
  return ivec2 (f / 16, int(mod (float(f), 16.)));
}

/* returns tan (2 * atan (d)) */
float
glyphy_tan2atan (const float d)
{
  return 2. * d / (1. - d * d);
}

glyphy_arc_endpoint_t
glyphy_arc_endpoint_decode (const vec4 v, const ivec2 nominal_size)
{
  vec2 p = (vec2 (glyphy_float_to_two_nimbles (v.a)) + v.gb) / 16.;
  float d = v.r;
  if (d == 0.)
    d = GLYPHY_INFINITY;
  else
#define GLYPHY_MAX_D .5
    d = float(glyphy_float_to_byte (d) - 128) * GLYPHY_MAX_D / 127.;
#undef GLYPHY_MAX_D
  return glyphy_arc_endpoint_t (p * vec2(nominal_size), d);
}

vec2
glyphy_arc_center (const glyphy_arc_t a)
{
  return mix (a.p0, a.p1, .5) +
	 glyphy_ortho (a.p1 - a.p0) / (2. * glyphy_tan2atan (a.d));
}

bool
glyphy_arc_wedge_contains (const glyphy_arc_t a, const vec2 p)
{
  float d2 = glyphy_tan2atan (a.d);
  return dot (p - a.p0, (a.p1 - a.p0) * mat2(1,  d2, -d2, 1)) >= 0. &&
	 dot (p - a.p1, (a.p1 - a.p0) * mat2(1, -d2,  d2, 1)) <= 0.;
}

float
glyphy_arc_wedge_signed_dist_shallow (const glyphy_arc_t a, const vec2 p)
{
  vec2 v = normalize (a.p1 - a.p0);
  float line_d = dot (p - a.p0, glyphy_ortho (v));
  if (a.d == 0.)
    return line_d;

  float d0 = dot ((p - a.p0), v);
  if (d0 < 0.)
    return sign (line_d) * distance (p, a.p0);
  float d1 = dot ((a.p1 - p), v);
  if (d1 < 0.)
    return sign (line_d) * distance (p, a.p1);
  float r = 2. * a.d * (d0 * d1) / (d0 + d1);
  if (r * line_d > 0.)
    return sign (line_d) * min (abs (line_d + r), min (distance (p, a.p0), distance (p, a.p1)));
  return line_d + r;
}

float
glyphy_arc_wedge_signed_dist (const glyphy_arc_t a, const vec2 p)
{
  if (abs (a.d) <= .03)
    return glyphy_arc_wedge_signed_dist_shallow (a, p);
  vec2 c = glyphy_arc_center (a);
  return sign (a.d) * (distance (a.p0, c) - distance (p, c));
}

float
glyphy_arc_extended_dist (const glyphy_arc_t a, const vec2 p)
{
  /* Note: this doesn't handle points inside the wedge. */
  vec2 m = mix (a.p0, a.p1, .5);
  float d2 = glyphy_tan2atan (a.d);
  if (dot (p - m, a.p1 - m) < 0.)
    return dot (p - a.p0, normalize ((a.p1 - a.p0) * mat2(+d2, -1, +1, +d2)));
  else
    return dot (p - a.p1, normalize ((a.p1 - a.p0) * mat2(-d2, -1, +1, -d2)));
}

int
glyphy_arc_list_offset (const vec2 p, const ivec2 nominal_size)
{
  ivec2 cell = ivec2 (clamp (floor (p), vec2 (0.,0.), vec2(nominal_size - 1)));
  return cell.y * nominal_size.x + cell.x;
}

glyphy_arc_list_t
glyphy_arc_list_decode (const vec4 v, const ivec2 nominal_size)
{
  glyphy_arc_list_t l;
  ivec4 iv = glyphy_vec4_to_bytes (v);
  l.side = 0; /* unsure */
  if (iv.r == 0) { /* arc-list encoded */
    l.offset = (iv.g * 256) + iv.b;
    l.num_endpoints = iv.a;
    if (l.num_endpoints == 255) {
      l.num_endpoints = 0;
      l.side = -1;
    } else if (l.num_endpoints == 0)
      l.side = +1;
  } else { /* single line encoded */
    l.num_endpoints = -1;
    l.line_distance = float(((iv.r - 128) * 256 + iv.g) - 0x4000) / float (0x1FFF)
                    * max (float (nominal_size.x), float (nominal_size.y));
    l.line_angle = float(-((iv.b * 256 + iv.a) - 0x8000)) / float (0x7FFF) * 3.14159265358979;
  }
  return l;
}
