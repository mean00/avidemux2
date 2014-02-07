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

#ifndef GLYPHY_GEOMETRY_HH
#define GLYPHY_GEOMETRY_HH

#include "glyphy-common.hh"

namespace GLyphy {
namespace Geometry {

template <typename Type> struct Pair;
struct Vector;
struct SignedVector;
struct Point;
struct Line;
struct Segment;
struct Arc;
struct Bezier;

/* returns tan (2 * atan (d)) */
inline double tan2atan (double d) { return 2 * d / (1 - d*d); }

/* returns sin (2 * atan (d)) */
inline double sin2atan (double d) { return 2 * d / (1 + d*d); }

/* returns cos (2 * atan (d)) */
inline double cos2atan (double d) { return (1 - d*d) / (1 + d*d); }

template <typename Type>
struct Pair {
  typedef Type ElementType;

  inline Pair (const Type &first_, const Type &second_) : first (first_), second (second_) {}

  Type first, second;
};

struct Point : glyphy_point_t {
  inline Point (double x_, double y_) { x = x_; y = y_; }
  inline explicit Point (const Vector &v);
  inline Point (const glyphy_point_t &p) { *(glyphy_point_t *)this = p; }

  inline bool operator == (const Point &p) const;
  inline bool operator != (const Point &p) const;
  inline Point& operator+= (const Vector &v);
  inline Point& operator-= (const Vector &v);
  inline const Point operator+ (const Vector &v) const;
  inline const Point operator- (const Vector &v) const;
  inline const Vector operator- (const Point &p) const;
  inline const Point midpoint (const Point &p) const;
  inline const Line bisector (const Point &p) const;
  inline double distance_to_point (const Point &p) const; /* distance to point! */
  inline double squared_distance_to_point (const Point &p) const; /* square of distance to point! */

  inline bool is_finite (void) const;
  inline const Point lerp (const double &a, const Point &p) const;
};

struct Vector {
  inline Vector (double dx_, double dy_) : dx (dx_), dy (dy_) {}
  inline explicit Vector (const Point &p) : dx (p.x), dy (p.y) {}

  inline bool operator == (const Vector &v) const;
  inline bool operator != (const Vector &v) const;
  inline const Vector operator+ (void) const;
  inline const Vector operator- (void) const;
  inline Vector& operator+= (const Vector &v);
  inline Vector& operator-= (const Vector &v);
  inline Vector& operator*= (const double &s);
  inline Vector& operator/= (const double &s);
  inline const Vector operator+ (const Vector &v) const;
  inline const Vector operator- (const Vector &v) const;
  inline const Vector operator* (const double &s) const;
  inline const Vector operator/ (const double &s) const;
  inline double operator* (const Vector &v) const; /* dot product */
  inline const Point operator+ (const Point &p) const;

  inline bool is_nonzero (void) const;
  inline double len (void) const;
  inline double len2 (void) const;
  inline const Vector normalized (void) const;
  inline const Vector ortho (void) const;
  inline const Vector normal (void) const; /* ortho().normalized() */
  inline double angle (void) const;

  inline const Vector rebase (const Vector &bx, const Vector &by) const;
  inline const Vector rebase (const Vector &bx) const;

  double dx, dy;
};

struct SignedVector : Vector {
  inline SignedVector (const Vector &v, bool negative_) : Vector (v), negative (negative_) {}

  inline bool operator == (const SignedVector &v) const;
  inline bool operator != (const SignedVector &v) const;
  inline const SignedVector operator- (void) const;

  bool negative;
};

struct Line {
  inline Line (double a_, double b_, double c_) : n (a_, b_), c (c_) {}
  inline Line (Vector n_, double c_) : n (n_), c (c_) {}
  inline Line (const Point &p0, const Point &p1) :
               n ((p1 - p0).ortho ()), c (n * Vector (p0)) {}

  inline const Point operator+ (const Line &l) const; /* line intersection! */
  inline const SignedVector operator- (const Point &p) const; /* shortest vector from point to line */


  inline const Line normalized (void) const;
  inline const Vector normal (void) const;

