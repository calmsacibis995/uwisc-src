#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: swab.c,v 1.3 86/09/08 14:45:26 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)swab.c	5.3 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 * Swab bytes
 * Jeffrey Mogul, Stanford
 */

swab(from, to, n)
	register char *from, *to;
	register int n;
{
	register unsigned long temp;
	
	n >>= 1; n++;
#define	STEP	temp = *from++,*to++ = *from++,*to++ = temp
	/* round to multiple of 8 */
	while ((--n) & 07)
		STEP;
	n >>= 3;
	while (--n >= 0) {
		STEP; STEP; STEP; STEP;
		STEP; STEP; STEP; STEP;
	}
}
