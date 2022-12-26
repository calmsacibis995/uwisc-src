#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: tmpnam.c,v 1.2 86/09/08 17:05:21 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)tmpnam.c	4.3 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

char *tmpnam(s)
char *s;
{
	static seed;

	sprintf(s, "temp.%d.%d", getpid(), seed++);
	return(s);
}