  Vector n; /* line normal */
  double c; /* n.dx*x + n.dy*y = c */
};

struct Segment {
  inline Segment (const Point &p0_, const Point &p1_) :
		  p0 (p0_), p1 (p1_) {}

  inline const SignedVector operator- (const Point &p) const; /* shortest vector from point to ***line*** */
  inline double distance_to_point (const Point &p) const; /* shortest distance from point to segment */
  inline double squared_distance_to_point (const Point &p) const; /* shortest distance squared from point to segment */
  inline bool contains_in_span (const Point &p) const; /* is p in the stripe formed by sliding this segment? */
  inline double max_distance_to_arc (const Arc &a) const;


  Point p0;
  Point p1;
};



struct Arc {
  inline Arc (const Point &p0_, const Point &p1_, const Point &pm, bool complement) :
	      p0 (p0_), p1 (p1_),
	      d (p0_ == pm || p1_ == pm ? 0 :
		 tan (((p1_-pm).angle () - (p0_-pm).angle ()) / 2 - (complement ? 0 : M_PI_2))) {}
  inline Arc (const Point &p0_, const Point &p1_, const double &d_) :
	      p0 (p0_), p1 (p1_), d (d_) {}
  inline Arc (const Point &center, double radius, const double &a0, const double &a1, bool complement) :
	      p0 (center + Vector (cos(a0),sin(a0)) * radius),
	      p1 (center + Vector (cos(a1),sin(a1)) * radius),
	      d (tan ((a1 - a0) / 4 - (complement ? 0 : M_PI_2))) {}
  inline Arc (const glyphy_arc_t &a) : p0 (a.p0), p1 (a.p1), d (a.d) {}
  inline operator glyphy_arc_t (void) const { glyphy_arc_t a = {p0, p1, d}; return a; }

  inline bool operator == (const Arc &a) const;
  inline bool operator != (const Arc &a) const;
  inline const SignedVector operator- (const Point &p) const; /* shortest vector from point to arc */

  inline double radius (void) const;
  inline const Point center (void) const;
  inline const Pair<Vector> tangents (void) const;

  inline Bezier approximate_bezier (double *error) const;

  inline bool wedge_contains_point (const Point &p) const;
  inline double distance_to_point (const Point &p) const;
  inline double squared_distance_to_point (const Point &p) const;
  inline double extended_dist (const Point &p) const;

  inline void extents (glyphy_extents_t &extents) const;

  Point p0, p1;
  double d; /* Depth */
};

struct Bezier {
  inline Bezier (const Point &p0_, const Point &p1_,
		 const Point &p2_, const Point &p3_) :
		 p0 (p0_), p1 (p1_), p2 (p2_), p3 (p3_) {}

  inline const Point point (const double &t) const;
  inline const Point midpoint (void) const;
  inline const Vector tangent (const double &t) const;
  inline const Vector d_tangent (const double &t) const;
  inline double curvature (const double &t) const;
  inline const Pair<Bezier> split (const double &t) const;
  inline const Pair<Bezier> halve (void) const;
  inline const Bezier segment (const double &t0, const double &t1) const;

