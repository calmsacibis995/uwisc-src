#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: strspn.c,v 1.2 86/09/08 17:05:10 tadl Exp $";
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
static char sccsid[] = "@(#)strspn.c	5.2 (Berkeley) 86/03/09";
#endif

strspn(s, set)
	register char *s, *set;
{
	register n = 0;
	register char *p;
	register c;

	while (c = *s++) {
		for (p = set; *p; p++)
			if (c == *p)
				break;
		if (!*p)
			return (n);
		n++;
	}
	return (n);
}
