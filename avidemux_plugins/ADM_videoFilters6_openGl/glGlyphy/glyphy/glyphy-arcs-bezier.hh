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

#ifndef GLYPHY_ARCS_BEZIER_HH
#define GLYPHY_ARCS_BEZIER_HH

#include "glyphy-common.hh"
#include "glyphy-geometry.hh"
#include "glyphy-arc-bezier.hh"

namespace GLyphy {
namespace ArcsBezier {

using namespace Geometry;
using namespace ArcBezier;

template <class ArcBezierApproximator>
class ArcsBezierApproximatorSpringSystem
{
  static inline void calc_arcs (const Bezier &b,
				const std::vector<double> &t,
				const ArcBezierApproximator &appx,
				std::vector<double> &e,
				std::vector<Arc > &arcs,
				double &max_e, double &min_e)
  {
    unsigned int n = t.size () - 1;
    e.resize (n);
    arcs.clear ();
    max_e = 0;
    min_e = GLYPHY_INFINITY;
    for (unsigned int i = 0; i < n; i++)
    {
      Bezier segment = b.segment (t[i], t[i + 1]);
      arcs.push_back (appx.approximate_bezier_with_arc (segment, &e[i]));

      max_e = std::max (max_e, e[i]);
      min_e = std::min (min_e, e[i]);
    }
  }

  static inline void jiggle (const Bezier &b,
			     const ArcBezierApproximator &appx,
			     std::vector<double> &t,
			     std::vector<double> &e,
			     std::vector<Arc > &arcs,
			     double &max_e, double &min_e,
			     double tolerance,
			     unsigned int &n_jiggle)
  {
    unsigned int n = t.size () - 1;
    //fprintf (stderr, "candidate n %d max_e %g min_e %g\n", n, max_e, min_e);
    unsigned int max_jiggle = log2 (n) + 1;
    unsigned int s;
    for (s = 0; s < max_jiggle; s++)
    {
      double total = 0;
      for (unsigned int i = 0; i < n; i++) {
	double l = t[i + 1] - t[i];
	double k_inv = l * pow (e[i], -.3);
	total += k_inv;
	e[i] = k_inv;
      }
      for (unsigned int i = 0; i < n; i++) {
	double k_inv = e[i];
	double l = k_inv / total;
	t[i + 1] = t[i] + l;
      }
      t[n] = 1.0; // Do this to get real 1.0, not .9999999999999998!

      calc_arcs (b, t, appx, e, arcs, max_e, min_e);

      //fprintf (stderr, "n %d jiggle %d max_e %g min_e %g\n", n, s, max_e, min_e);

      n_jiggle++;
      if (max_e < tolerance || (2 * min_e - max_e > tolerance))
	break;
    }
    //if (s == max_jiggle) fprintf (stderr, "JIGGLE OVERFLOW n %d s %d\n", n, s);
  }

  public:
  static void approximate_bezier_with_arcs (const Bezier &b,
					    double tolerance,
					    const ArcBezierApproximator &appx,
					    std::vector<Arc> &arcs,
					    double *perror,
					    unsigned int max_segments = 100)
  {
    std::vector<double> t;
    std::vector<double> e;
    double max_e, min_e;
    unsigned int n_jiggle = 0;

    /* Technically speaking we can bsearch for n. */
    for (unsigned int n = 1; n <= max_segments; n++)
    {
      t.resize (n + 1);
      for (unsigned int i = 0; i < n; i++)
        t[i] = double (i) / n;
      t[n] = 1.0; // Do this out of the loop to get real 1.0, not .9999999999999998!

      calc_arcs (b, t, appx, e, arcs, max_e, min_e);

      for (unsigned int i = 0; i < n; i++)
	if (e[i] <= tolerance) {
	  jiggle (b, appx, t, e, arcs, max_e, min_e, tolerance, n_jiggle);
	  break;
	}

      if (max_e <= tolerance)
        break;
    }
    if (perror)
      *perror = max_e;
    //fprintf (stderr, "n_jiggle %d\n", n_jiggle);
  }
};

} /* namespace ArcsBezier */
} /* namespace GLyphy */

#endif /* GLYPHY_ARCS_BEZIER_HH */
