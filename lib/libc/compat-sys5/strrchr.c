#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: strrchr.c,v 1.2 86/09/08 17:05:04 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strrchr.c	5.2 (berkeley) 86/03/09";
#endif LIBC_SCCS and not lint

/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
 *
 * This routine is just "rindex" renamed.
 */

#define NULL 0

char *
strrchr(sp, c)
register char *sp, c;
{
	register char *r;

	r = NULL;
	do {
		if (*sp == c)
			r = sp;
	} while (*sp++);
	return(r);
}
