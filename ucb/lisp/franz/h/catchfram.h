/*					-[Sat Jan 29 13:56:53 1983 by jkf]-
 * 	catchfram.h			$Locker:  $
 * catch frame definition
 *
 * $Header: catchfram.h,v 1.1 86/08/26 22:08:19 root Exp $
 *
 * (c) copyright 1982, Regents of the University of California
 */

struct catchfr {		/* catch and errset frame */
    struct catchfr *link;	/* link to next catchframe */
	   lispval flag;		/* Do we print ?  */
	   lispval labl;	/* label caught at this point  */
    struct nament *svbnp;	/* saved bnp */
	   lispval retenv[11];  /* reset environment - actually a savblock */
	   lispval rs[4];	/* regis 6-11 and 13 */
       lispval (*retadr)();	/* address to continue execution */
};

struct savblock {
	lispval envir[10];
	struct savblock *savlnk;
};
