#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header:$";
#endif
/*
 * RCS info
 *	$Locker:$
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)fgets.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include	<stdio.h>

char *
fgets(s, n, iop)
char *s;
register FILE *iop;
{
	register c;
	register char *cs;

	cs = s;
	while (--n>0 && (c = getc(iop)) != EOF) {
		*cs++ = c;
		if (c=='\n')
			break;
	}
	if (c == EOF && cs==s)
		return(NULL);
	*cs++ = '\0';
	return(s);
}
