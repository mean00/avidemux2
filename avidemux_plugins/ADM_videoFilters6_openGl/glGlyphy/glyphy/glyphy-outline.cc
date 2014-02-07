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
 * Google Author(s): Behdad Esfahbod
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "glyphy-common.hh"
#include "glyphy-geometry.hh"

using namespace GLyphy::Geometry;


void
glyphy_outline_reverse (glyphy_arc_endpoint_t *endpoints,
			unsigned int           num_endpoints)
{
  if (!num_endpoints)
    return;

  // Shift the d's first
  double d0 = endpoints[0].d;
  for (unsigned int i = 0; i < num_endpoints - 1; i++)
    endpoints[i].d = endpoints[i + 1].d == GLYPHY_INFINITY ? GLYPHY_INFINITY : -endpoints[i + 1].d;
  endpoints[num_endpoints - 1].d = d0;

  // Reverse
  for (unsigned int i = 0, j = num_endpoints - 1; i < j; i++, j--) {
    glyphy_arc_endpoint_t t = endpoints[i];
    endpoints[i] = endpoints[j];
    endpoints[j] = t;
  }
}


static bool
winding (const glyphy_arc_endpoint_t *endpoints,
	 unsigned int                 num_endpoints)
{
  /*
   * Algorithm:
   *
   * - Find the lowest-x part of the contour,
   * - If the point is an endpoint:
   *   o compare the angle of the incoming and outgoing edges of that point
   *     to find out whether it's CW or CCW,
   * - Otherwise, compare the y of the two endpoints of the arc with lowest-x point.
   *
   * Note:
   *
   * We can use a simpler algorithm here: Act as if arcs are lines, then use the
   * triangle method to calculate the signed area of the contour and get the sign.
   * It should work for all cases we care about.  The only case failing would be
   * that of two endpoints and two arcs.  But we can even special-case that.
   */

  unsigned int corner = 1;
  for (unsigned int i = 2; i < num_endpoints; i++)
    if (endpoints[i].p.x < endpoints[corner].p.x ||
	(endpoints[i].p.x == endpoints[corner].p.x &&
	 endpoints[i].p.y < endpoints[corner].p.y))
      corner = i;

  double min_x = endpoints[corner].p.x;
  int winner = -1;
  Point p0 (0, 0);
  for (unsigned int i = 0; i < num_endpoints; i++) {
    const glyphy_arc_endpoint_t &endpoint = endpoints[i];
    if (endpoint.d == GLYPHY_INFINITY || endpoint.d == 0 /* arcs only, not lines */) {
      p0 = endpoint.p;
      continue;
    }
    Arc arc (p0, endpoint.p, endpoint.d);
    p0 = endpoint.p;

    Point c = arc.center ();
    double r = arc.radius ();
    if (c.x - r < min_x && arc.wedge_contains_point (c - Vector (r, 0))) {
      min_x = c.x - r;
      winner = i;
    }
  }

  if (winner == -1)
  {
    // Corner is lowest-x.  Find the tangents of the two arcs connected to the
    // corner and compare the tangent angles to get contour direction.
    const glyphy_arc_endpoint_t ethis = endpoints[corner];
    const glyphy_arc_endpoint_t eprev = endpoints[corner - 1];
    const glyphy_arc_endpoint_t enext = endpoints[corner < num_endpoints - 1 ? corner + 1 : 1];
    double in  = (-Arc (eprev.p, ethis.p, ethis.d).tangents ().second).angle ();
    double out = (+Arc (ethis.p, enext.p, enext.d).tangents ().first ).angle ();
    return out > in;
  }
  else
  {
    // Easy.
    return endpoints[winner].d < 0;
  }

  return false;
}


static int
categorize (double v, double ref)
{
  return v < ref - GLYPHY_EPSILON ? -1 : v > ref + GLYPHY_EPSILON ? +1 : 0;
}

static bool
is_zero (double v)
{
  return fabs (v) < GLYPHY_EPSILON;
}

