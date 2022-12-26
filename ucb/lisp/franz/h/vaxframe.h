/*					-[Sat Jan 29 14:02:34 1983 by jkf]-
 * 	vaxframe.h			$Locker:  $
 * vax calling frame definition
 *
 * $Header: vaxframe.h,v 1.1 86/08/26 22:09:02 root Exp $
 *
 * (c) copyright 1982, Regents of the University of California
 */

struct machframe {
	lispval	(*handler)();
	long	mask;
	lispval	*ap;
struct 	machframe	*fp;
	lispval	(*pc)();
	lispval	*r6;
	lispval *r7;
};
