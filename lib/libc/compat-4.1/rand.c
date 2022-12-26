#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: rand.c,v 1.2 86/09/08 17:02:10 tadl Exp $";
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
static char sccsid[] = "@(#)rand.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

static	long	randx = 1;

srand(x)
unsigned x;
{
	randx = x;
}

rand()
{
	return((randx = randx * 1103515245 + 12345) & 0x7fffffff);
}
