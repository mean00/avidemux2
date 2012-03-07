#include <math.h>
#ifndef M_E
 #define M_E     2.7182818284590452354
#endif
#ifndef M_PI
 #define M_PI    3.14159265358979323846
#endif

#include <errno.h>

/*
 * template for tinypy math functions
 * with one parameter. 
 *
 * @cfunc is the coresponding function name in C
 * math library.
 */
#define TP_MATH_FUNC1(cfunc)                        \
    static tp_obj math_##cfunc(TP) {                \
        double x = TP_NUM();                        \
        double r = 0.0;                             \
                                                    \
        errno = 0;                                  \
        r = cfunc(x);                               \
        if (errno == EDOM || errno == ERANGE) {     \
            tp_raise(tp_None, tp_printf(tp, "%s(x): x=%f "		\
                                        "out of range", __func__, x));	\
        }                                           \
                                                    \
        return (tp_number(r));                      \
    }

/*
 * template for tinypy math functions
 * with two parameters. 
 *
 * @cfunc is the coresponding function name in C
 * math library.
 */
#define TP_MATH_FUNC2(cfunc)                        \
    static tp_obj math_##cfunc(TP) {                \
        double x = TP_NUM();                        \
        double y = TP_NUM();                        \
        double r = 0.0;                             \
                                                    \
        errno = 0;                                  \
        r = cfunc(x, y);                            \
        if (errno == EDOM || errno == ERANGE) {     \
            tp_raise(tp_None, tp_printf(tp, "%s(x, y): x=%f,y=%f "	\
                                        "out of range", __func__, x, y)); \
        }                                           \
                                                    \
        return (tp_number(r));                      \
    }


/*
 * PI definition: 3.1415926535897931
 */
static tp_obj   math_pi;

/*
 * E definition: 2.7182818284590451
 */
static tp_obj   math_e;

/*
 * acos(x)
 *
 * return arc cosine of x, return value is measured in radians.
 * if x falls out -1 to 1, raise out-of-range exception.
 */
TP_MATH_FUNC1(acos)

/*
 * asin(x)
 *
 * return arc sine of x, measured in radians, actually [-PI/2, PI/2]
 * if x falls out of -1 to 1, raise out-of-range exception
 */
TP_MATH_FUNC1(asin)

/*
 * atan(x)
 *
 * return arc tangent of x, measured in radians,
 */
TP_MATH_FUNC1(atan)

/*
 * atan2(x, y)
 *
 * return arc tangent of x/y, measured in radians.
 * unlike atan(x/y), both the signs of x and y 
 * are considered to determine the quaderant of 
 * the result.
 */
TP_MATH_FUNC2(atan2)

/*
 * ceil(x)
 *
 * return the ceiling of x, i.e, the smallest
 * integer >= x.
 */
TP_MATH_FUNC1(ceil)

/*
 * cos(x)
 *
 * return cosine of x. x is measured in radians.
 */
TP_MATH_FUNC1(cos)

/*
 * cosh(x)
 *
 * return hyperbolic cosine of x.
 */
TP_MATH_FUNC1(cosh)

/*
 * degrees(x)
 *
 * converts angle x from radians to degrees.
 * NOTE: this function is introduced by python,
 * so we cannot wrap it directly in TP_MATH_FUNC1(),
 * here the solution is defining a new 
 * C function - degrees().
 */
static const double degToRad = 
                3.141592653589793238462643383 / 180.0;
static double degrees(double x)
{
    return (x / degToRad);
}

TP_MATH_FUNC1(degrees)

/*
 * exp(x)
 *
 * return the value e raised to power of x.
 * e is the base of natural logarithms.
 */
TP_MATH_FUNC1(exp)

/*
 * fabs(x)
 *
 * return the absolute value of x.
 */
TP_MATH_FUNC1(fabs)

/*
 * floor(x)
 *
 * return the floor of x, i.e, the largest integer <= x
 */
TP_MATH_FUNC1(floor)

/*
 * fmod(x, y)
 *
 * return the remainder of dividing x by y. that is,
 * return x - n * y, where n is the quotient of x/y.
 * NOTE: this function relies on the underlying platform.
 */
TP_MATH_FUNC2(fmod)

/*
 * frexp(x)
 *
 * return a pair (r, y), which satisfies:
 * x = r * (2 ** y), and r is normalized fraction
 * which is laid between 1/2 <= abs(r) < 1.
 * if x = 0, the (r, y) = (0, 0).
 */
