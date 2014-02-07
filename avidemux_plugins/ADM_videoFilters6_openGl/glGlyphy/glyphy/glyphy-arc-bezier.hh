/*
 * Copyright 2012,2013 Google, Inc. All Rights Reserved.
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

#ifndef GLYPHY_ARC_BEZIER_HH
#define GLYPHY_ARC_BEZIER_HH

#include "glyphy-common.hh"
#include "glyphy-geometry.hh"

namespace GLyphy {
namespace ArcBezier {

using namespace Geometry;


class MaxDeviationApproximatorExact
{
  public:
  /* Returns 3 max(abs(d₀ t (1-t)² + d₁ t² (1-t)) for 0≤t≤1. */
  static double approximate_deviation (double d0, double d1)
  {
    double candidates[4] = {0,1};
    unsigned int num_candidates = 2;
    if (d0 == d1)
      candidates[num_candidates++] = .5;
    else {
      double delta = d0*d0 - d0*d1 + d1*d1;
      double t2 = 1. / (3 * (d0 - d1));
      double t0 = (2 * d0 - d1) * t2;
      if (delta == 0)
	candidates[num_candidates++] = t0;
      else if (delta > 0) {
	/* This code can be optimized to avoid the sqrt if the solution
	 * is not feasible (ie. lies outside (0,1)).  I have implemented
	 * that in cairo-spline.c:_cairo_spline_bound().  Can be reused
	 * here.
	 */
	double t1 = sqrt (delta) * t2;
	candidates[num_candidates++] = t0 - t1;
	candidates[num_candidates++] = t0 + t1;
      }
    }

    double e = 0;
    for (unsigned int i = 0; i < num_candidates; i++) {
      double t = candidates[i];
      double ee;
      if (t < 0. || t > 1.)
	continue;
      ee = fabs (3 * t * (1-t) * (d0 * (1 - t) + d1 * t));
      e = std::max (e, ee);
    }

    return e;
  }
};



template <class MaxDeviationApproximator>
class ArcBezierErrorApproximatorBehdad
{
  public:
  static double approximate_bezier_arc_error (const Bezier &b0, const Arc &a)
  {
    assert (b0.p0 == a.p0);
    assert (b0.p3 == a.p1);

    double ea;
    Bezier b1 = a.approximate_bezier (&ea);

    assert (b0.p0 == b1.p0);
    assert (b0.p3 == b1.p3);

    Vector v0 = b1.p1 - b0.p1;
    Vector v1 = b1.p2 - b0.p2;

    Vector b = (b0.p3 - b0.p0).normalized ();
    v0 = v0.rebase (b);
    v1 = v1.rebase (b);

    Vector v (MaxDeviationApproximator::approximate_deviation (v0.dx, v1.dx),
	      MaxDeviationApproximator::approximate_deviation (v0.dy, v1.dy));

    /* Edge cases: If d*d is too close too large default to a weak bound. */
    if (a.d * a.d > 1. - 1e-4)
      return ea + v.len ();

    /* If the wedge doesn't contain control points, default to weak bound. */
    if (!a.wedge_contains_point (b0.p1) || !a.wedge_contains_point (b0.p2))
      return ea + v.len ();

    /* If straight line, return the max ortho deviation. */
    if (fabs (a.d) < 1e-6)
      return ea + v.dy;

    /* We made sure that fabs(a.d) < 1 */
    double tan_half_alpha = fabs (tan2atan (a.d));

    double tan_v = v.dx / v.dy;

    double eb;
    if (fabs (tan_v) <= tan_half_alpha)
      return ea + v.len ();

    double c2 = (a.p1 - a.p0).len () * .5;
    double r = a.radius ();

    eb = Vector (c2 + v.dx, c2 / tan_half_alpha + v.dy).len () - r;
    assert (eb >= 0);

    return ea + eb;
  }
};



template <class ArcBezierErrorApproximator>
class ArcBezierApproximatorMidpointSimple
{
  public:
  static const Arc approximate_bezier_with_arc (const Bezier &b, double *error)
  {
    Arc a (b.p0, b.p3, b.midpoint (), false);

    *error = ArcBezierErrorApproximator::approximate_bezier_arc_error (b, a);

    return a;
  }
};

template <class ArcBezierErrorApproximator>
class ArcBezierApproximatorMidpointTwoPart
{
  public:
  static const Arc approximate_bezier_with_arc (const Bezier &b, double *error, double mid_t = .5)
  {
    Pair<Bezier > pair = b.split (mid_t);
    Point m = pair.second.p0;

    Arc a0 (b.p0, m, b.p3, true);
    Arc a1 (m, b.p3, b.p0, true);

    double e0 = ArcBezierErrorApproximator::approximate_bezier_arc_error (pair.first, a0);
    double e1 = ArcBezierErrorApproximator::approximate_bezier_arc_error (pair.second, a1);
    *error = std::max (e0, e1);

    return Arc (b.p0, b.p3, m, false);
  }
};

template <class ArcBezierErrorApproximator>
class ArcBezierApproximatorQuantized
{
  public:
  ArcBezierApproximatorQuantized (double _max_d = GLYPHY_INFINITY, unsigned int _d_bits = 0) :
    max_d (_max_d), d_bits (_d_bits) {};

  protected:
  double max_d;
  unsigned int d_bits;

  public:
  const Arc approximate_bezier_with_arc (const Bezier &b, double *error) const
  {
    double mid_t = .5;
    Arc a (b.p0, b.p3, b.point (mid_t), false);
    Arc orig_a = a;

    if (isfinite (max_d)) {
      assert (max_d >= 0);
      if (fabs (a.d) > max_d)
        a.d = a.d < 0 ? -max_d : max_d;
    }
    if (d_bits && max_d != 0) {
      assert (isfinite (max_d));
      assert (fabs (a.d) <= max_d);
      int mult = (1 << (d_bits - 1)) - 1;
      int id = round (a.d / max_d * mult);
      assert (-mult <= id && id <= mult);
      a.d = id * max_d / mult;
      assert (fabs (a.d) <= max_d);
    }

    /* Error introduced by arc quantization */
    double ed = fabs (a.d - orig_a.d) * (a.p1 - a.p0).len () * .5;

    ArcBezierApproximatorMidpointTwoPart<ArcBezierErrorApproximator>
	    ::approximate_bezier_with_arc (b, error, mid_t);

    if (ed) {
      *error += ed;

      /* Try a simple one-arc approx which works with the quantized arc.
       * May produce smaller error bound. */
      double e = ArcBezierErrorApproximator::approximate_bezier_arc_error (b, a);
      if (e < *error)
        *error = e;
    }

    return a;
  }
};

typedef MaxDeviationApproximatorExact MaxDeviationApproximatorDefault;
typedef ArcBezierErrorApproximatorBehdad<MaxDeviationApproximatorDefault> ArcBezierErrorApproximatorDefault;
typedef ArcBezierApproximatorMidpointTwoPart<ArcBezierErrorApproximatorDefault> ArcBezierApproximatorDefault;
typedef ArcBezierApproximatorQuantized<ArcBezierErrorApproximatorDefault> ArcBezierApproximatorQuantizedDefault;

} /* namespace ArcBezier */
} /* namespace GLyphy */

#endif /* GLYPHY_ARC_BEZIER_HH */
