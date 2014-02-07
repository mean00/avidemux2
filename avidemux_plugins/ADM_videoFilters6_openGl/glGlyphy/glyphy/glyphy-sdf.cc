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

using namespace GLyphy::Geometry;

/*
 * TODO
 * 
 * Sync this with the shader sdf
 */

double
glyphy_sdf_from_arc_list (const glyphy_arc_endpoint_t *endpoints,
			  unsigned int                 num_endpoints,
			  const glyphy_point_t        *p,
			  glyphy_point_t              *closest_p /* may be NULL; TBD not implemented yet */)
{
  Point c = *p;
  Point p0 (0, 0);
  Arc closest_arc (p0, p0, 0);
  double min_dist = GLYPHY_INFINITY;
  int side = 0;
  for (unsigned int i = 0; i < num_endpoints; i++) {
    const glyphy_arc_endpoint_t &endpoint = endpoints[i];
    if (endpoint.d == GLYPHY_INFINITY) {
      p0 = endpoint.p;
      continue;
    }
    Arc arc (p0, endpoint.p, endpoint.d);
    p0 = endpoint.p;

    if (arc.wedge_contains_point (c)) {
      double sdist = arc.distance_to_point (c); /* TODO This distance has the wrong sign.  Fix */
      double udist = fabs (sdist) * (1 - GLYPHY_EPSILON);
      if (udist <= min_dist) {
        min_dist = udist;
	side = sdist >= 0 ? -1 : +1;
      }
    } else {
      double udist = std::min ((arc.p0 - c).len (), (arc.p1 - c).len ());
      if (udist < min_dist) {
        min_dist = udist;
	side = 0; /* unsure */
	closest_arc = arc;
      } else if (side == 0 && udist == min_dist) {
	/* If this new distance is the same as the current minimum,
	 * compare extended distances.  Take the sign from the arc
	 * with larger extended distance. */
	double old_ext_dist = closest_arc.extended_dist (c);
	double new_ext_dist = arc.extended_dist (c);

	double ext_dist = fabs (new_ext_dist) <= fabs (old_ext_dist) ?
			  old_ext_dist : new_ext_dist;

	/* For emboldening and stuff: */
	// min_dist = fabs (ext_dist);
	side = ext_dist >= 0 ? +1 : -1;
      }
    }
  }

  if (side == 0) {
    // Technically speaking this should not happen, but it does.  So try to fix it.
    double ext_dist = closest_arc.extended_dist (c);
    side = ext_dist >= 0 ? +1 : -1;
  }

  return side * min_dist;
}
