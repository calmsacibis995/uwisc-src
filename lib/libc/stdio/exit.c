#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: exit.c,v 1.2 86/09/08 14:50:50 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)exit.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

exit(code)
	int code;
{

	_cleanup();
	_exit(code);
}
