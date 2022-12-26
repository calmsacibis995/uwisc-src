#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: setbuffer.c,v 1.2 86/09/08 14:52:02 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)setbuffer.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include	<stdio.h>

setbuffer(iop, buf, size)
	register FILE *iop;
	char *buf;
	int size;
{
	if (iop->_base != NULL && iop->_flag&_IOMYBUF)
		free(iop->_base);
	iop->_flag &= ~(_IOMYBUF|_IONBF|_IOLBF);
	if ((iop->_base = buf) == NULL) {
		iop->_flag |= _IONBF;
		iop->_bufsiz = NULL;
	} else {
		iop->_ptr = iop->_base;
		iop->_bufsiz = size;
	}
	iop->_cnt = 0;
}

/*
 * set line buffering for either stdout or stderr
 */
setlinebuf(iop)
	register FILE *iop;
{
	char *buf;
	extern char *malloc();

	fflush(iop);
	setbuffer(iop, NULL, 0);
	buf = malloc(BUFSIZ);
	if (buf != NULL) {
		setbuffer(iop, buf, BUFSIZ);
		iop->_flag |= _IOLBF|_IOMYBUF;
	}
}
