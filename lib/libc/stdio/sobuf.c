#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: sobuf.c,v 1.2 86/09/08 14:52:08 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)sobuf.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <stdio.h>

char	_sobuf[BUFSIZ];