static bool
even_odd (const glyphy_arc_endpoint_t *c_endpoints,
	  unsigned int                 num_c_endpoints,
	  const glyphy_arc_endpoint_t *endpoints,
	  unsigned int                 num_endpoints)
{
  /*
   * Algorithm:
   *
   * - For a point on the contour, draw a halfline in a direction
   *   (eg. decreasing x) to infinity,
   * - Count how many times it crosses all other contours,
   * - Pay special attention to points falling exactly on the halfline,
   *   specifically, they count as +.5 or -.5, depending the direction
   *   of crossing.
   *
   * All this counting is extremely tricky:
   *
   * - Floating point equality cannot be relied on here,
   * - Lots of arc analysis needed,
   * - Without having a point that we know falls /inside/ the contour,
   *   there are legitimate cases that we simply cannot handle using
   *   this algorithm.  For example, imagine the following glyph shape:
   *
   *         +---------+
   *         | +-----+ |
   *         |  \   /  |
   *         |   \ /   |
   *         +----o----+
   *
   *   If the glyph is defined as two outlines, and when analysing the
   *   inner outline we happen to pick the point denoted by 'o' for
   *   analysis, there simply is no way to differentiate this case from
   *   the following case:
   *
   *         +---------+
   *         |         |
   *         |         |
   *         |         |
   *         +----o----+
   *             / \
   *            /   \
   *           +-----+
   *
   *   However, in one, the triangle should be filled in, and in the other
   *   filled out.
   *
   *   One way to work around this may be to do the analysis for all endpoints
   *   on the outline and take majority.  But even that can fail in more
   *   extreme yet legitimate cases, such as this one:
   *
   *           +--+--+
   *           | / \ |
   *           |/   \|
   *           +     +
   *           |\   /|
   *           | \ / |
   *           +--o--+
   *
   *   The only correct algorithm I can think of requires a point that falls
   *   fully inside the outline.  While we can try finding such a point (not
   *   dissimilar to the winding algorithm), it's beyond what I'm willing to
   *   implement right now.
   */

  const Point p = c_endpoints[0].p;

  double count = 0;
  Point p0 (0, 0);
  for (unsigned int i = 0; i < num_endpoints; i++) {
    const glyphy_arc_endpoint_t &endpoint = endpoints[i];
    if (endpoint.d == GLYPHY_INFINITY) {
      p0 = endpoint.p;
      continue;
    }
    Arc arc (p0, endpoint.p, endpoint.d);
    p0 = endpoint.p;

    /*
     * Skip our own contour
     */
    if (&endpoint >= c_endpoints && &endpoint < c_endpoints + num_c_endpoints)
      continue;

    /* End-point y's compared to the ref point; lt, eq, or gt */
    unsigned s0 = categorize (arc.p0.y, p.y);
    unsigned s1 = categorize (arc.p1.y, p.y);

    if (is_zero (arc.d))
    {
      /* Line */

      if (!s0 || !s1)
      {
        /*
	 * Add +.5 / -.5 for each endpoint on the halfline, depending on
	 * crossing direction.
	 */
        Pair<Vector> t = arc.tangents ();
        if (!s0 && arc.p0.x < p.x + GLYPHY_EPSILON)
	  count += .5 * categorize (t.first.dy, 0);
        if (!s1 && arc.p1.x < p.x + GLYPHY_EPSILON)
	  count += .5 * categorize (t.second.dy, 0);
	continue;
      }

      if (s0 == s1)
        continue; // Segment fully above or below the halfline

      // Find x pos that the line segment would intersect the half-line.
      double x = arc.p0.x + (arc.p1.x - arc.p0.x) * ((p.y - arc.p0.y) / (arc.p1.y - arc.p0.y));

      if (x >= p.x - GLYPHY_EPSILON)
	continue; // Does not intersect halfline

      count++; // Add one for full crossing
      continue;
    }
    else
    {
      /* Arc */

      if (!s0 || !s1)
      {
        /*
	 * Add +.5 / -.5 for each endpoint on the halfline, depending on
	 * crossing direction.
	 */
        Pair<Vector> t = arc.tangents ();

	/* Arc-specific logic:
	 * If the tangent has dy==0, use the other endpoint's
	 * y value to decide which way the arc will be heading.
	 */
	if (is_zero (t.first.dy))
	  t.first.dy  = +categorize (arc.p1.y, p.y);
	if (is_zero (t.second.dy))
	  t.second.dy = -categorize (arc.p0.y, p.y);

        if (!s0 && arc.p0.x < p.x + GLYPHY_EPSILON)
	  count += .5 * categorize (t.first.dy, 0);
        if (!s1 && arc.p1.x < p.x + GLYPHY_EPSILON)
	  count += .5 * categorize (t.second.dy, 0);
      }

      Point c = arc.center ();
      double r = arc.radius ();
      if (c.x - r >= p.x)
        continue; // No chance
      /* Solve for arc crossing line with y = p.y */
      double dy = p.y - c.y;
      double x2 = r * r - dy * dy;
      if (x2 <= GLYPHY_EPSILON)
        continue; // Negative delta, no crossing
      double dx = sqrt (x2);
      /* There's two candidate points on the arc with the same y as the
       * ref point. */
      Point pp[2] = { Point (c.x - dx, p.y),
		      Point (c.x + dx, p.y) };

#define POINTS_EQ(a,b) (is_zero (a.x - b.x) && is_zero (a.y - b.y))
      for (unsigned int i = 0; i < ARRAY_LENGTH (pp); i++)
      {
        /* Make sure we don't double-count endpoints that fall on the
	 * halfline as we already accounted for those above */
        if (!POINTS_EQ (pp[i], arc.p0) && !POINTS_EQ (pp[i], arc.p1) &&
	    pp[i].x < p.x - GLYPHY_EPSILON && arc.wedge_contains_point (pp[i]))
	  count++; // Add one for full crossing
      }
#undef POINTS_EQ
    }
  }

  return !(int (floor (count)) & 1);
}