  Point p0, p1, p2, p3;
};


/* Implementations */


/* Point */

inline Point::Point (const Vector &v) {
  x = v.dx;
  y = v.dy;
}
inline bool Point::operator == (const Point &p) const {
  return x == p.x && y == p.y;
}
inline bool Point::operator != (const Point &p) const {
  return !(*this == p);
}
inline Point& Point::operator+= (const Vector &v) {
  x += v.dx;
  y += v.dy;
  return *this;
}
inline Point& Point::operator-= (const Vector &v) {
  x -= v.dx;
  y -= v.dy;
  return *this;
}
inline const Point Point::operator+ (const Vector &v) const {
  return Point (*this) += v;
}
inline const Point Point::operator- (const Vector &v) const {
  return Point (*this) -= v;
}
inline const Vector Point::operator- (const Point &p) const {
  return Vector (x - p.x, y - p.y);
}

inline const Point Point::midpoint (const Point &p) const {
  return *this + (p - *this) / 2;
}
inline const Line Point::bisector (const Point &p) const {
  Vector d = p - *this;
  return Line (d.dx * 2, d.dy * 2, d * Vector (p) + d * Vector (*this));
}

inline double Point::distance_to_point (const Point &p) const {
  return ((*this) - p).len ();
}

inline double Point::squared_distance_to_point (const Point &p) const {
  return ((*this) - p).len2 ();
}

inline bool Point::is_finite (void) const {
  return isfinite (x) && isfinite (y);
}
inline const Point Point::lerp (const double &a, const Point &p) const {
  /* The following two cases are special-cased to get better floating
   * point stability.  We require that points that are the same be
   * bit-equal. */
  if (a == 0)   return *this;
  if (a == 1.0) return p;
  return Point ((1-a) * x + a * p.x, (1-a) * y + a * p.y);
}


/* Vector */

inline bool Vector::operator == (const Vector &v) const {
  return dx == v.dx && dy == v.dy;
}
inline bool Vector::operator != (const Vector &v) const {
  return !(*this == v);
}
inline const Vector Vector::operator+ (void) const {
  return *this;
}
inline const Vector Vector::operator- (void) const {
  return Vector (-dx, -dy);
}
inline Vector& Vector::operator+= (const Vector &v) {
  dx += v.dx;
  dy += v.dy;
  return *this;
}
inline Vector& Vector::operator-= (const Vector &v) {
  dx -= v.dx;
  dy -= v.dy;
  return *this;
}
inline Vector& Vector::operator*= (const double &s) {
  dx *= s;
  dy *= s;
  return *this;
}
inline Vector& Vector::operator/= (const double &s) {
  dx /= s;
  dy /= s;
  return *this;
}
inline const Vector Vector::operator+ (const Vector &v) const {
  return Vector (*this) += v;
}
inline const Vector Vector::operator- (const Vector &v) const {
  return Vector (*this) -= v;
}
inline const Vector Vector::operator* (const double &s) const {
  return Vector (*this) *= s;
}
inline const Vector operator* (const double &s, const Vector &v) {
  return v * s;
}
inline const Vector Vector::operator/ (const double &s) const {
  return Vector (*this) /= s;
}
inline double Vector::operator* (const Vector &v) const { /* dot product */
  return dx * v.dx + dy * v.dy;
}
inline const Point Vector::operator+ (const Point &p) const {
  return p + *this;
}

inline bool Vector::is_nonzero (void) const {
  return dx || dy;
}
inline double Vector::len (void) const {
  return hypot (dx, dy);
}
inline double Vector::len2 (void) const {
  return dx * dx + dy * dy;
}
inline const Vector Vector::normalized (void) const {
  double d = len ();
  return d ? *this / d : *this;
}
inline const Vector Vector::ortho (void) const {
  return Vector (-dy, dx);
}
inline const Vector Vector::normal (void) const {
  return ortho ().normalized ();
}
inline double Vector::angle (void) const {
  return atan2 (dy, dx);
}

inline const Vector Vector::rebase (const Vector &bx,
				    const Vector &by) const {
  return Vector (*this * bx, *this * by);
}
inline const Vector Vector::rebase (const Vector &bx) const {
  return rebase (bx, bx.ortho ());
}


/* SignedVector */

inline bool SignedVector::operator == (const SignedVector &v) const {
  return (const Vector &)(*this) == (const Vector &)(v) && negative == v.negative;
}
inline bool SignedVector::operator != (const SignedVector &v) const {
  return !(*this == v);
}
inline const SignedVector SignedVector::operator- (void) const {
  return SignedVector (-(const Vector &)(*this), !negative);
}


/* Line */

inline const Point Line::operator+ (const Line &l) const {
  double det = n.dx * l.n.dy - n.dy * l.n.dx;
  if (!det)
    return Point (GLYPHY_INFINITY, GLYPHY_INFINITY);
  return Point ((c * l.n.dy - n.dy * l.c) / det,
		       (n.dx * l.c - c * l.n.dx) / det);
}
inline const SignedVector Line::operator- (const Point &p) const {
  double mag = -(n * Vector (p) - c) / n.len ();
  return SignedVector (n.normalized () * mag, mag < 0); /******************************************************************************************* FIX. *************************************/
}

inline const SignedVector operator- (const Point &p, const Line &l) {
  return -(l - p);
}

inline const Line Line::normalized (void) const {
  double d = n.len ();
  return d ? Line (n / d, c / d) : *this;
}
inline const Vector Line::normal (void) const {
  return n;
}

/* Segment */
inline const SignedVector Segment::operator- (const Point &p) const {
  /* shortest vector from point to line */
  return p - Line (p1, p0); /************************************************************************************************** Should the order (p1, p0) depend on d?? ***********************/
}

/* Segment */
inline bool Segment::contains_in_span (const Point &p) const {
  if (p0 == p1)
    return false;

  /* shortest vector from point to line */
  Line temp (p0, p1);
  double mag = -(temp.n * Vector (p) - temp.c) / temp.n.len ();
  Vector y (temp.n.normalized () * mag);
  Point z = y + p;

  // Check if z is between p0 and p1.

  if (fabs (p1.y - p0.y) > fabs (p1.x - p0.x)) {
    return ((z.y - p0.y > 0 && p1.y - p0.y > z.y - p0.y) ||
            (z.y - p0.y < 0 && p1.y - p0.y < z.y - p0.y));
  }
  else {
    return ((0 < z.x - p0.x && z.x - p0.x < p1.x - p0.x) ||
            (0 > z.x - p0.x && z.x - p0.x > p1.x - p0.x));
  }
}

inline double Segment::distance_to_point (const Point &p) const {
  if (p0 == p1)
    return 0;

  // Check if z is between p0 and p1.
  Line temp (p0, p1);
  if (contains_in_span (p))
    return -(temp.n * Vector (p) - temp.c) / temp.n.len ();

  double dist_p_p0 = p.distance_to_point (p0);
  double dist_p_p1 = p.distance_to_point (p1);
  return (dist_p_p0 < dist_p_p1 ? dist_p_p0 : dist_p_p1) * (-(temp.n * Vector (p) - temp.c) < 0 ? -1 : 1);
}


inline double Segment::squared_distance_to_point (const Point &p) const {
  if (p0 == p1)
    return 0;

  // Check if z is between p0 and p1.
  Line temp (p0, p1);
  if (contains_in_span (p))
    return (temp.n * Vector (p) - temp.c) * (temp.n * Vector (p) - temp.c) / (temp.n * temp.n);

  double dist_p_p0 = p.squared_distance_to_point (p0);
  double dist_p_p1 = p.squared_distance_to_point (p1);
  return (dist_p_p0 < dist_p_p1 ? dist_p_p0 : dist_p_p1);
}


inline double Segment::max_distance_to_arc (const Arc &a) const {
  double max_distance = fabs(a.distance_to_point(p0)) ;
  return  max_distance >  fabs(a.distance_to_point(p1)) ? max_distance : fabs(a.distance_to_point(p1)) ;
}



/* Arc */

inline bool Arc::operator == (const Arc &a) const {
  return p0 == a.p0 && p1 == a.p1 && d == a.d;
}
inline bool Arc::operator != (const Arc &a) const {
  return !(*this == a);
}


inline const SignedVector Arc::operator- (const Point &p) const {

  if (fabs(d) < 1e-5) {
    Segment arc_segment (p0, p1);
    return arc_segment - p;
  }
  if (wedge_contains_point (p)){
    Vector difference = (center () - p).normalized () * fabs (p.distance_to_point (center ()) - radius ());

    return SignedVector  (difference, ((p - center ()).len () < radius ()) ^ (d < 0));
  }
  double d0 = p.squared_distance_to_point (p0);
  double d1 = p.squared_distance_to_point (p1);

  Arc other_arc (p0, p1, (1.0 + d) / (1.0 - d));  /********************************* NOT Robust. But works? *****************/
  Vector normal = center () - (d0 < d1 ? p0 : p1) ;

  if (normal.len() == 0)
    return SignedVector (Vector (0, 0), true);    /************************************ Check sign of this S.D. *************/

  return SignedVector (Line (normal.dx, normal.dy, normal * Vector ((d0 < d1 ? p0 : p1))) - p, !other_arc.wedge_contains_point(p));
}

inline const SignedVector operator- (const Point &p, const Arc &a) {
  return -(a - p);
}



inline double Arc::radius (void) const
{
  return fabs ((p1 - p0).len () / (2 * sin2atan (d)));
}

inline const Point Arc::center (void) const
{
  return (p0.midpoint (p1)) + (p1 - p0).ortho () / (2 * tan2atan (d));
}

inline const Pair<Vector> Arc::tangents (void) const
{
  Vector dp = (p1 - p0) * .5;
  Vector pp = dp.ortho () * -sin2atan (d);
  dp = dp * cos2atan (d);
  return Pair<Vector> (dp + pp, dp - pp);
}



inline Bezier Arc::approximate_bezier (double *error) const
{
  Vector dp = p1 - p0;
  Vector pp = dp.ortho ();

  if (error)
    *error = dp.len () * pow (fabs (d), 5) / (54 * (1 + d*d));

  dp *= ((1 - d*d) / 3);
  pp *= (2 * d / 3);

  Point p0s = p0 + dp - pp;
  Point p1s = p1 - dp - pp;

  return Bezier (p0, p0s, p1s, p1);
}


inline bool Arc::wedge_contains_point (const Point &p) const
{
  Pair<Vector> t = tangents ();
  if (fabs (d) <= 1)
    return (p - p0) * t.first  >= 0 && (p - p1) * t.second <= 0;
  else
    return (p - p0) * t.first  >= 0 || (p - p1) * t.second <= 0;
}


/* Distance may not always be positive, but will be to an endpoint whenever necessary. */
inline double Arc::distance_to_point (const Point &p) const {
  if (fabs(d) < 1e-5) {
    Segment arc_segment (p0, p1);
    return arc_segment.distance_to_point (p);
  }

  SignedVector difference = *this - p;

  if (wedge_contains_point (p) && fabs(d) > 1e-5)
    return fabs (p.distance_to_point (center ()) - radius ()) * (difference.negative ? -1 : 1);
  double d1 = p.squared_distance_to_point (p0);
  double d2 = p.squared_distance_to_point (p1);
  return (d1 < d2 ? sqrt(d1) : sqrt(d2)) * (difference.negative ? -1 : 1);
}

/* Distance will be to an endpoint whenever necessary. */
inline double Arc::squared_distance_to_point (const Point &p) const {
  if (fabs(d) < 1e-5) {
    Segment arc_segment (p0, p1);
    return arc_segment.squared_distance_to_point (p);
  }

  //SignedVector difference = *this - p;

  if (wedge_contains_point (p) && fabs(d) > 1e-5) {
    double answer = p.distance_to_point (center ()) - radius ();
    return answer * answer;
  }
  double d1 = p.squared_distance_to_point (p0);
  double d2 = p.squared_distance_to_point (p1);
  return (d1 < d2 ? d1 : d2);
}

inline double Arc::extended_dist (const Point &p) const {
  Point m = p0.lerp (.5, p1);
  Vector dp = p1 - p0;
  Vector pp = dp.ortho ();
  float d2 = tan2atan (d);
  if ((p - m) * (p1 - m) < 0)
    return (p - p0) * (pp + dp * d2).normalized ();
  else
    return (p - p1) * (pp - dp * d2).normalized ();
}

inline void Arc::extents (glyphy_extents_t &extents) const {
  glyphy_extents_clear (&extents);
  glyphy_extents_add (&extents, &p0);
  glyphy_extents_add (&extents, &p1);
  Point c = center ();
  double r = radius ();
  Point p[4] = {c + r * Vector (-1,  0),
		c + r * Vector (+1,  0),
		c + r * Vector ( 0, -1),
		c + r * Vector ( 0, +1)};
  for (unsigned int i = 0; i < 4; i++)
    if (wedge_contains_point (p[i]))
      glyphy_extents_add (&extents, &p[i]);
}


/* Bezier */

inline const Point Bezier::point (const double &t) const {
  Point p01 = p0.lerp (t, p1);
  Point p12 = p1.lerp (t, p2);
  Point p23 = p2.lerp (t, p3);
  Point p012 = p01.lerp (t, p12);
  Point p123 = p12.lerp (t, p23);
  Point p0123 = p012.lerp (t, p123);
  return p0123;
}

inline const Point Bezier::midpoint (void) const
{
  Point p01 = p0.midpoint (p1);
  Point p12 = p1.midpoint (p2);
  Point p23 = p2.midpoint (p3);
  Point p012 = p01.midpoint (p12);
  Point p123 = p12.midpoint (p23);
  Point p0123 = p012.midpoint (p123);
  return p0123;
}

inline const Vector Bezier::tangent (const double &t) const
{
  double t_2_0 = t * t;
  double t_0_2 = (1 - t) * (1 - t);

  double _1__4t_1_0_3t_2_0 = 1 - 4 * t + 3 * t_2_0;
  double _2t_1_0_3t_2_0    =     2 * t - 3 * t_2_0;

  return Vector (-3 * p0.x * t_0_2
			+3 * p1.x * _1__4t_1_0_3t_2_0
			+3 * p2.x * _2t_1_0_3t_2_0
			+3 * p3.x * t_2_0,
			-3 * p0.y * t_0_2
			+3 * p1.y * _1__4t_1_0_3t_2_0
			+3 * p2.y * _2t_1_0_3t_2_0
			+3 * p3.y * t_2_0);
}

inline const Vector Bezier::d_tangent (const double &t) const {
  return Vector (6 * ((-p0.x + 3*p1.x - 3*p2.x + p3.x) * t + (p0.x - 2*p1.x + p2.x)),
			6 * ((-p0.y + 3*p1.y - 3*p2.y + p3.y) * t + (p0.y - 2*p1.y + p2.y)));
}

inline double Bezier::curvature (const double &t) const {
  Vector dpp = tangent (t).ortho ();
  Vector ddp = d_tangent (t);
  /* normal vector len squared */
  double len = dpp.len ();
  double curvature = (dpp * ddp) / (len * len * len);
  return curvature;
}

inline const Pair<Bezier > Bezier::split (const double &t) const {
  Point p01 = p0.lerp (t, p1);
  Point p12 = p1.lerp (t, p2);
  Point p23 = p2.lerp (t, p3);
  Point p012 = p01.lerp (t, p12);
  Point p123 = p12.lerp (t, p23);
  Point p0123 = p012.lerp (t, p123);
  return Pair<Bezier> (Bezier (p0, p01, p012, p0123),
		       Bezier (p0123, p123, p23, p3));
}

inline const Pair<Bezier > Bezier::halve (void) const
{
  Point p01 = p0.midpoint (p1);
  Point p12 = p1.midpoint (p2);
  Point p23 = p2.midpoint (p3);
  Point p012 = p01.midpoint (p12);
  Point p123 = p12.midpoint (p23);
  Point p0123 = p012.midpoint (p123);
  return Pair<Bezier> (Bezier (p0, p01, p012, p0123),
		       Bezier (p0123, p123, p23, p3));
}

inline const Bezier Bezier::segment (const double &t0, const double &t1) const
{
  Point p01 = p0.lerp (t0, p1);
  Point p12 = p1.lerp (t0, p2);
  Point p23 = p2.lerp (t0, p3);
  Point p012 = p01.lerp (t0, p12);
  Point p123 = p12.lerp (t0, p23);
  Point p0123 = p012.lerp (t0, p123);

  Point q01 = p0.lerp (t1, p1);
  Point q12 = p1.lerp (t1, p2);
  Point q23 = p2.lerp (t1, p3);
  Point q012 = q01.lerp (t1, q12);
  Point q123 = q12.lerp (t1, q23);
  Point q0123 = q012.lerp (t1, q123);

  return Bezier (p0123,
		 p0123 + (p123 - p0123) * ((t1 - t0) / (1 - t0)),
		 q0123 + (q012 - q0123) * ((t1 - t0) / t1),
		 q0123);
}

} /* namespace Geometry */
} /* namespace GLyphy */

#endif /* GLYPHY_GEOMETRY_HH */
