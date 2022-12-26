#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: putchar.c,v 1.2 86/09/08 14:51:46 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)putchar.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 * A subroutine version of the macro putchar
 */
#include <stdio.h>

#undef putchar

putchar(c)
register c;
{
	putc(c, stdout);
}
