#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: fwrite.c,v 1.2 86/09/08 14:51:34 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)fwrite.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include	<stdio.h>

fwrite(ptr, size, count, iop)
	register char *ptr;
	unsigned size, count;
	register FILE *iop;
{
	register int s;

	s = size * count;
	if (iop->_flag & _IOLBF)
		while (s > 0) {
			if (--iop->_cnt > -iop->_bufsiz && *ptr != '\n')
				*iop->_ptr++ = *ptr++;
			else if (_flsbuf(*(unsigned char *)ptr++, iop) == EOF)
				break;
			s--;
		}
	else while (s > 0) {
		if (iop->_cnt < s) {
			if (iop->_cnt > 0) {
				bcopy(ptr, iop->_ptr, iop->_cnt);
				ptr += iop->_cnt;
				iop->_ptr += iop->_cnt;
				s -= iop->_cnt;
			}
			if (_flsbuf(*(unsigned char *)ptr++, iop) == EOF)
				break;
			s--;
		}
		if (iop->_cnt >= s) {
			bcopy(ptr, iop->_ptr, s);
			iop->_ptr += s;
			iop->_cnt -= s;
			return (count);
		}
	}
	return (size != 0 ? count - ((s + size - 1) / size) : 0);
}
