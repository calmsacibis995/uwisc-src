#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: clrerr.c,v 1.2 86/09/08 14:50:43 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)clrerr.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <stdio.h>
#undef	clearerr

clearerr(iop)
	register FILE *iop;
{
	iop->_flag &= ~(_IOERR|_IOEOF);
}
