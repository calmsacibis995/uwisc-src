/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ftell_.c	5.1	6/7/85
 */

/*
 * return current file position
 *
 * calling sequence:
 *	integer curpos, ftell
 *	curpos = ftell(lunit)
 * where:
 *	lunit is an open logical unit
 *	curpos will be the current offset in bytes from the start of the
 *		file associated with that logical unit
 *		or a (negative) system error code.
 */

#include	"../libI77/fiodefs.h"
#include	"../libI77/f_errno.h"

extern unit units[];

long ftell_(lu)
long *lu;
{
	if (*lu < 0 || *lu >= MXUNIT)
		return(-(long)(errno=F_ERUNIT));
	if (!units[*lu].ufd)
		return(-(long)(errno=F_ERNOPEN));
	return(ftell(units[*lu].ufd));
}
