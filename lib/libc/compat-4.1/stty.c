#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: stty.c,v 1.2 86/09/08 17:02:15 tadl Exp $";
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
static char sccsid[] = "@(#)stty.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 * Writearound to old stty system call.
 */

#include <sgtty.h>

stty(fd, ap)
	struct sgtty *ap;
{

	return(ioctl(fd, TIOCSETP, ap));
}
