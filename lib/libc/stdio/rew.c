#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: rew.c,v 1.2 86/09/08 14:51:52 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rew.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include	<stdio.h>

rewind(iop)
register FILE *iop;
{
	fflush(iop);
	lseek(fileno(iop), 0L, 0);
	iop->_cnt = 0;
	iop->_ptr = iop->_base;
	iop->_flag &= ~(_IOERR|_IOEOF);
	if (iop->_flag & _IORW)
		iop->_flag &= ~(_IOREAD|_IOWRT);
}
