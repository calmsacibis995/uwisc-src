/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)msgs.h	5.1 (Berkeley) 6/4/85
 */

/*
 * Local definitions for msgs.
 */

#ifdef UW
#define USRMSGS "/usr/spool/msgs"       /* msgs directory */
#else
#define USRMSGS	"/usr/msgs"		/* msgs directory */
#endif UW
#define MAIL	"/usr/ucb/Mail -f %s"	/* could set destination also */
#define PAGE	"/usr/ucb/more -%d"	/* crt screen paging program */