static tp_obj math_frexp(TP) {
    double x = TP_NUM();
    int    y = 0;   
    double r = 0.0;
    tp_obj rList = tp_list(tp);

    errno = 0;
    r = frexp(x, &y);
    if (errno == EDOM || errno == ERANGE) {
        tp_raise(tp_None, tp_printf(tp, "%s(x): x=%f, "
                                    "out of range", __func__, x));
    }

    _tp_list_append(tp, rList.list.val, tp_number(r));
    _tp_list_append(tp, rList.list.val, tp_number((tp_num)y));
    return (rList);
}


/*
 * hypot(x, y)
 *
 * return Euclidean distance, namely,
 * sqrt(x*x + y*y)
 */
TP_MATH_FUNC2(hypot)


/*
 * ldexp(x, y)
 *
 * return the result of multiplying x by 2
 * raised to y.
 */
TP_MATH_FUNC2(ldexp)

/*
 * log(x, [base])
 *
 * return logarithm of x to given base. If base is
 * not given, return the natural logarithm of x.
 * Note: common logarithm(log10) is used to compute 
 * the denominator and numerator. based on fomula:
 * log(x, base) = log10(x) / log10(base).
 */
static tp_obj math_log(TP) {
    double x = TP_NUM();
    tp_obj b = TP_DEFAULT(tp_None);
    double y = 0.0;
    double den = 0.0;   /* denominator */
    double num = 0.0;   /* numinator */
    double r = 0.0;     /* result */

    if (b.type == TP_NONE)
        y = M_E;
    else if (b.type == TP_NUMBER)
        y = (double)b.number.val;
    else
        tp_raise(tp_None, tp_printf(tp, "%s(x, [base]): base invalid", __func__));

    errno = 0;
    num = log10(x);
    if (errno == EDOM || errno == ERANGE)
        goto excep;

    errno = 0;
    den = log10(y);
    if (errno == EDOM || errno == ERANGE)
        goto excep;

    r = num / den;

    return (tp_number(r));

excep:
    tp_raise(tp_None, tp_printf(tp, "%s(x, y): x=%f,y=%f "
                                "out of range", __func__, x, y));
}

/*
 * log10(x)
 *
 * return 10-based logarithm of x.
 */
TP_MATH_FUNC1(log10)

/*
 * modf(x)
 *
 * return a pair (r, y). r is the integral part of
 * x and y is the fractional part of x, both holds
 * the same sign as x.
 */
static tp_obj math_modf(TP) {
    double x = TP_NUM();
    double y = 0.0; 
    double r = 0.0;
    tp_obj rList = tp_list(tp);

    errno = 0;
    r = modf(x, &y);
    if (errno == EDOM || errno == ERANGE) {
        tp_raise(tp_None, tp_printf(tp, "%s(x): x=%f, "
                                    "out of range", __func__, x));
    }

    _tp_list_append(tp, rList.list.val, tp_number(r));
    _tp_list_append(tp, rList.list.val, tp_number(y));
    return (rList);
}

/*
 * pow(x, y)
 *
 * return value of x raised to y. equivalence of x ** y.
 * NOTE: conventionally, tp_pow() is the implementation
 * of builtin function pow(); whilst, math_pow() is an
 * alternative in math module.
 */
static tp_obj math_pow(TP) {
    double x = TP_NUM();
    double y = TP_NUM();
    double r = 0.0;

    errno = 0;
    r = pow(x, y);
    if (errno == EDOM || errno == ERANGE) {
        tp_raise(tp_None, tp_printf(tp, "%s(x, y): x=%f,y=%f "
                                    "out of range", __func__, x, y));
    }

    return (tp_number(r));
}


/*
 * radians(x)
 *
 * converts angle x from degrees to radians.
 * NOTE: this function is introduced by python,
 * adopt same solution as degrees(x).
 */
static double radians(double x)
{
    return (x * degToRad);
}

TP_MATH_FUNC1(radians)

/*
 * sin(x)
 *
 * return sine of x, x is measured in radians.
 */
TP_MATH_FUNC1(sin)

/*
 * sinh(x)
 *
 * return hyperbolic sine of x.
 * mathematically, sinh(x) = (exp(x) - exp(-x)) / 2.
 */
TP_MATH_FUNC1(sinh)

/*
 * sqrt(x)
 *
 * return square root of x.
 * if x is negtive, raise out-of-range exception.
 */
TP_MATH_FUNC1(sqrt)

/*
 * tan(x)
 *
 * return tangent of x, x is measured in radians.
 */
TP_MATH_FUNC1(tan)

/*
 * tanh(x)
 * 
 * return hyperbolic tangent of x.
 * mathematically, tanh(x) = sinh(x) / cosh(x).
 */
TP_MATH_FUNC1(tanh)
