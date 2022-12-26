#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: memchr.c,v 1.2 86/09/08 17:04:05 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Sys5 compat routine
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)memchr.c	5.2 (Berkeley) 86/03/09";
#endif

char *
memchr(s, c, n)
	register char *s;
	register c, n;
{
	while (--n >= 0)
		if (*s++ == c)
			return (--s);
	return (0);
}
