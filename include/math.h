/*
 * RCS Info	
 *	$Header: math.h,v 1.1 86/09/05 08:02:24 tadl Exp $
 *	$Locker: tadl $
 */
/*	math.h	4.6	9/11/85	*/

extern double asinh(), acosh(), atanh();
extern double erf(), erfc();
extern double exp(), expm1(), log(), log10(), log1p(), pow();
extern double fabs(), floor(), ceil(), rint();
extern double lgamma();
extern double hypot(), cabs();
extern double copysign(), drem(), logb(), scalb();
extern int finite();
#ifdef vax
extern double infnan();
#endif
extern double j0(), j1(), jn(), y0(), y1(), yn();
extern double sin(), cos(), tan(), asin(), acos(), atan(), atan2();
extern double sinh(), cosh(), tanh();
extern double cbrt(), sqrt();
extern double modf(), ldexp(), frexp(), atof();

#define HUGE	1.701411733192644270e38
