#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: printf.c,v 1.2 86/09/08 14:51:43 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)printf.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include	<stdio.h>

printf(fmt, args)
char *fmt;
{
	_doprnt(fmt, &args, stdout);
	return(ferror(stdout)? EOF: 0);
}
