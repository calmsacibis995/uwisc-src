#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: memccpy.c,v 1.2 86/09/08 17:03:59 tadl Exp $";
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
static char sccsid[] = "@(#)memccpy.c	5.2 (Berkeley) 86/03/09";
#endif

char *
memccpy(t, f, c, n)
	register char *t, *f;
	register c, n;
{
	while (--n >= 0)
		if ((*t++ = *f++) == c)
			return (t);
	return (0);
}
