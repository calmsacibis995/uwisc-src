/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)fgetc_.c	5.1	6/7/85
 */

/*
 * get a character from a logical unit bypassing formatted I/O
 *
 * calling sequence:
 *	integer fgetc
 *	ierror = fgetc (unit, char)
 * where:
 *	char will return a character from logical unit
 *	ierror will be 0 if successful; a system error code otherwise.
 */

#include	"../libI77/fiodefs.h"
#include	"../libI77/f_errno.h"

extern unit units[];	/* logical units table from iolib */

long fgetc_(u, c, clen)
long *u; char *c; long clen;
{
	int	i;
	unit	*lu;

	if (*u < 0 || *u >= MXUNIT)
		return((long)(errno=F_ERUNIT));
	lu = &units[*u];
	if (!lu->ufd)
		return((long)(errno=F_ERNOPEN));
	if (lu->uwrt && ! nowreading(lu))
		return((long)errno);
	if ((i = getc (lu->ufd)) < 0)
	{
		if (feof(lu->ufd))
			return(-1L);
		i = errno;
		clearerr(lu->ufd);
		return((long)i);
	}
	*c = i;
	return(0L);
}
