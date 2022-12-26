#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: fakcu.c,v 1.3 86/09/08 14:42:41 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)fakcu.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 * Null cleanup routine to resolve reference in exit() 
 * if not using stdio.
 */
_cleanup()
{
}
