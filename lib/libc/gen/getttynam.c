#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: getttynam.c,v 1.3 86/09/08 14:43:28 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getttynam.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <ttyent.h>

struct ttyent *
getttynam(tty)
	char *tty;
{
	register struct ttyent *t;

	setttyent();
	while (t = getttyent()) {
		if (strcmp(tty, t->ty_name) == 0)
			break;
	}
	endttyent();
	return (t);
}
