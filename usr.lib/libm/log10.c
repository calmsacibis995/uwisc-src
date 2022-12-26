/* 
 * Copyright (c) 1985 Regents of the University of California.
 * 
 * Use and reproduction of this software are granted  in  accordance  with
 * the terms and conditions specified in  the  Berkeley  Software  License
 * Agreement (in particular, this entails acknowledgement of the programs'
 * source, and inclusion of this notice) with the additional understanding
 * that  all  recipients  should regard themselves as participants  in  an
 * ongoing  research  project and hence should  feel  obligated  to report
 * their  experiences (good or bad) with these elementary function  codes,
 * using "sendbug 4bsd-bugs@BERKELEY", to the authors.
 */

#ifndef lint
static char sccsid[] = "@(#)log10.c	1.2 (Berkeley) 8/21/85";
#endif not lint

/* LOG10(X)
 * RETURN THE BASE 10 LOGARITHM OF x
 * DOUBLE PRECISION (VAX D format 56 bits, IEEE DOUBLE 53 BITS)
 * CODED IN C BY K.C. NG, 1/20/85; 
 * REVISED BY K.C. NG on 1/23/85, 3/7/85, 4/16/85.
 * 
 * Required kernel function:
 *	log(x)
 *
 * Method :
 *			     log(x)
 *		log10(x) = ---------  or  [1/log(10)]*log(x)
 *			    log(10)
 *
 *    Note:
 *	  [log(10)]   rounded to 56 bits has error  .0895  ulps,
 *	  [1/log(10)] rounded to 53 bits has error  .198   ulps;
 *	  therefore, for better accuracy, in VAX D format, we divide 
 *	  log(x) by log(10), but in IEEE Double format, we multiply 
 *	  log(x) by [1/log(10)].
 *
 * Special cases:
 *	log10(x) is NaN with signal if x < 0; 
 *	log10(+INF) is +INF with no signal; log10(0) is -INF with signal;
 *	log10(NaN) is that NaN with no signal.
 *
 * Accuracy:
 *	log10(X) returns the exact log10(x) nearly rounded. In a test run
 *	with 1,536,000 random arguments on a VAX, the maximum observed
 *	error was 1.74 ulps (units in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 */

#ifdef VAX	/* VAX D format (56 bits) */
/* static double */
/* ln10hi =  2.3025850929940456790E0     ; Hex   2^  2   *  .935D8DDDAAA8AC */
static long    ln10hix[] = { 0x5d8d4113, 0xa8acddaa};
#define   ln10hi    (*(double*)ln10hix)
#else	/* IEEE double */
static double
ivln10 =  4.3429448190325181667E-1    ; /*Hex   2^ -2   *  1.BCB7B1526E50E */
#endif

double log10(x)
double x;
{
	double log();

#ifdef VAX
	return(log(x)/ln10hi);
#else	/* IEEE double */
	return(ivln10*log(x));
#endif
}
