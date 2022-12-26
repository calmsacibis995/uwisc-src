#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: valloc.c,v 1.3 86/09/08 14:46:02 tadl Exp $";
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
static char sccsid[] = "@(#)valloc.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

char	*malloc();

char *
valloc(i)
	int i;
{
	int valsiz = getpagesize(), j;
	char *cp = malloc(i + (valsiz-1));

	j = ((int)cp + (valsiz-1)) &~ (valsiz-1);
	return ((char *)j);
}
