#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: strout.c,v 1.2 86/09/08 14:52:14 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strout.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include	<stdio.h>

_strout(count, string, adjust, file, fillch)
register char *string;
register count;
int adjust;
register FILE *file;
{
	while (adjust < 0) {
		if (*string=='-' && fillch=='0') {
			putc(*string++, file);
			count--;
		}
		putc(fillch, file);
		adjust++;
	}
	while (--count>=0)
		putc(*string++, file);
	while (adjust) {
		putc(fillch, file);
		adjust--;
	}
}
