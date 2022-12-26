/*					-[Sat Jan 29 13:57:36 1983 by jkf]-
 * 	gtabs.h				$Locker:  $
 * global lispval table
 *
 * $Header: gtabs.h,v 1.1 86/08/26 22:08:42 root Exp $
 *
 * (c) copyright 1982, Regents of the University of California
 */

/*  these are the tables of global lispvals known to the interpreter	*/
/*  and compiler.  They are not used by the garbage collector.		*/
#define GFTABLEN 200
#define GCTABLEN 8
extern lispval gftab[GFTABLEN];
extern lispval gctab[GCTABLEN];
