#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: strpbrk.c,v 1.2 86/09/08 17:04:58 tadl Exp $";
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
 * Sys 5 compat routine...
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strpbrk.c	5.2 (Berkeley) 86/03/09";
#endif

char *
strpbrk(s, brk)
	register char *s, *brk;
{
	register char *p;
	register c;

	while (c = *s) {
		for (p = brk; *p; p++)
			if (c == *p)
				return (s);
		s++;
	}
	return (0);
}
