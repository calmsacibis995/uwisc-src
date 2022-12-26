#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: getchar.c,v 1.2 86/09/08 14:51:37 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getchar.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 * A subroutine version of the macro getchar.
 */
#include <stdio.h>

#undef getchar

getchar()
{
	return(getc(stdin));
}