static bool
process_contour (glyphy_arc_endpoint_t       *endpoints,
		 unsigned int                 num_endpoints,
		 const glyphy_arc_endpoint_t *all_endpoints,
		 unsigned int                 num_all_endpoints,
		 bool                         inverse)
{
  /*
   * Algorithm:
   *
   * - Find the winding direction and even-odd number,
   * - If the two disagree, reverse the contour, inplace.
   */

  if (!num_endpoints)
    return false;

  if (num_endpoints < 3) {
    abort (); // Don't expect this
    return false; // Need at least two arcs
  }
  if (Point (endpoints[0].p) != Point (endpoints[num_endpoints-1].p)) {
    abort (); // Don't expect this
    return false; // Need a closed contour
   }

  if (inverse ^
      winding (endpoints, num_endpoints) ^
      even_odd (endpoints, num_endpoints, all_endpoints, num_all_endpoints))
  {
    glyphy_outline_reverse (endpoints, num_endpoints);
    return true;
  }

  return false;
}

/* Returns true if outline was modified */
glyphy_bool_t
glyphy_outline_winding_from_even_odd (glyphy_arc_endpoint_t *endpoints,
				      unsigned int           num_endpoints,
				      glyphy_bool_t          inverse)
{
  /*
   * Algorithm:
   *
   * - Process one contour at a time.
   */

  unsigned int start = 0;
  bool ret = false;
  for (unsigned int i = 1; i < num_endpoints; i++) {
    const glyphy_arc_endpoint_t &endpoint = endpoints[i];
    if (endpoint.d == GLYPHY_INFINITY) {
      ret = ret | process_contour (endpoints + start, i - start, endpoints, num_endpoints, bool (inverse));
      start = i;
    }
  }
  ret = ret | process_contour (endpoints + start, num_endpoints - start, endpoints, num_endpoints, bool (inverse));
  return ret;
}
